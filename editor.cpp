#include "editor.h"
#include <fstream>
#include <iostream>
#include <gtkmm/messagedialog.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/scrolledwindow.h>

#include "util.h"

Editor::Editor() : Gtk::Box(Gtk::ORIENTATION_VERTICAL) {
    pack_start(notebook_, Gtk::PACK_EXPAND_WIDGET);
    notebook_.signal_switch_page().connect(sigc::mem_fun(*this, &Editor::on_switch_page));
    show_all_children();

    add_events(Gdk::KEY_PRESS_MASK);
    set_font_size(14);
}

// Destructor
Editor::~Editor() = default;

void Editor::save_file(const std::string& file_path) {
    auto it = std::find_if(tabs_.begin(), tabs_.end(), [&file_path](const auto& pair) {
        return pair.second.file_path == file_path;
    });

    if (it != tabs_.end()) {
        std::ofstream file(file_path);
        if (file.is_open()) {
            file << it->second.text_buffer->get_text();
            file.close();
            std::cout << "File saved: " << file_path << std::endl;
            it->second.modified = false;
            update_tab_label(it->first);
        } else {
            std::cerr << "Error saving file: " << file_path << std::endl;
        }
    }
}

void Editor::save_current_tab() {
    int current_page = notebook_.get_current_page();
    auto it = tabs_.find(current_page);
    if (it != tabs_.end()) {
        save_file(it->second.file_path);
    }
}

void Editor::open_new_tab(const std::string& file_path) {
    if (is_binary_file(file_path)) {
        error_bell();
        show_error_dialog(this->get_toplevel(), "Error: cannot open binary file: " + file_path);
        return;
    }

    for (const auto& [num, tab] : tabs_) {
        if (tab.file_path == file_path) {
            notebook_.set_current_page(num);
            return;
        }
    }

    Gtk::ScrolledWindow* scrolled_window = Gtk::manage(new Gtk::ScrolledWindow());
    scrolled_window->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);

    Gtk::TextView* text_view = Gtk::manage(new Gtk::TextView());
    Glib::RefPtr<Gtk::TextBuffer> text_buffer = Gtk::TextBuffer::create();
    text_view->set_buffer(text_buffer);
    text_view->set_top_margin(5);
    text_view->set_bottom_margin(5);
    text_view->set_left_margin(5);
    text_view->set_right_margin(5);

    // Apply the font size to the new text view
    Pango::FontDescription font_desc;
    font_desc.set_size(font_size_ * PANGO_SCALE);
    text_view->override_font(font_desc);

    scrolled_window->add(*text_view);

    std::string filename = file_path.substr(file_path.find_last_of('/') + 1);
    for (const auto& [num, tab] : tabs_) {
        if (tab.file_path.substr(tab.file_path.find_last_of('/') + 1) == filename) {
            filename = file_path.substr(0, file_path.find_last_of('/'));
            break;
        }
    }

    Gtk::Box* tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    Gtk::Label* tab_label = Gtk::manage(new Gtk::Label(filename));
    Gtk::Button* close_button = Gtk::manage(new Gtk::Button("x"));
    tab_label->set_margin_end(5);

    int page_num = notebook_.append_page(*scrolled_window, *tab_box);

    close_button->signal_clicked().connect([this, page_num]() {
        on_tab_close_button_clicked(page_num);
    });

    tab_box->pack_start(*tab_label, Gtk::PACK_EXPAND_WIDGET);
    tab_box->pack_start(*close_button, Gtk::PACK_SHRINK);

    notebook_.set_current_page(page_num);

    tabs_[page_num] = {file_path, text_buffer, scrolled_window, tab_label, false};

    text_buffer->signal_changed().connect([this, page_num]() {
        on_text_buffer_changed(page_num);
    });

    tab_box->show_all();
    notebook_.show_all();
}

void Editor::on_switch_page(Gtk::Widget* page, guint page_num) {
    auto it = tabs_.find(page_num);
    if (it != tabs_.end()) {
        Tab& tab = it->second;
        if (tab.text_buffer->get_text().empty()) {
            std::ifstream file(tab.file_path, std::ios::in | std::ios::binary);
            if (file.is_open()) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                tab.text_buffer->set_text(buffer.str());
            }
        }
    }
}

bool Editor::on_key_press_event(GdkEventKey* event) {
    if ((event->state & GDK_CONTROL_MASK) && event->keyval == GDK_KEY_s) {
        save_current_tab();
        return true;
    }

    return Gtk::Box::on_key_press_event(event);
}

void Editor::on_tab_close_button_clicked(int page_num) {
    auto it = tabs_.find(page_num);
    if (it != tabs_.end()) {
        tabs_.erase(it);
        notebook_.remove_page(page_num);
    }
}

void Editor::on_text_buffer_changed(int page_num) {
    auto it = tabs_.find(page_num);
    if (it != tabs_.end() && !it->second.modified) {
        it->second.modified = true;
        update_tab_label(page_num);
    }
}

void Editor::update_tab_label(int page_num) {
    auto it = tabs_.find(page_num);
    if (it != tabs_.end()) {
        std::string label_text = it->second.file_path.substr(it->second.file_path.find_last_of('/') + 1);
        if (it->second.modified) {
            label_text += " *";
        }
        it->second.tab_label->set_text(label_text);
    }
}

void Editor::set_font_size(int size) {
    font_size_ = size;
    for (const auto& [num, tab] : tabs_) {
        Pango::FontDescription font_desc;
        font_desc.set_size(font_size_ * PANGO_SCALE);
        tab.text_buffer->get_insert()->get_iter().get_buffer()->get_tag_table()->foreach([font_desc](const Glib::RefPtr<Gtk::TextTag>& tag) {
            tag->property_font_desc() = font_desc;
        });
    }
}
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
}

// Destructor
Editor::~Editor() = default;

// Save file method
void Editor::save_file(const std::string& file_path) {
    std::cout << "Saving file to " << file_path << std::endl;
}

// Open a new tab
void Editor::open_new_tab(const std::string& file_path) {
    if (is_binary_file(file_path)) {
        error_bell();
        show_error_dialog(this->get_toplevel(), "Error: cannot open binary file: " + file_path);
        return;
    }

    // Check if the file is already open
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

    scrolled_window->add(*text_view);

    // Get filename from path
    std::string filename = file_path.substr(file_path.find_last_of('/') + 1);

    // If there is a matching filename in the tabs, show the directory name as well
    for (const auto& [num, tab] : tabs_) {
        if (tab.file_path.substr(tab.file_path.find_last_of('/') + 1) == filename) {
            filename = file_path.substr(0, file_path.find_last_of('/'));
            break;
        }
    }

    Gtk::Box* tab_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    Gtk::Label* tab_label = Gtk::manage(new Gtk::Label(filename));
    Gtk::Button* close_button = Gtk::manage(new Gtk::Button("x"));

    // add padding between the label and the close button
    tab_label->set_margin_end(5);

    int page_num = notebook_.append_page(*scrolled_window, *tab_box);

    close_button->signal_clicked().connect([this, page_num]() {
        on_tab_close_button_clicked(page_num);
    });

    tab_box->pack_start(*tab_label, Gtk::PACK_EXPAND_WIDGET);
    tab_box->pack_start(*close_button, Gtk::PACK_SHRINK);


    notebook_.set_current_page(page_num);

    tabs_[page_num] = {file_path, text_buffer, scrolled_window};

    tab_box->show_all();
    notebook_.show_all();
}

// Load file content when switching pages
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

void Editor::on_tab_close_button_clicked(int page_num) {
    auto it = tabs_.find(page_num);
    if (it != tabs_.end()) {
        tabs_.erase(it);
        notebook_.remove_page(page_num);
    }
}


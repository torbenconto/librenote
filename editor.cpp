//
// Created by tconto on 7/25/24.
//

#include "editor.h"
#include <fstream>
#include <iostream>
#include <gtkmm/messagedialog.h>


bool is_binary_file(const std::string& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (file.is_open()) {
        char ch;
        while (file.get(ch)) {
            if (ch == '\0') {
                return true;
            }
        }
    }
    return false;
}
Editor::Editor() {
    set_border_width(5);

    text_view_.set_wrap_mode(Gtk::WRAP_NONE);
    text_view_.set_hexpand(true);
    text_view_.set_vexpand(true);
    add(text_view_);
    text_buffer_ = Gtk::TextBuffer::create();
    text_view_.set_buffer(text_buffer_);

    show_all_children();
}

Editor::~Editor() = default;

void Editor::load_file(const std::string& file_path) {
    if (is_binary_file(file_path)) {
        error_bell();
        show_error_dialog("Error: cannot open binary files");
        return;
    }
    std::ifstream file(file_path);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        text_buffer_->set_text(buffer.str());
    }
}

void Editor::save_file(const std::string& file_path) {
    std::cout << "Saving file to " << file_path << std::endl;
}

void Editor::show_error_dialog(const std::string& message) {
    auto *parent = dynamic_cast<Gtk::Window *>(get_toplevel());
    Gtk::MessageDialog dialog(*parent, message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.run();
}
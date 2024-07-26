//
// Created by tconto on 7/25/24.
//

#include "util.h"

#include <fstream>
#include <gtkmm/messagedialog.h>

void show_error_dialog(Gtk::Container* cont, const std::string& message) {
    auto *parent = dynamic_cast<Gtk::Window *>(cont);
    Gtk::MessageDialog dialog(*parent, message, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    dialog.run();
}

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

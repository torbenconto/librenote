//
// Created by tconto on 7/25/24.
//

#ifndef UTIL_H
#define UTIL_H

#include <gtkmm/window.h>
#include <string>


void show_error_dialog(Gtk::Container* cont, const std::string& message);

bool is_binary_file(const std::string& file_path);

#endif //UTIL_H

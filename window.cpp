//
// Created by tconto on 7/25/24.
//

#include "window.h"

#include <iostream>
#include <gtkmm/messagedialog.h>
#include <gtkmm/paned.h>


Window::Window() {
    set_title("librenote alpha");

    Gtk::Paned *hpaned = manage(new Gtk::Paned(Gtk::ORIENTATION_HORIZONTAL));
    add(*hpaned);

    Gtk::ScrolledWindow *scrolled_window_explorer = manage(new Gtk::ScrolledWindow());
    scrolled_window_explorer->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window_explorer->set_min_content_width(200);
    scrolled_window_explorer->add(explorer_);
    hpaned->add1(*scrolled_window_explorer);

    Gtk::ScrolledWindow *scrolled_window_editor = manage(new Gtk::ScrolledWindow());
    scrolled_window_editor->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window_editor->add(editor_);
    scrolled_window_explorer->set_hexpand(true);
    scrolled_window_explorer->set_vexpand(true);
    hpaned->add2(*scrolled_window_editor);

    hpaned->set_position(200);

    explorer_.signal_file_selected().connect(sigc::mem_fun(*this, &Window::on_file_selected));

    show_all_children();
}


Window::~Window() = default;

void Window::on_file_selected(const std::string& file_path) {
    std::cout << "File selected: " << file_path << std::endl;
    editor_.open_new_tab(file_path);
}

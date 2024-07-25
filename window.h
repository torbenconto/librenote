//
// Created by tconto on 7/25/24.
//

#ifndef WINDOW_H
#define WINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/notebook.h>

#include "editor.h"
#include "explorer.h"


class Window : public Gtk::Window {
public:
    Window();
    ~Window() override;

protected:
    Explorer explorer_;
    Editor editor_;
    Gtk::Notebook tabs_;

    void on_file_selected(const std::string& file_path);
};



#endif //WINDOW_H

//
// Created by tconto on 7/25/24.
//

#ifndef EDITOR_H
#define EDITOR_H

#include <gtkmm/box.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>


class Editor : public Gtk::Box{
public:
    Editor();
    ~Editor() override;

    void load_file(const std::string& file_path);
    void save_file(const std::string& file_path);
protected:
    Gtk::TextView text_view_;
    Glib::RefPtr<Gtk::TextBuffer> text_buffer_;
private:
    void show_error_dialog(const std::string& message);
};



#endif //EDITOR_H

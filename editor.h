//
// Created by tconto on 7/25/24.
//

#ifndef EDITOR_H
#define EDITOR_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/notebook.h>
#include <gtkmm/textview.h>

struct Tab {
    std::string file_path;
    Glib::RefPtr<Gtk::TextBuffer> text_buffer;
    Gtk::Widget* parent;
};

class Editor : public Gtk::Box {
public:
    Editor();
    ~Editor() override;

    void save_file(const std::string& file_path);
    void open_new_tab(const std::string& file_path);
protected:
    Gtk::TextView text_view_;
    std::map<int, Tab> tabs_;
    Gtk::Notebook notebook_;
private:
    void on_switch_page(Gtk::Widget* page, guint page_num);
    void on_tab_close_button_clicked(int page_num);
};



#endif //EDITOR_H

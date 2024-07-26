#ifndef EXPLORER_H
#define EXPLORER_H

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <filesystem>

class Explorer : public Gtk::ScrolledWindow {
public:
    Explorer();
    ~Explorer() override;

    void populate();

    sigc::signal<void, const std::string&> signal_file_selected();

protected:
    Gtk::Menu contextMenu_;
    Gtk::MenuItem createFileMenuItem_;
    Gtk::MenuItem createFolderMenuItem_;
    Gtk::MenuItem deleteMenuItem_;
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(column_name);
            add(column_icon);
        }

        Gtk::TreeModelColumn<Glib::ustring> column_name;
        Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf>> column_icon;
    };

    ModelColumns columns_;
    Glib::RefPtr<Gtk::TreeStore> treeModel_;
    Gtk::TreeView treeView_;
    Glib::RefPtr<Gdk::Pixbuf> folderIcon_;
    Glib::RefPtr<Gdk::Pixbuf> fileIcon_;

    void populate_directory(const std::filesystem::path& path, const Gtk::TreeModel::Row& parent_row);
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
    bool on_button_press(GdkEventButton* event);
    void on_create_file_menu_item();
    void on_create_folder_menu_item();
    void on_delete_menu_item();
    std::string get_user_input(const std::string& title, const std::string& label);

    // Drag-and-Drop handlers
    void on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& context);
    void on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time);
    void on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time);

    std::string drag_data_;

private:
    sigc::signal<void, const std::string&> file_selected_signal_;
};

#endif // EXPLORER_H
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
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        ModelColumns() {
            add(column_name);
        }

        Gtk::TreeModelColumn<Glib::ustring> column_name;
    };

    ModelColumns columns_;
    Glib::RefPtr<Gtk::TreeStore> treeModel_;
    Gtk::TreeView treeView_;

    void populate_directory(const std::filesystem::path& path, const Gtk::TreeModel::Row& parent_row);
    void on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

private:
    sigc::signal<void, const std::string&> file_selected_signal_;
};

#endif // EXPLORER_H
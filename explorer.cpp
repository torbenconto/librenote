#include "explorer.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <gtkmm/box.h>
#include <gtkmm/dialog.h>

#include "util.h"

Explorer::Explorer() {
    treeView_.set_headers_visible(true);
    add(treeView_);

    treeModel_ = Gtk::TreeStore::create(columns_);
    treeView_.set_model(treeModel_);

    std::string path = std::filesystem::current_path().string();
    std::string dirName = path.substr(path.find_last_of('/') + 1);
    Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(dirName));
    Gtk::CellRendererText* textRenderer = manage(new Gtk::CellRendererText());
    Gtk::CellRendererPixbuf* iconRenderer = manage(new Gtk::CellRendererPixbuf());

    column->pack_start(*iconRenderer, false);
    column->pack_start(*textRenderer, true);
    column->add_attribute(textRenderer->property_text(), columns_.column_name);
    column->add_attribute(iconRenderer->property_pixbuf(), columns_.column_icon);

    treeView_.append_column(*column);

    column->set_expand(true);
    column->set_resizable(true);
    column->set_fixed_width(-1);
    column->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);

    treeView_.signal_row_activated().connect(sigc::mem_fun(*this, &Explorer::on_row_activated));
    treeView_.signal_button_press_event().connect(sigc::mem_fun(*this, &Explorer::on_button_press));

    createFileMenuItem_.set_label("New File");
    createFileMenuItem_.signal_activate().connect(sigc::mem_fun(*this, &Explorer::on_create_file_menu_item));
    contextMenu_.append(createFileMenuItem_);

    createFolderMenuItem_.set_label("New Folder");
    createFolderMenuItem_.signal_activate().connect(sigc::mem_fun(*this, &Explorer::on_create_folder_menu_item));
    contextMenu_.append(createFolderMenuItem_);

    deleteMenuItem_.set_label("Delete");
    deleteMenuItem_.signal_activate().connect(sigc::mem_fun(*this, &Explorer::on_delete_menu_item));
    contextMenu_.append(deleteMenuItem_);

    contextMenu_.show_all();

    folderIcon_ = Gdk::Pixbuf::create_from_file("assets/folder.png");
    fileIcon_ = Gdk::Pixbuf::create_from_file("assets/file.png");

    populate();

    // Setup drag and drop
    std::vector<Gtk::TargetEntry> target_entries;
    target_entries.push_back(Gtk::TargetEntry("text/uri-list", Gtk::TARGET_SAME_WIDGET));

    treeView_.enable_model_drag_source(target_entries, Gdk::BUTTON1_MASK, Gdk::DragAction::ACTION_MOVE);
    treeView_.enable_model_drag_dest(target_entries, Gdk::DragAction::ACTION_MOVE);
    treeView_.signal_drag_begin().connect(sigc::mem_fun(*this, &Explorer::on_drag_begin));
    treeView_.signal_drag_data_get().connect(sigc::mem_fun(*this, &Explorer::on_drag_data_get));
    treeView_.signal_drag_data_received().connect(sigc::mem_fun(*this, &Explorer::on_drag_data_received));
}

Explorer::~Explorer() = default;

void Explorer::populate() {
    treeModel_->clear();
    std::filesystem::path path = std::filesystem::current_path();
    populate_directory(path, Gtk::TreeModel::Row());
}

void Explorer::populate_directory(const std::filesystem::path& path, const Gtk::TreeModel::Row& parent_row) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        Gtk::TreeModel::Row row = *(parent_row ? treeModel_->append(parent_row.children()) : treeModel_->append());
        row[columns_.column_name] = entry.path().filename().string();
        if (entry.is_regular_file()) {
            row[columns_.column_icon] = fileIcon_;
        } else if (entry.is_directory()) {
            row[columns_.column_icon] = folderIcon_;
            populate_directory(entry.path(), row);
        }
    }
}

void Explorer::on_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column) {
    Gtk::TreeModel::iterator iter = treeModel_->get_iter(path);
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];
        std::filesystem::path file_path = std::filesystem::current_path() / name.raw();
        if (std::filesystem::is_regular_file(file_path)) {
            file_selected_signal_.emit(file_path.string());
        }
    }
}

bool Explorer::on_button_press(GdkEventButton* event) {
    if (event->button == 3) {
        Gtk::TreeModel::Path path;
        Gtk::TreeViewColumn* column;
        int cell_x, cell_y;

        if (treeView_.get_path_at_pos(event->x, event->y, path, column, cell_x, cell_y)) {
            treeView_.set_cursor(path, *column, false);
            contextMenu_.popup(event->button, event->time);
            return true;
        }
    }
    return false;
}

void Explorer::on_create_file_menu_item() {
    Gtk::TreeModel::iterator iter = treeView_.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];

        // Get the full path of the selected item
        Gtk::TreeModel::Path tree_path = treeModel_->get_path(iter);
        std::filesystem::path full_path = std::filesystem::current_path();

        // Traverse up the tree to construct the full path
        while (tree_path.up()) {
            Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(tree_path);
            if (parent_iter) {
                Gtk::TreeModel::Row parent_row = *parent_iter;
                Glib::ustring parent_name = parent_row[columns_.column_name];
                full_path /= parent_name.c_str();
            }
        }
        full_path /= name.c_str();

        if (!std::filesystem::is_directory(full_path)) {
            full_path = std::filesystem::current_path();
        }

        std::string file_name = get_user_input("New File", "Enter file name:");
        if (!file_name.empty()) {
            std::filesystem::path file_path = full_path / file_name;
            std::ofstream ofs(file_path, std::ofstream::out);
            ofs.close();
            populate();
        } else {
            show_error_dialog(this->get_toplevel(), "Empty file name provided.");
        }
    }
}

void Explorer::on_create_folder_menu_item() {
    Gtk::TreeModel::iterator iter = treeView_.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];

        // Get the full path of the selected item
        Gtk::TreeModel::Path tree_path = treeModel_->get_path(iter);
        std::filesystem::path full_path = std::filesystem::current_path();

        // Traverse up the tree to construct the full path
        std::vector<std::string> path_components;
        while (tree_path.size() > 0) {
            Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(tree_path);
            if (parent_iter) {
                Gtk::TreeModel::Row parent_row = *parent_iter;
                Glib::ustring parent_name = parent_row[columns_.column_name];
                path_components.push_back(parent_name.c_str());
            }
            tree_path.up();
        }

        // Reverse the path components to construct the correct path
        std::reverse(path_components.begin(), path_components.end());
        for (const auto& component : path_components) {
            full_path /= component;
        }
        std::cout << "Creating folder in: " << full_path << std::endl;

        if (!std::filesystem::is_directory(full_path)) {
            full_path = std::filesystem::current_path();
        }

        std::string folder_name = get_user_input("New Folder", "Enter folder name:");
        if (!folder_name.empty()) {
            std::filesystem::path new_dir_path = full_path / folder_name;
            std::filesystem::create_directory(new_dir_path);
            populate();
        } else {
            show_error_dialog(this->get_toplevel(), "Empty folder name provided.");
        }
    }
}

void Explorer::on_delete_menu_item() {
    Gtk::TreeModel::iterator iter = treeView_.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];

        // Get the full path of the selected item
        Gtk::TreeModel::Path tree_path = treeModel_->get_path(iter);
        std::filesystem::path full_path = std::filesystem::current_path();

        // Traverse up the tree to construct the full path
        std::vector<std::string> path_components;
        while (tree_path.size() > 0) {
            Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(tree_path);
            if (parent_iter) {
                Gtk::TreeModel::Row parent_row = *parent_iter;
                Glib::ustring parent_name = parent_row[columns_.column_name];
                path_components.push_back(parent_name.c_str());
            }
            tree_path.up();
        }

        // Reverse the path components to construct the correct path
        std::reverse(path_components.begin(), path_components.end());
        for (const auto& component : path_components) {
            full_path /= component;
        }

        std::cout << "Deleting: " << full_path << std::endl;

        if (std::filesystem::exists(full_path)) {
            std::filesystem::remove_all(full_path);
            populate();
        }
    }
}

std::string Explorer::get_user_input(const std::string& title, const std::string& label) {
    Gtk::Dialog dialog(title, true);
    dialog.add_button("Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("OK", Gtk::RESPONSE_OK);

    Gtk::Box* content_area = dialog.get_content_area();
    Gtk::Label label_widget(label);
    Gtk::Entry entry;

    content_area->pack_start(label_widget, Gtk::PACK_SHRINK);
    content_area->pack_start(entry, Gtk::PACK_SHRINK);

    dialog.show_all_children();

    int result = dialog.run();
    if (result == Gtk::RESPONSE_OK) {
        return entry.get_text();
    }

    return "";
}

sigc::signal<void, const std::string&> Explorer::signal_file_selected() {
    return file_selected_signal_;
}

// Drag-and-Drop handlers
void Explorer::on_drag_begin(const Glib::RefPtr<Gdk::DragContext>& context) {
    Gtk::TreeModel::iterator iter = treeView_.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];

        // Get the full path of the selected item
        Gtk::TreeModel::Path tree_path = treeModel_->get_path(iter);
        std::filesystem::path full_path = std::filesystem::current_path();

        // Traverse up the tree to construct the full path
        std::vector<std::string> path_components;
        while (tree_path.size() > 0) {
            Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(tree_path);
            if (parent_iter) {
                Gtk::TreeModel::Row parent_row = *parent_iter;
                Glib::ustring parent_name = parent_row[columns_.column_name];
                path_components.push_back(parent_name.c_str());
            }
            tree_path.up();
        }

        // Reverse the path components to construct the correct path
        std::reverse(path_components.begin(), path_components.end());
        for (const auto& component : path_components) {
            full_path /= component;
        }

        // Set the drag data to the full path
        context->set_data("text/uri-list", reinterpret_cast<void*>(const_cast<char*>(full_path.string().c_str())));
    }
}

void Explorer::on_drag_data_get(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time) {
    Gtk::TreeModel::iterator iter = treeView_.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        Glib::ustring name = row[columns_.column_name];

        // Get the full path of the selected item
        Gtk::TreeModel::Path tree_path = treeModel_->get_path(iter);
        std::filesystem::path full_path = std::filesystem::current_path();

        // Traverse up the tree to construct the full path
        std::vector<std::string> path_components;
        while (tree_path.size() > 0) {
            Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(tree_path);
            if (parent_iter) {
                Gtk::TreeModel::Row parent_row = *parent_iter;
                Glib::ustring parent_name = parent_row[columns_.column_name];
                path_components.push_back(parent_name.c_str());
            }
            tree_path.up();
        }

        // Reverse the path components to construct the correct path
        std::reverse(path_components.begin(), path_components.end());
        for (const auto& component : path_components) {
            full_path /= component;
        }

        // Set the selection data to the full path
        std::string data = full_path.string();
        selection_data.set(selection_data.get_target(), 8, (const guchar*)data.c_str(), data.size());
    }
}

void Explorer::on_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int x, int y, const Gtk::SelectionData& selection_data, guint info, guint time) {
    if (selection_data.get_data_type() == "text/uri-list") {
        drag_data_ = std::string((const char*)selection_data.get_data());

        Gtk::TreeModel::Path dest_path;
        Gtk::TreeViewColumn* dest_column;
        int cell_x, cell_y;

        treeView_.convert_widget_to_tree_coords(x, y, cell_x, cell_y);

        if (treeView_.get_path_at_pos(cell_x, cell_y, dest_path, dest_column, cell_x, cell_y)) {
            std::cout << "Destination path found" << std::endl;
            Gtk::TreeModel::iterator dest_iter = treeModel_->get_iter(dest_path);
            if (dest_iter) {
                Gtk::TreeModel::Row dest_row = *dest_iter;
                Glib::ustring dest_name = dest_row[columns_.column_name];

                // Get the full path of the destination item
                std::filesystem::path dest_full_path = std::filesystem::current_path();
                std::vector<std::string> path_components;
                while (dest_path.size() > 0) {
                    Gtk::TreeModel::iterator parent_iter = treeModel_->get_iter(dest_path);
                    if (parent_iter) {
                        Gtk::TreeModel::Row parent_row = *parent_iter;
                        Glib::ustring parent_name = parent_row[columns_.column_name];
                        path_components.push_back(parent_name.c_str());
                    }
                    dest_path.up();
                }

                std::reverse(path_components.begin(), path_components.end());
                for (const auto& component : path_components) {
                    dest_full_path /= component;
                }

                std::filesystem::path source_path = std::filesystem::current_path() / drag_data_;

                if (source_path == dest_full_path) {
                    error_bell();
                    show_error_dialog(this->get_toplevel(), "Cannot move an item into itself.");
                    return;
                }

                if (source_path.parent_path() == dest_full_path) {
                    error_bell();
                    show_error_dialog(this->get_toplevel(), "Cannot move an item into the same dir.");
                    return;
                }

                if (dest_full_path.parent_path() == std::filesystem::current_path() && std::filesystem::is_regular_file(dest_full_path)) {
                    dest_full_path = std::filesystem::current_path();
                }

                if (std::filesystem::is_directory(dest_full_path)) {
                    std::filesystem::path source_path = std::filesystem::current_path() / drag_data_;
                    if (std::filesystem::exists(source_path)) {
                        std::filesystem::path destination_path = dest_full_path / source_path.filename();
                        std::filesystem::rename(source_path, destination_path);
                        populate();
                    }
                } else {
                    error_bell();
                    show_error_dialog(this->get_toplevel(), "Destination is not a directory");
                    std::cout << "Destination is not a directory" << std::endl;
                }
            }
        }
    }
}

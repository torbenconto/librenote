#include "explorer.h"
#include <filesystem>
#include <iostream>

Explorer::Explorer() {
    treeView_.set_headers_visible(true);
    add(treeView_);

    treeModel_ = Gtk::TreeStore::create(columns_);
    treeView_.set_model(treeModel_);

    std::string path = std::filesystem::current_path().string();
    std::string dirName = path.substr(path.find_last_of('/') + 1);
    Gtk::TreeViewColumn* column = manage(new Gtk::TreeViewColumn(dirName, columns_.column_name));
    treeView_.append_column(*column);

    column->set_expand(true);
    column->set_resizable(true);
    column->set_fixed_width(-1);
    column->set_sizing(Gtk::TREE_VIEW_COLUMN_AUTOSIZE);


    treeView_.signal_row_activated().connect(sigc::mem_fun(*this, &Explorer::on_row_activated));

    populate();
}

Explorer::~Explorer() = default;

void Explorer::populate() {
    std::filesystem::path path = std::filesystem::current_path();
    populate_directory(path, Gtk::TreeModel::Row());
}

void Explorer::populate_directory(const std::filesystem::path& path, const Gtk::TreeModel::Row& parent_row) {
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        Gtk::TreeModel::Row row = *(parent_row ? treeModel_->append(parent_row.children()) : treeModel_->append());
        row[columns_.column_name] = entry.path().filename().string();

        if (entry.is_directory()) {
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

sigc::signal<void, const std::string&> Explorer::signal_file_selected() {
    std::cout << "Signal file selected" << std::endl;
    return file_selected_signal_;
}
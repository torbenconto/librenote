#include <iostream>

#include "window.h"


int main(int argc, char *argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.torbenconto.librenote");

    Window window;

    app->run(window);
}

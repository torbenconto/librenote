cmake_minimum_required(VERSION 3.0)
project(librenote)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})

add_compile_options(${GTKMM_CFLAGS_OTHER})

add_executable(librenote main.cpp
        window.cpp
        window.h
        explorer.cpp
        explorer.h
        editor.cpp
        editor.h
)

# Link against Gtkmm libraries
target_link_libraries(librenote ${GTKMM_LIBRARIES})

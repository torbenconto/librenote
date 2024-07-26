cmake_minimum_required(VERSION 3.10)
project(librenote)

set(CMAKE_CXX_STANDARD 17)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM REQUIRED gtkmm-3.0)

include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})
add_definitions(${GTKMM_CFLAGS_OTHER})

add_executable(librenote main.cpp
        window.cpp
        window.h
        explorer.cpp
        explorer.h
        editor.cpp
        editor.h
        util.cpp
        util.h
)

target_link_libraries(librenote ${GTKMM_LIBRARIES})

add_custom_command(
        TARGET librenote POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:librenote>/assets
)

# Custom target to ensure assets are copied
add_custom_target(copy_assets ALL
        DEPENDS librenote
)
cmake_minimum_required(VERSION 3.16)
project(screenshot-cpp-mg)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
qt_add_executable(screenshot-cpp-mg WIN32
    main.cpp
    captureoverlay.cpp
    captureoverlay.h
    preview.cpp
    preview.h
    utils.cpp
    utils.h
    communicator.h
    drawcommands.h
    settings.cpp
    settings.h
    app.rc
)
qt_add_resources(screenshot-cpp-mg "resources"
    PREFIX "/"
    FILES
        screenshot-mg.ico
        svgs/arrow.svg
        svgs/cancel.svg
        svgs/copy.svg
        svgs/delete.svg
        svgs/draw.svg
        svgs/elli-fill.svg
        svgs/elli-outline.svg
        svgs/line.svg
        svgs/pan.svg
        svgs/rect-fill.svg
        svgs/rect-outline.svg
        svgs/reset-window-size.svg
        svgs/reset-zoom-level.svg
        svgs/save.svg
)
target_link_libraries(screenshot-cpp-mg PRIVATE
    Qt6::Widgets
    Qt6::Svg
)
add_custom_command(TARGET screenshot-cpp-mg POST_BUILD
    COMMAND "C:/Qt/6.11.1/msvc2022_64/bin/windeployqt.exe"
        --no-translations
        $<TARGET_FILE:screenshot-cpp-mg>
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        $<TARGET_FILE_DIR:screenshot-cpp-mg>/networkinformation
    COMMAND ${CMAKE_COMMAND} -E remove_directory
        $<TARGET_FILE_DIR:screenshot-cpp-mg>/tls
    COMMAND ${CMAKE_COMMAND} -E remove
        $<TARGET_FILE_DIR:screenshot-cpp-mg>/Qt6Network.dll
    COMMENT "Running windeployqt and cleaning up..."
)
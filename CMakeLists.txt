cmake_minimum_required(VERSION 3.16)
project(screenshot-cpp-mg)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Svg)
qt_add_executable(screenshot-cpp-mg WIN32
    src/main.cpp
    src/captureoverlay.cpp
    src/captureoverlay.h
    src/preview.cpp
    src/preview.h
    src/utils.cpp
    src/utils.h
    src/communicator.h
    src/drawcommands.h
    src/settings.cpp
    src/settings.h
    src/windowoverlay.cpp
    src/windowoverlay.h
    src/types.h
    src/app.rc
)
target_include_directories(screenshot-cpp-mg PRIVATE src)
qt_add_resources(screenshot-cpp-mg "resources"
    PREFIX "/"
    BASE "assets"
    FILES
        assets/screenshot-mg.ico
        assets/arrow.svg
        assets/cancel.svg
        assets/copy.svg
        assets/delete.svg
        assets/draw.svg
        assets/elli-fill.svg
        assets/elli-outline.svg
        assets/line.svg
        assets/pan.svg
        assets/rect-fill.svg
        assets/rect-outline.svg
        assets/reset-window-size.svg
        assets/reset-zoom-level.svg
        assets/save.svg
)
target_link_libraries(screenshot-cpp-mg PRIVATE
    Qt6::Widgets
    Qt6::Svg
    dwmapi
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
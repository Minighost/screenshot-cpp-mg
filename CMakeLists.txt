cmake_minimum_required(VERSION 3.16)
project(ScreenshotTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "C:/Qt/6.11.1/msvc2022_64")
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_add_executable(ScreenshotTool
    main.cpp
    overlay.cpp
    overlay.h
    preview.cpp
    preview.h
    utils.cpp
    utils.h
)
target_link_libraries(ScreenshotTool PRIVATE
    Qt6::Widgets
)
add_custom_command(TARGET ScreenshotTool POST_BUILD
    COMMAND "C:/Qt/6.11.1/msvc2022_64/bin/windeployqt.exe"
        $<TARGET_FILE:ScreenshotTool>
    COMMENT "Running windeployqt..."
)
cmake_minimum_required(VERSION 3.16)

project(calculator VERSION 0.1 LANGUAGES CXX)

# GUI app notice for Windows
set(CMAKE_WIN32_EXECUTABLE ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Using Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets OpenGL)

set(PROJECT_SOURCES
    main.cpp
    calcwindow.cpp
    calcwindow.h
    calcwindow.ui
    PlotGlWidget.h
    PlotGlWidget.cpp
)

qt_add_executable(test
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(test PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    Qt6::OpenGL
)

set_target_properties(test PROPERTIES
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS test
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_finalize_executable(test)

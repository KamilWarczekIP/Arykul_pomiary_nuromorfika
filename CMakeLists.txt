cmake_minimum_required(VERSION 3.19)
project(Arykul_pomiary_nuromorfika LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

qt_add_executable(Arykul_pomiary_nuromorfika
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    backend.hpp backend.cpp
    res.qrc
    qplot.hpp qplot.cpp
)

target_link_libraries(Arykul_pomiary_nuromorfika
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_link_libraries(Arykul_pomiary_nuromorfika PRIVATE Qt6::Core)

include(GNUInstallDirs)
# Visa dla keytheya
include_directories("C:\\Program Files\\IVI Foundation\\VISA\\Win64\\Include")
target_link_libraries(Arykul_pomiary_nuromorfika PRIVATE "C:\\Program Files\\IVI Foundation\\VISA\\Win64\\Lib_x64\\msc\\visa32.lib")

# WaveForms SDK
if(WIN32)
    set(WAVEFORMS_INCLUDE "C:/Program Files (x86)/Digilent/WaveFormsSDK/inc")
    set(WAVEFORMS_LIB "C:/Program Files (x86)/Digilent/WaveFormsSDK/lib/x64/dwf.lib")
else()
    set(WAVEFORMS_INCLUDE "/usr/include")
    set(WAVEFORMS_LIB "/usr/lib/libdwf.so")
endif()

include_directories(${WAVEFORMS_INCLUDE})

target_link_libraries(Arykul_pomiary_nuromorfika PRIVATE ${WAVEFORMS_LIB})


install(TARGETS Arykul_pomiary_nuromorfika
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Arykul_pomiary_nuromorfika
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

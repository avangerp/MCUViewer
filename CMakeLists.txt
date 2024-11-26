cmake_minimum_required(VERSION 3.8)

if(MAKE_TESTS)
    add_subdirectory(test)
    return()
endif()

project(MCUViewer)

set(MCUVIEWER_VERSION 1.0.1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

if(PRODUCTION)
    set(CMAKE_BUILD_TYPE Release)
    message("Building for production!")
else()
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "ReleaseWithDbg")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g1")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g1")
endif()

add_compile_options(-Wall
    -Wfatal-errors
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    -Wno-missing-field-initializers)

# set(JLINK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jlink/inc/)

include(FetchContent)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0)

FetchContent_GetProperties(spdlog)

if(NOT spdlog_POPULATED)
    FetchContent_Populate(spdlog)
    add_subdirectory(${spdlog_SOURCE_DIR} ${spdlog_BINARY_DIR})
endif()

# this is to avoid warnings from spdlog
get_target_property(SPDLOG_IID spdlog INTERFACE_INCLUDE_DIRECTORIES)
set_target_properties(spdlog PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${SPDLOG_IID}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
# include(${CMAKE_MODULE_PATH}/Findlibusb.cmake)
# find_package(libusb REQUIRED)
# find_package(glfw3 REQUIRED)
# set(STLINK_LINUX ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stlink/lib/macos/libstlink.a)
# set(JLINK_LINUX ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jlink/lib/macos/libjlinkarm.8.10.8.dylib)
# set(INSTALL_PATH /usr/local/MCUViewer)
# set(LIB_INSTALL_PATH /usr/local/lib)
# set(DESKTOP_FILE_PATH /usr/share/applications)

# set(LIB_CONF_FILE ${CMAKE_CURRENT_SOURCE_DIR}/launch/install/Unix/MCUViewer.conf)
# set(LIB_CONF_FILE_PATH /etc/ld.so.conf.d/)

# find_package(OpenGL REQUIRED)
# find_package(GLUT REQUIRED)

set(STLINK_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stlink/lib/macos/libstlink.a)
set(GLFW3_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GLFW/lib/macos/libglfw3.a)
set(GDB_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GDB/macos/gdb)
set(SPDLOG_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/lib/macos/libspdlog.a)
set(LIBUSB_MACOS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/lib/macos/libusb-1.0.a)

set(PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/commons.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/Gui.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiPlots.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiSwoPlots.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiSwoControl.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiAbout.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiImportVariables.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui/GuiAcqusition.mm
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryReader/MemoryReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryReader/StlinkDebugProbe.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryReader/JlinkDebugProbe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Plot/Plot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Variable/Variable.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MovingAverage/MovingAverage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigHandler/ConfigHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileHandler/NFDFileHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImguiPlugins/ImguiPlugins.cpp   
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlotHandler/PlotHandlerBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlotHandler/PlotHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlotHandler/TracePlotHandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TraceReader/TraceReader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TraceReader/StlinkTraceProbe.cpp
    # ${CMAKE_CURRENT_SOURCE_DIR}/src/TraceReader/JlinkTraceProbe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GdbParser/GdbParser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CSVStreamer/CSVStreamer.cpp)

set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/imgui_impl_metal.mm)

set(IMPLOT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/implot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/implot_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/implot_items.cpp)

set_source_files_properties(
    ${IMPLOT_SOURCES}
    PROPERTIES
    COMPILE_FLAGS "-w"
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/nfd/)

string(REGEX MATCH "([0-9]+)\\.([0-9]+)\\.([0-9]+)" _ "${MCUVIEWER_VERSION}")
set(EXECUTABLE ${CMAKE_PROJECT_NAME}-${CMAKE_MATCH_1}_${CMAKE_MATCH_2}_${CMAKE_MATCH_3})

add_executable(${EXECUTABLE} ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${IMPLOT_SOURCES})

set(GIT_VERSION_HPP ${CMAKE_CURRENT_SOURCE_DIR}/src/gitversion.hpp)

# Add a custom command to generate gitversion.hpp
add_custom_command(
    OUTPUT ${GIT_VERSION_HPP}
    COMMAND ${CMAKE_COMMAND} -E env PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR} python3 ${CMAKE_CURRENT_SOURCE_DIR}/launch/addGitVersion.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/.git/HEAD
    COMMENT "Generating ${GIT_VERSION_HPP}..."
    VERBATIM
)

add_custom_target(addGitVersion ALL DEPENDS ${GIT_VERSION_HPP})
add_dependencies(${EXECUTABLE} addGitVersion)


target_compile_definitions(${EXECUTABLE}
    PRIVATE MCUVIEWER_VERSION_MAJOR=${CMAKE_MATCH_1}
    PRIVATE MCUVIEWER_VERSION_MINOR=${CMAKE_MATCH_2}
    PRIVATE MCUVIEWER_VERSION_REVISION=${CMAKE_MATCH_3}
)

target_include_directories(${EXECUTABLE} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Gui
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MemoryReader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Plot
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ScrollingBuffer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/MovingAverage
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Variable
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConfigHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImguiPlugins
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FileHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/src/PlotHandler
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TraceReader
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RingBuffer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Statistics
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GdbParser
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CSVStreamer)

target_include_directories(${EXECUTABLE} SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stlink/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/libusb/inc/libusb-1.0/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/backends/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GLFW/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/GLFW/inc/GLFW/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/implot/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mINI/src/mini/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nfd/src/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CLI11/inc/)


# include_directories(${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${STLINK_INCLUDE_DIRS})
target_link_libraries(${EXECUTABLE} 
                        ${GLFW3_MACOS}
                        ${STLINK_MACOS} 
                        ${SPDLOG_MACOS} 
                        ${LIBUSB_MACOS} 
                        "-framework Metal"
                        "-framework MetalKit"
                        "-framework Cocoa"
                        "-framework IOKit"
                        "-framework CoreVideo"
                        "-framework QuartzCore"
                        "-framework AppKit"
                        "-framework UniformTypeIdentifiers"
                        "-framework Security"
                        nfd
                        )

add_custom_command(TARGET ${EXECUTABLE} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui.ini
    ${CMAKE_BINARY_DIR}/imgui.ini)
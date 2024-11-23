// Dear ImGui: standalone example application for GLFW + Metal, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include <spdlog/sinks/stdout_color_sinks.h>

#include <iostream>

#include "../commons.hpp"
// #include "CLI11.hpp"
// #include "ConfigHandler.hpp"
#include "Gui.hpp"
#include "NFDFileHandler.hpp"
#include "PlotHandler.hpp"
#include "gitversion.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

std::shared_ptr<spdlog::logger> logger;

int main(int argc, char** argv)
{
    NFDFileHandler fileHandler;
    auto loggerPtr = logger.get();
    // Call Gui
    Gui gui(&fileHandler, loggerPtr);
    // return Gui(argc, argv);
    
}
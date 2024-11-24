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
#include "ConfigHandler.hpp"
#include "Gui.hpp"
#include "NFDFileHandler.hpp"
#include "PlotHandler.hpp"
#include "gitversion.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

std::atomic<bool> done = false;
std::shared_ptr<spdlog::logger> logger;

void prepareLogger();

int main(int argc, char** argv)
{
    bool debug = false;
    std::string projectPath = "";

    prepareLogger();

    if (debug)
		logger->set_level(spdlog::level::debug);
	else
		logger->set_level(spdlog::level::info);

    logger->info("Starting MCUViewer!");

    auto loggerPtr = logger.get();

    NFDFileHandler fileHandler;
    
    Gui gui(&fileHandler, loggerPtr);

	logger->info("Closing MCUViewer!");
	logger->flush();
	spdlog::shutdown();
    return 0;
}

void prepareLogger()
{
#if defined(__APPLE__) || defined(_UNIX)
	std::string logDirectory = std::string(std::getenv("HOME")) + "/MCUViewer/logs/logfile.txt";
#elif _WIN32
	std::string logDirectory = std::string(std::getenv("APPDATA")) + "/MCUViewer/logs/logfile.txt";
#else
#error "Your system is not supported!"
#endif

	spdlog::sinks_init_list sinkList = {std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
										std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logDirectory, 5 * 1024 * 1024, 10)};
	logger = std::make_shared<spdlog::logger>("logger", sinkList.begin(), sinkList.end());
	spdlog::register_logger(logger);
}
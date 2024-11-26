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
#include "CLI11.hpp"
#include "ConfigHandler.hpp"
#include "Gui.hpp"
#include "NFDFileHandler.hpp"
#include "PlotHandler.hpp"
#include "gitversion.hpp"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

std::atomic<bool> done = false;
std::mutex mtx;
std::shared_ptr<spdlog::logger> logger;
CLI::App app{"MDtool"};

void prepareLogger();
void prepareCLIParser(bool& debug, std::string& projectPath);

int main(int argc, char** argv)
{
    bool debug = false;
    std::string projectPath = "";
    prepareCLIParser(debug, projectPath);

    CLI11_PARSE(app, argc, argv);

    prepareLogger();

    if (debug)
		logger->set_level(spdlog::level::debug);
	else
		logger->set_level(spdlog::level::info);

    logger->info("Starting MCUViewer!");
    logger->info("Version: {}.{}.{}", MCUVIEWER_VERSION_MAJOR, MCUVIEWER_VERSION_MINOR, MCUVIEWER_VERSION_REVISION);
	logger->info("Commit hash {}", GIT_HASH);

    auto loggerPtr = logger.get();

    PlotHandler plotHandler(done, &mtx, loggerPtr);
    TracePlotHandler tracePlotHandler(done, &mtx, loggerPtr);
    ConfigHandler configHandler("", &plotHandler, &tracePlotHandler, loggerPtr);
    NFDFileHandler fileHandler;
    GdbParser parser(loggerPtr);
    
    Gui gui(&plotHandler, &configHandler, &fileHandler, &tracePlotHandler, done, &mtx, &parser, loggerPtr, projectPath);

    while (!done)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	logger->info("Closing MCUViewer!");
	logger->flush();
	spdlog::shutdown();
    return 0;
}

void prepareLogger()
{
	std::string logDirectory = std::string(std::getenv("HOME")) + "/MCUViewer/logs/logfile.txt";
	spdlog::sinks_init_list sinkList = {std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
										std::make_shared<spdlog::sinks::rotating_file_sink_mt>(logDirectory, 5 * 1024 * 1024, 10)};
	logger = std::make_shared<spdlog::logger>("logger", sinkList.begin(), sinkList.end());
	spdlog::register_logger(logger);
}


void prepareCLIParser(bool& debug, std::string& projectPath)
{
	app.fallthrough();
	app.ignore_case();
	app.add_flag("-d,--debug", debug, "Use for extra debug messages and logs");
	app.add_option("-p,--project", projectPath, "Use to open a project directly from command line");
}

#ifndef _GUI_HPP
#define _GUI_HPP

#include <thread>
#include <string>

#include "IDebugProbe.hpp"
#include "IFileHandler.hpp"

#include "PlotHandler.hpp"

#include "imgui.h"
#include "implot.h"

class Gui
{
    public:
        Gui(IFileHandler* fileHandler, spdlog::logger* logger);
    private:
        
        PlotHandler* plotHandler;
        std::shared_ptr<IDebugProbe> stlinkProbe;
        std::shared_ptr<IDebugProbe> debugProbeDevice;
        
        float contentScale = 1.0f;

        IFileHandler* fileHandler;

        spdlog::logger* logger;
};

#endif
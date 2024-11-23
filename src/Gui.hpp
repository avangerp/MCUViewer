#ifndef _GUI_HPP
#define _GUI_HPP

#include "imgui.h"
#include "implot.h"
#include "PlotHandlerBase.hpp"
#include "PlotHandler.hpp"
#include "IDebugProbe.hpp"
#include "IFileHandler.hpp"

class Gui
{
    public:
        Gui(IFileHandler* fileHandler, spdlog::logger* logger);
    private:
        float contentScale = 1.0f;

        IFileHandler* fileHandler;
        PlotHandler* plotHandler;
        std::shared_ptr<IDebugProbe> stlinkProbe;
        std::shared_ptr<IDebugProbe> debugProbeDevice;

        spdlog::logger* logger;
};

#endif
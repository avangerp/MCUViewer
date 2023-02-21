#ifndef _GUI_HPP
#define _GUI_HPP

#include <mutex>
#include <thread>

#include "ConfigHandler.hpp"
#include "PlotHandler.hpp"
#include "imgui.h"

class Gui
{
   public:
	enum class state
	{
		STOP = 0,
		RUN = 1,
	};

	Gui(PlotHandler* plotHandler, ConfigHandler* configHandler);
	~Gui();

   private:
	state viewerState = state::STOP;
	const std::map<state, std::string> viewerStateMap{{state::RUN, "RUNNING"}, {state::STOP, "STOPPED"}};
	std::vector<Variable> vars;
	std::thread threadHandle;
	PlotHandler* plotHandler;
	ConfigHandler* configHandler;
	std::mutex mtx;
	std::string projectConfigFile;
	std::string projectElfFile;
	bool showAcqusitionSettingsWindow = false;

	bool done = false;
	void mainThread();
	void drawMenu();
	void drawStartButton();
	void drawAddVariableButton();
	void drawVarTable();
	void drawPlotsTree();
	void drawAcqusitionSettingsWindow();
	std::string intToHexString(uint32_t i);
};

#endif
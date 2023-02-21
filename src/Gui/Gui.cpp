#include "Gui.hpp"

#include <SDL.h>
#include <SDL_opengl.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include "ElfReader.hpp"
#include "VarReader.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"
#include "implot.h"
#include "nfd.h"

struct InputTextCallback_UserData
{
	std::string* Str;
	ImGuiInputTextCallback ChainCallback;
	void* ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
	InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		// Resize string callback
		// If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
		std::string* str = user_data->Str;
		IM_ASSERT(data->Buf == str->c_str());
		str->resize(data->BufTextLen);
		data->Buf = (char*)str->c_str();
	}
	else if (user_data->ChainCallback)
	{
		// Forward to user callback, if any
		data->UserData = user_data->ChainCallbackUserData;
		return user_data->ChainCallback(data);
	}
	return 0;
}
namespace ImGui
{
bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
}
bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
	IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
	flags |= ImGuiInputTextFlags_CallbackResize;

	InputTextCallback_UserData cb_user_data;
	cb_user_data.Str = str;
	cb_user_data.ChainCallback = callback;
	cb_user_data.ChainCallbackUserData = user_data;
	return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

Gui::Gui(PlotHandler* plotHandler, ConfigHandler* configHandler) : plotHandler(plotHandler), configHandler(configHandler)
{
	// std::string file("~/STMViewer/test/STMViewer_test/Debug/STMViewer_test.elf");

	// ElfReader* elf = new ElfReader(file);
	// std::vector<std::string> names({"test.ua", "test.ia", "test.ub", "dupa", "test.tri", "test.triangle", "test.a", "test.b", "test.c"});
	// vars = elf->getVariableVectorBatch(names);
	/* TODO reserve is not the best solution! */
	vars.reserve(200);
	threadHandle = std::thread(&Gui::mainThread, this);
}

Gui::~Gui()
{
	if (threadHandle.joinable())
		threadHandle.join();
}

void Gui::mainThread()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
	{
		printf("Error: %s\n", SDL_GetError());
		return;
	}
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	SDL_Window* window;

	window = SDL_CreateWindow("STMViewer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1500, 1000, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_GLContext gl_context = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);	// Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImPlot::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	NFD_Init();

	bool show_demo_window = true;
	bool p_open = true;

	while (!done)
	{
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				done = true;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
				done = true;
		}

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		if (show_demo_window)
			ImPlot::ShowDemoWindow();

		ImGui::Begin("Plots", &p_open, 0);
		if (showAcqusitionSettingsWindow)
			drawAcqusitionSettingsWindow();
		plotHandler->drawAll();
		drawMenu();
		ImGui::End();

		ImGui::Begin("VarViewer", &p_open, 0);
		drawStartButton();
		ImGui::SameLine();
		drawAddVariableButton();
		drawVarTable();
		drawPlotsTree();
		ImGui::End();

		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

		SDL_GL_SwapWindow(window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	NFD_Quit();
	SDL_Quit();
}

void Gui::drawMenu()
{
	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("New"))
		{
		}
		if (ImGui::MenuItem("Open", "Ctrl+O"))
		{
			nfdchar_t* outPath;
			nfdfilteritem_t filterItem[1] = {{"Project files", "cfg"}};
			nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
			if (result == NFD_OKAY)
			{
				configHandler->changeConfigFile(std::string(outPath));
				vars.clear();
				plotHandler->removeAllPlots();
				projectElfFile = configHandler->getElfFilePath();
				configHandler->readConfigFile(vars);
				std::cout << outPath << std::endl;
				NFD_FreePath(outPath);
			}
			else if (result == NFD_ERROR)
			{
				std::cout << "Error: %s\n"
						  << NFD_GetError() << std::endl;
			}
		}
		if (ImGui::BeginMenu("Open Recent"))
		{
			ImGui::MenuItem("fish_hat.c");
			ImGui::MenuItem("fish_hat.inl");
			ImGui::MenuItem("fish_hat.h");
			if (ImGui::BeginMenu("More.."))
			{
				ImGui::MenuItem("Hello");
				ImGui::MenuItem("Sailor");
				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Save", "Ctrl+S"))
		{
		}
		if (ImGui::MenuItem("Save As.."))
		{
		}
		if (ImGui::MenuItem("Quit"))
		{
			done = true;
		}
		ImGui::EndMenu();
	}
	if (ImGui::BeginMenu("Options"))
	{
		ImGui::MenuItem("Acqusition settings...", NULL, &showAcqusitionSettingsWindow);
		ImGui::EndMenu();
	}
	ImGui::EndMainMenuBar();
}

void Gui::drawStartButton()
{
	if (viewerState == state::RUN)
	{
		ImVec4 color = (ImVec4)ImColor::HSV(0.365f, 0.94f, 0.37f);
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
	}
	else if (viewerState == state::STOP)
	{
		ImVec4 color = ImColor::HSV(0.116f, 0.97f, 0.72f);
		ImGui::PushStyleColor(ImGuiCol_Button, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
	}

	if (ImGui::Button(viewerStateMap.at(viewerState).c_str(), ImVec2(200, 50)))
	{
		if (viewerState == state::STOP)
		{
			viewerState = state::RUN;
			plotHandler->eraseAllPlotData();
			plotHandler->setViewerState((PlotHandler::state)state::RUN);
		}
		else
		{
			plotHandler->setViewerState((PlotHandler::state)state::STOP);
			viewerState = state::STOP;
		}
	}

	ImGui::PopStyleColor(3);
}
void Gui::drawAddVariableButton()
{
	if (ImGui::Button("Add variable"))
	{
		Variable* newVar = new Variable("new");
		newVar->setAddress(0x20000000);
		newVar->setType(Variable::type::U8);
		vars.push_back(*newVar);
	}
}

void Gui::drawVarTable()
{
	static ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

	ImVec2 outer_size = ImVec2(0.0f, 300);
	if (ImGui::BeginTable("table_scrolly", 3, flags, outer_size))
	{
		ImGui::TableSetupScrollFreeze(0, 1);  // Make top row always visible
		ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_None);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_None);
		ImGui::TableHeadersRow();

		// Demonstrate using clipper for large vertical lists
		ImGuiListClipper clipper;
		clipper.Begin(vars.size());
		while (clipper.Step())
		{
			for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::PushID(row);
				ImGui::InputText("##edit", &vars[row].getName(), 0, NULL, NULL);
				ImGui::PopID();
				ImGui::TableSetColumnIndex(1);
				ImGui::Text(("0x" + std::string(intToHexString(vars[row].getAddress()))).c_str());
				ImGui::TableSetColumnIndex(2);
				ImGui::Text(vars[row].getTypeStr().c_str());
			}
		}
		ImGui::EndTable();
	}
}

void Gui::drawPlotsTree()
{
	if (ImGui::TreeNode("Plots"))
	{
		for (uint32_t i = 0; i < plotHandler->getPlotsCount(); i++)
		{
			ImGui::SetNextItemOpen(true, ImGuiCond_Once);

			if (ImGui::TreeNode((void*)(intptr_t)i, (plotHandler->getPlot(i)->getName()).c_str(), i))
			{
				ImGui::Text("blah blah");
				ImGui::SameLine();
				if (ImGui::SmallButton("button"))
				{
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
}

void Gui::drawAcqusitionSettingsWindow()
{
	ImGui::Begin("Acqusition Settings", &showAcqusitionSettingsWindow, 0);
	ImGui::Text("Please pick *.elf file");
	ImGui::InputText("##", &projectElfFile, 0, NULL, NULL);
	ImGui::SameLine();
	if (ImGui::SmallButton("..."))
	{
		nfdchar_t* outPath;
		nfdfilteritem_t filterItem[1] = {{"Executable files", "elf"}};
		nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, 1, NULL);
		if (result == NFD_OKAY)
		{
			std::cout << outPath << std::endl;
			projectElfFile = std::string(outPath);
			NFD_FreePath(outPath);
		}
		else if (result == NFD_ERROR)
		{
			std::cout << "Error: %s\n"
					  << NFD_GetError() << std::endl;
		}
	}

	ImGui::End();
}

std::string Gui::intToHexString(uint32_t var)
{
	std::stringstream ss;
	ss << std::hex << var;
	return ss.str();
}
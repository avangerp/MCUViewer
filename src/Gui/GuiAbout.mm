#include "../commons.hpp"
#include "../gitversion.hpp"
#include "Gui.hpp"

void Gui::drawAboutWindow()
{
	if (showAboutWindow)
		ImGui::OpenPopup("About");

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(500 * contentScale, 300 * contentScale));
	if (ImGui::BeginPopupModal("About", &showAboutWindow, 0))
	{
		drawCenteredText("MCUViewer");
		// std::string line2("version: " + std::to_string(MCUVIEWER_VERSION_MAJOR) + "." + std::to_string(MCUVIEWER_VERSION_MINOR) + "." + std::to_string(MCUVIEWER_VERSION_REVISION));
		// drawCenteredText(std::move(line2));
		drawCenteredText(std::string(GIT_HASH));
		ImGui::SameLine();
		const bool copy = ImGui::SmallButton("copy");
		if (copy)
		{
			ImGui::LogToClipboard();
			ImGui::LogText("%s", GIT_HASH);
			ImGui::LogFinish();
		}

		ImGui::Dummy(ImVec2(-1, 20 * contentScale));
		drawCenteredText("by Piotr Wasilewski (klonyyy)");
		ImGui::Dummy(ImVec2(-1, 20 * contentScale));

		const float buttonHeight = 25.0f * contentScale;

		ImGui::SetCursorPosX((ImGui::GetWindowSize().x - 210 * contentScale) / 2.0f);

		if (ImGui::Button("Releases", ImVec2(100 * contentScale, buttonHeight)))
			openWebsite("https://github.com/klonyyy/MCUViewer/releases");
		ImGui::SameLine();
		if (ImGui::Button("Support <3", ImVec2(100 * contentScale, buttonHeight)))
			openWebsite("https://github.com/sponsors/klonyyy");

		ImGui::SetCursorPos(ImVec2(0, ImGui::GetWindowSize().y - buttonHeight / 2.0f - ImGui::GetFrameHeightWithSpacing()));
		if (ImGui::Button("Done", ImVec2(-1, buttonHeight)))
		{
			showAboutWindow = false;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
}

bool Gui::openWebsite(const char* url)
{
	const char* browser = getenv("BROWSER");
	if (browser == NULL)
		browser = "xdg-open";
	char command[256];
	snprintf(command, sizeof(command), "%s %s", browser, url);
	// auto status = system(command);

	return true;
}

#include "WO.h"
#include "irrklang.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "WOImGui.h"
#include "AftrImGuiIncludes.h"
#include "AftrImgui_Markdown_Renderer.h"
#include <list>
#include <string>
#include "AssetMenu.h"
using namespace Aftr;

void AssetMenu::AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine) {
	ImGui::SetWindowPos("AssetMenu", ImVec2(0, 0));
	static std::filesystem::path selected_path = "";
	static AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();
	ImGui::Begin("AssetMenu");

	{
		if (ImGui::Button("Import Asset"))
			gui->fileDialog_show_Open("");
		if (gui->fileDialog_has_selected_path("")) {
			selected_path = *gui->fileDialog_get_selected_path(""); //can only get one time, this clears the dialog's state!
			std::string pathAsString = selected_path.string();

			int pathLength = pathAsString.length(); // Prevents multiple identical function calls
			if (pathLength > 4) // Impossible path (5 characters is minimum path length)
			{
				std::string extension = pathAsString.substr(pathLength - 3);

				if (extension == "obj")
					assets.importModel(pathAsString);
				if (extension == "wav")
					assets.importAudio(engine, pathAsString.c_str());
			}
		}
		if (ImGui::CollapsingHeader("Assets"))
		{
			for (std::list<WO*>::iterator it = assets.WorldObjects.begin(); it != assets.WorldObjects.end(); ++it) 
				ImGui::Text(("    " + (*it)->getLabel()).c_str());
		}
		if (ImGui::CollapsingHeader("Audio"))
		{
			for (std::list<Audio>::iterator it = assets.AudioSources.begin(); it != assets.AudioSources.end(); ++it)
				ImGui::Text(("	" + (*it).first).c_str());
		}

	}

	ImGui::End();

}

void AssetMenu::importModel(const std::string& path)
{
	std::string label = path;
	int lastSlashPos = path.rfind('\\'); // Find the position of the last '/'
	int lastDotPos = path.rfind('.'); // Find the position of the last '.'
	if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
		label = path.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
	// Check if asset already exists
	for (std::list<WO*>::iterator it = WorldObjects.begin(); it != WorldObjects.end(); ++it)
	{
		if ((*it)->getLabel() == label)
			return;
	}
	WO* wo = WO::New((path), Vector(1, 1, 1));
	wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	wo->setLabel(label);
	WorldObjects.push_back(wo);
}

void AssetMenu::importAudio(irrklang::ISoundEngine* engine, const char* soundFileName)
{
	std::string name(soundFileName);
	int lastSlashPos = name.rfind('\\'); // Find the position of the last '/'
	int lastDotPos = name.rfind('.'); // Find the position of the last '.'
	if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
		name = name.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
	// Check if audio source already exists
	for (std::list<Audio>::iterator it = AudioSources.begin(); it != AudioSources.end(); ++it)
	{
		if ((*it).first == name)
			return;
	}
	AudioSources.push_back(std::make_pair(name, engine->addSoundSourceFromFile(soundFileName)));
}
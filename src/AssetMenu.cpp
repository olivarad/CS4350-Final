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

void AssetMenu::AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine) 
{
	ImGui::SetWindowPos("AssetMenu", ImVec2(0, 0));
	static std::filesystem::path selected_path = "";
	static AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();
	static char playlistName[256] = {};
	ImGui::Begin("AssetMenu");
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !assets.ShowingPlaylistCreatorMenu)
			ImGui::SetWindowCollapsed(true);
		if (ImGui::Button("Import Asset"))
			gui->fileDialog_show_Open("Select obj or wav files");
		if (gui->fileDialog_has_selected_path("Select obj or wav files")) {
			selected_path = *gui->fileDialog_get_selected_path("Select obj or wav files"); //can only get one time, this clears the dialog's state!
			std::string pathAsString = selected_path.string();

			int pathLength = pathAsString.length(); // Prevents multiple identical function calls
			if (pathLength > 4) // Impossible path (5 characters is minimum path length)
			{
				std::string extension = pathAsString.substr(pathLength - 3);

				if (extension == "obj")
				{	
					gui->fileDialog_show_Open("Select jpg file to skin model");
					if (gui->fileDialog_has_selected_path("Select jpg file to skin model"))
					{
						static std::filesystem::path texturePath = *gui->fileDialog_get_selected_path("Select jpg file to skin model");
						std::string texturePathString = texturePath.string();
						extension = texturePathString.substr(texturePathString.length() - 3);
						if (extension == "jpg")
							assets.importModel(pathAsString, texturePathString);
						else
							std::cerr << "Invalid File Extension" << std::endl;
					}
				}
				else if (extension == "wav")
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
			for (std::set<Audio>::iterator it = assets.AudioSources.begin(); it != assets.AudioSources.end(); ++it)
				if (ImGui::Button(((*it).first).c_str()))
				{
					if (assets.CurrentBackgroudSound != nullptr)
						engine->stopAllSoundsOfSoundSource(assets.CurrentBackgroudSound);
					assets.currentPlaylist = "";
					assets.currentPlaylistAudio = nullptr;
					assets.CurrentBackgroudSound = (*it).second;
					//std::cout << assets.CurrentBackgroudSound << std::endl;
					engine->play2D((*it).second);
				}
			if (ImGui::Button("Stop Audio Playback"))
			{
				if (assets.CurrentBackgroudSound != nullptr)
				{
					engine->stopAllSoundsOfSoundSource(assets.CurrentBackgroudSound);
					assets.CurrentBackgroudSound = nullptr;
					assets.currentPlaylist = "";
					assets.currentPlaylistAudio = nullptr;
				}
			}
		}
		if (ImGui::Button("Make Playlist"))
		{
			std::memset(playlistName, '\0', sizeof(playlistName));
			assets.selectedAudio.clear();
			assets.ShowingPlaylistCreatorMenu = true;
		}
		if (assets.ShowingPlaylistCreatorMenu)
		{
			ImGui::OpenPopup("Playlist Creator");
			if (ImGui::BeginPopup("Playlist Creator"))
			{
				ImVec2 popupSize = ImGui::GetWindowSize();
				ImVec2 centerPos = ImVec2((ImGui::GetIO().DisplaySize.x - popupSize.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - popupSize.y) * 0.25f);
				ImGui::SetWindowPos(centerPos);
				ImGui::Text("Playlist Creator");
				ImGui::InputText("Playlist Name", playlistName, sizeof(playlistName));
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Text("Select Audio");
				ImGui::NewLine();
				ImGui::Spacing();
				for (std::set<Audio>::const_iterator it = assets.AudioSources.begin(); it != assets.AudioSources.end(); ++it)
				{
					ImGui::SameLine();
					if (ImGui::Button((it->first).c_str()))
					{
						auto selectedAudioIterator = std::find(assets.selectedAudio.begin(), assets.selectedAudio.end(), *it);
						if (selectedAudioIterator != assets.selectedAudio.end()) // Audio already exists in hypothetical playlist
							assets.selectedAudio.remove(*it);
						else
							assets.selectedAudio.push_back(*it);
					}
				}
				ImGui::NewLine();
				ImGui::Text("Selected Audio Sources");
				ImGui::NewLine();
				for (std::list<Audio>::const_iterator it = assets.selectedAudio.begin(); it != assets.selectedAudio.end(); ++it)
				{
					ImGui::SameLine();
					ImGui::Text(std::string(it->first + "  ").c_str());
				}
				if ((ImGui::Button("Confirm") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) && std::string(playlistName) != "Stop Audio Playback")
				{
					if (playlistName[0] != '\0' && !assets.selectedAudio.empty())
					{

						// Check if the playlist name is already used
						static std::set<std::string> usedPlaylistNames;
						if (usedPlaylistNames.find(playlistName) == usedPlaylistNames.end())
						{
							assets.PlayLists.push_back({ playlistName, assets.selectedAudio });
							assets.selectedAudio.clear();
							assets.ShowingPlaylistCreatorMenu = false;
							ImGui::CloseCurrentPopup();

							// Mark the playlist name as used
							usedPlaylistNames.insert(playlistName);
						}
						else
						{
							std::cout << "Playlist with the name '" << playlistName << "' already exists." << std::endl;
						}
					}
					else
					{
						std::cout << "Playlist requires a name and at least one audio source (name cannot be \"Stop Audio Playback\")" << std::endl;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					assets.selectedAudio.clear();
					assets.ShowingPlaylistCreatorMenu = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		if (ImGui::CollapsingHeader("Playlists"))
		{
			for (std::list<PlayList>::iterator it = assets.PlayLists.begin(); it != assets.PlayLists.end();)
				if (ImGui::CollapsingHeader(((*it).first).c_str()))
				{
					ImGui::Text("Playlist Audio");
					ImGui::NewLine();
					for (std::list<Audio>::iterator audioIt = it->second.begin(); audioIt != it->second.end(); ++audioIt)
					{
						ImGui::SameLine();
						if (ImGui::Button(((audioIt->first) + "  ").c_str()))
						{
							if (assets.CurrentBackgroudSound != nullptr)
								engine->stopAllSoundsOfSoundSource(assets.CurrentBackgroudSound);
							assets.currentPlaylist = it->first;
							assets.currentPlaylistAudio = audioIt->second;
							assets.CurrentBackgroudSound = audioIt->second;
							//std::cout << assets.CurrentBackgroudSound << std::endl;
							engine->play2D(audioIt->second);
						}
					}
					ImGui::NewLine;
					if (ImGui::Button("Stop Audio Playback"))
					{
						if (assets.CurrentBackgroudSound != nullptr)
						{
							engine->stopAllSoundsOfSoundSource(assets.CurrentBackgroudSound);
							assets.CurrentBackgroudSound = nullptr;
							assets.currentPlaylist = "";
							assets.currentPlaylistAudio = nullptr;
						}
					}
					ImGui::NewLine();
					if (ImGui::Button("Delete Playlist")) // Erase the playlist and get the next valid iterator
					{
						if (assets.currentPlaylist == it->first) // same playlist
						{
							if (assets.CurrentBackgroudSound != nullptr)
								engine->stopAllSoundsOfSoundSource(assets.CurrentBackgroudSound);
							assets.CurrentBackgroudSound = nullptr;
							assets.currentPlaylist = "";
							assets.currentPlaylistAudio = nullptr;
						}
						it = assets.PlayLists.erase(it);
					}
					else
					{
						++it;
					}
				}
				else
				{
					++it;
				}
		}
		if (!assets.currentPlaylist.empty() && assets.currentPlaylistAudio != nullptr)
		{
			std::cout << "Playing Playlist Audio" << std::endl;

			if (!engine->isCurrentlyPlaying(assets.currentPlaylistAudio->getName()))
			{
				for (auto playlistIterator = assets.PlayLists.begin(); playlistIterator != assets.PlayLists.end(); ++playlistIterator)
				{
					if (playlistIterator->first == assets.currentPlaylist)
					{
						PlayList currentPlaylist = *playlistIterator;

						for (auto audioIterator = currentPlaylist.second.begin(); audioIterator != currentPlaylist.second.end(); ++audioIterator)
						{
							if (audioIterator->second == assets.currentPlaylistAudio) // current audio found
							{
								auto nextAudio = std::next(audioIterator);
								if (nextAudio != currentPlaylist.second.end())
								{
									assets.CurrentBackgroudSound = assets.currentPlaylistAudio = nextAudio->second;
									engine->play2D(assets.currentPlaylistAudio);
								}
								else
								{
									assets.currentPlaylistAudio = nullptr;
									assets.currentPlaylist = "";
								}
								break;
							}
						}
						break;
					}
				}
			}
		}

	}
	ImGui::End();

}

void AssetMenu::importModel(const std::string& path, const std::string& texturePath)
{
	std::string label = path;
	int lastSlashPos = path.rfind('\\'); // Find the position of the last '/'
	int lastDotPos = path.rfind('.'); // Find the position of the last '.'
	if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
		label = path.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
	WO* wo = WO::New((path), Vector(1, 1, 1));
	wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	wo->upon_async_model_loaded([wo, texturePath]()
		{
			ModelMeshSkin skin(ManagerTex::loadTexAsync(texturePath).value());
			skin.setMeshShadingType(MESH_SHADING_TYPE::mstAUTO);
			skin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
			skin.setDiffuse(aftrColor4f(0.6f, 0.6f, 0.6f, 0.6f)); //Diffuse color components (ie, matte shading color of this object)
			skin.setSpecular(aftrColor4f(0.6f, 0.6f, 0.6f, 1.0f)); //Specular color component (ie, how "shiney" it is)
			skin.setSpecularCoefficient(1000); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
			wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(skin);
		});
	wo->setLabel(label);
	WorldObjects.push_back(wo);
	worldLst->push_back(wo);
}

void AssetMenu::importAudio(irrklang::ISoundEngine* engine, const char* soundFileName)
{
	std::string name(soundFileName);
	int lastSlashPos = name.rfind('\\'); // Find the position of the last '/'
	int lastDotPos = name.rfind('.'); // Find the position of the last '.'
	if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
		name = name.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
	if (name == "Stop Audio Playback")
		return;
	// Check if audio source already exists
	for (std::set<Audio>::iterator it = AudioSources.begin(); it != AudioSources.end(); ++it)
	{
		if ((*it).first == name)
			return;
	}
	AudioSources.insert(std::make_pair(name, engine->addSoundSourceFromFile(soundFileName)));
}
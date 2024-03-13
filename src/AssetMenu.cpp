#include "WO.h"
#include "irrklang.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "WOImGui.h"
#include "AftrImGuiIncludes.h"
#include "AftrImgui_Markdown_Renderer.h"
#include "WorldContainer.h"
#include <list>
#include <string>
#include "AssetMenu.h"
using namespace Aftr;

void AssetMenu::AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine, WorldContainer* worldLst)
{
	ImGui::SetWindowPos("AssetMenu", ImVec2(0, 0));
	static std::filesystem::path selected_path = "";
	static AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();
	static char playlistName[256] = {};
	static char label[256] = {};
	static std::string pathAsString = "";
	static ObjectandTexture asset = std::make_pair(std::make_pair("", ""), std::make_pair("", ""));
	static float position[3];
	ImGui::Begin("AssetMenu");
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !assets.ShowingPlaylistCreatorMenu)
			ImGui::SetWindowCollapsed(true);
		if (ImGui::Button("Import Asset"))
			gui->fileDialog_show_Open("Select A .obj .jpg or .wav File");
		if (gui->fileDialog_has_selected_path("Select A .obj .jpg or .wav File"))
		{
			selected_path = *gui->fileDialog_get_selected_path("Select A .obj .jpg or .wav File"); //can only get one time, this clears the dialog's state!
			pathAsString = selected_path.string();

			int pathLength = pathAsString.length(); // Prevents multiple identical function calls
			if (pathLength > 4) // Impossible path (5 characters is minimum path length)
			{
				std::string extension = pathAsString.substr(pathLength - 3);

				if (extension == "obj")
				{
					std::string label = pathAsString;
					int lastSlashPos = pathAsString.rfind('\\'); // Find the position of the last '/'
					int lastDotPos = pathAsString.rfind('.'); // Find the position of the last '.'
					if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
						label = pathAsString.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
					assets.objectsPaths.insert(std::make_pair(label, pathAsString)); // store the object to be imported fully after texturing
				}
				else if (extension == "wav")
					assets.importAudio(engine, pathAsString.c_str());
				else if (extension == "jpg")
				{
					std::string label = pathAsString;
					int lastSlashPos = pathAsString.rfind('\\'); // Find the position of the last '/'
					int lastDotPos = pathAsString.rfind('.'); // Find the position of the last '.'
					if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
						label = pathAsString.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
					assets.texturePaths.insert(std::make_pair(label, pathAsString)); // Store texture to skin object at a later time
				}
				else
					std::cerr << "Invalid File Extension" << std::endl;
			}
			else
				std::cerr << "Impossible File Path Length" << std::endl;
		}
		if (ImGui::CollapsingHeader("Object Files"))
		{
			for (std::set<std::pair<std::string, std::string>>::const_iterator it = assets.objectsPaths.begin(); it != assets.objectsPaths.end(); ++it)
				ImGui::Text(("    " + it->first).c_str());
		}
		if (ImGui::CollapsingHeader("Texture Files"))
		{
			for (std::set<std::pair<std::string, std::string>>::const_iterator it = assets.texturePaths.begin(); it != assets.texturePaths.end(); ++it)
				ImGui::Text(("    " + it->first).c_str());
		}
		if (ImGui::CollapsingHeader("Assets"))
		{
			if (ImGui::Button("Create Asset From Object And Texture"))
				assets.ShowingAssetCreatorMenu = true;
			for (std::set<ObjectandTexture>::iterator it = assets.texturedObjects.begin(); it != assets.texturedObjects.end(); ++it)
				ImGui::Text(("    " + it->first.first + "-" + it->second.first).c_str());
		}
		if (ImGui::CollapsingHeader("Instanced Assets"))
		{
			if (ImGui::Button("Instance Asset"))
			{
				std::memset(position, 0, 3);
				asset.first.first = asset.first.second = asset.second.first = asset.second.second = "";
				std::memset(label, '\0', sizeof(label));
				assets.ShowingInstanceObjectMenu = true;
			}
			ImGui::Text("SelectedInstance: ");
			ImGui::SameLine();
			assets.selectedInstance == nullptr ? ImGui::Text("No Instance Selected") : ImGui::Text((assets.selectedInstance->getLabel()).c_str());
			ImGui::Text("Modify Position");
			ImGui::InputFloat3(" ", position);
			ImGui::Text("Modify Rotation");
			if (ImGui::Button("45 DEG") && assets.selectedInstance != nullptr)
				assets.selectedInstance->rotateAboutGlobalZ(45 * DEGtoRAD);
			ImGui::SameLine();
			if (ImGui::Button("-45 DEG") && assets.selectedInstance != nullptr)
				assets.selectedInstance->rotateAboutGlobalZ(-45 * DEGtoRAD);
			if (ImGui::Button("Clear Selection"))
				assets.selectedInstance = nullptr;
			if (assets.selectedInstance != nullptr)
			{
				assets.selectedInstance->setPosition(Vector(position[0], position[1], position[2]));
			}
			for (std::list<WO*>::const_iterator it = assets.WorldObjects.begin(); it != assets.WorldObjects.end(); ++it)
				if (ImGui::Button(((*it)->getLabel()).c_str()))
				{
					assets.selectedInstance = *it;
					Vector pos = (*it)->getPosition();
					position[0] = pos[0];
					position[1] = pos[1];
					position[2] = pos[2];
				}
		}
		if (assets.ShowingAssetCreatorMenu)
		{
			ImGui::OpenPopup("Asset Creator");
			if (ImGui::BeginPopup("Asset Creator"))
			{
				static std::pair<std::string, std::string> selectedModel = std::make_pair("", "");
				static std::pair<std::string, std::string> selectedTexture = std::make_pair("", "");
				ImVec2 popupSize = ImGui::GetWindowSize();
				ImVec2 centerPos = ImVec2((ImGui::GetIO().DisplaySize.x - popupSize.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - popupSize.y) * 0.25f);
				ImGui::SetWindowPos(centerPos);
				ImGui::Text("Asset Creator");
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Text("Select Model");
				ImGui::NewLine();
				ImGui::Spacing();
				for (std::set<std::pair<std::string, std::string>>::const_iterator it = assets.objectsPaths.begin(); it != assets.objectsPaths.end(); ++it)
				{
					ImGui::SameLine();
					if (ImGui::Button((it->first).c_str()))
						selectedModel = *it;
				}
				ImGui::NewLine();
				ImGui::Text("Select Texture");
				ImGui::NewLine();
				ImGui::Spacing();
				for (std::set<std::pair<std::string, std::string>>::const_iterator it = assets.texturePaths.begin(); it != assets.texturePaths.end(); ++it)
				{
					ImGui::SameLine();
					if (ImGui::Button((it->first).c_str()))
						selectedTexture = *it;
				}

				ImGui::Text("Selected (Model, Texture) Pair");
				ImGui::SameLine();
				ImGui::Text((" (" + selectedModel.first + ", " + selectedTexture.first + ")").c_str());
				ImGui::NewLine();
				if ((ImGui::Button("Confirm") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))))
				{
					if (selectedModel.first != "" && selectedTexture.first != "")
					{
						assets.textureModel(selectedModel, selectedTexture);
						assets.ShowingAssetCreatorMenu = false;
						ImGui::CloseCurrentPopup();
					}
					else
						std::cout << "Assets requires a model and a texture" << std::endl;
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					assets.ShowingAssetCreatorMenu = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		if (assets.ShowingInstanceObjectMenu)
		{
			ImGui::OpenPopup("Instance Object Menu");
			if (ImGui::BeginPopup("Instance Object Menu"))
			{
				ImVec2 popupSize = ImGui::GetWindowSize();
				ImVec2 centerPos = ImVec2((ImGui::GetIO().DisplaySize.x - popupSize.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - popupSize.y) * 0.25f);
				ImGui::SetWindowPos(centerPos);
				ImGui::Text("Instance Object Menu");
				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
				ImGui::Text("Provide Label");
				ImGui::NewLine();
				ImGui::InputText("label", label, sizeof(label));
				ImGui::NewLine();
				ImGui::Text("Choose Object");
				ImGui::NewLine();
				for (std::set<ObjectandTexture>::iterator it = assets.texturedObjects.begin(); it != assets.texturedObjects.end(); ++it)
				{
					if (ImGui::Button((it->first.first + "-" + it->second.first).c_str()))
						asset = *it;
					ImGui::NewLine();
				}
				ImGui::Text("Provide position");
				ImGui::InputFloat3(" ", position);
				if ((ImGui::Button("Confirm") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))))
				{
					if (label[0] != '\0' && asset.first.second != "" && asset.second.second != "")
					{
						assets.instanceObject(label, asset, worldLst, Vector(position[0], position[1], position[2]));
						assets.ShowingInstanceObjectMenu = false;
						ImGui::CloseCurrentPopup();
					}
					else if (asset.first.second != "" && asset.second.second != "")
					{
						assets.instanceObject(std::string(asset.first.first + "-" + asset.second.first), asset, worldLst, Vector(position[0], position[1], position[2]));
						assets.ShowingInstanceObjectMenu = false;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					assets.ShowingInstanceObjectMenu = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
		if (ImGui::CollapsingHeader("Audio"))
		{
			if (ImGui::SliderFloat("Master Volume", &assets.masterVolume, 0, 1))
				engine->setSoundVolume(assets.masterVolume);
			for (std::set<Audio>::iterator it = assets.AudioSources.begin(); it != assets.AudioSources.end(); ++it)
				if (ImGui::Button(("    " + (*it).first).c_str()))
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
			if (ImGui::SliderFloat("Master Volume", &assets.masterVolume, 0, 1))
				engine->setSoundVolume(assets.masterVolume);
			if (ImGui::Button("Make Playlist"))
			{
				std::memset(playlistName, '\0', sizeof(playlistName));
				assets.selectedAudio.clear();
				assets.ShowingPlaylistCreatorMenu = true;
			}
			for (std::list<PlayList>::iterator it = assets.PlayLists.begin(); it != assets.PlayLists.end();)
				if (ImGui::CollapsingHeader(("    " + (*it).first).c_str()))
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

void AssetMenu::textureModel(const std::pair<std::string, std::string>& object, const std::pair<std::string, std::string>& texture)
{
	texturedObjects.insert(make_pair(object, texture));
}

void AssetMenu::instanceObject(const std::string& label, ObjectandTexture asset, WorldContainer* worldLst, const Vector& position)
{
	WO* wo = WO::New((asset.first.second), Vector(1, 1, 1));
	wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	wo->upon_async_model_loaded([wo, asset]()
		{
			ModelMeshSkin skin(ManagerTex::loadTexAsync(asset.second.second).value());
			skin.setMeshShadingType(MESH_SHADING_TYPE::mstAUTO);
			skin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
			skin.setDiffuse(aftrColor4f(0.6f, 0.6f, 0.6f, 0.6f)); //Diffuse color components (ie, matte shading color of this object)
			skin.setSpecular(aftrColor4f(0.6f, 0.6f, 0.6f, 1.0f)); //Specular color component (ie, how "shiney" it is)
			skin.setSpecularCoefficient(1000); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
			wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(skin);
		});
	wo->setLabel(label);
	wo->setPosition(position);
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
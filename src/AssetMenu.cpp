#include "WO.h"
#include "irrklang.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "WOImGui.h"
#include "AftrImGuiIncludes.h"
#include "AftrImgui_Markdown_Renderer.h"
#include "WorldContainer.h"
#include "NetMsg.h"
#include "NetMessengerClient.h"
#include "NetMsgImportObject.h"
#include "NetMsgImportTexture.h"
#include "NetMsgTextureModel.h"
#include "NetMsgInstanceAsset.h"
#include "NetMsgModifyPose.h"
#include <list>
#include <string>
#include <memory>
#include "AssetMenu.h"
#include "GLViewNewModule.h"
#include <fstream>
#include <array>
#include <algorithm>
using namespace Aftr;

void AssetMenu::AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine, WorldContainer* worldLst)
{
	// ImGui::SetWindowPos("AssetMenu", ImVec2(0, 0));
	static std::filesystem::path selected_path = "";
	static AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();
	static char playlistName[256] = {};
	static std::string pathAsString = "";
	static float position[3];
	ImGui::Begin("AssetMenu");
	{
		if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)) && !assets.ShowingPlaylistCreatorMenu)
			ImGui::SetWindowCollapsed(true);
		if (ImGui::Button("Import Asset"))
			gui->fileDialog_show_Open("Select A .obj .jpg or .wav File");
		if (gui->fileDialog_has_selected_path("Select A .obj .jpg or .wav File"))
		{
			selected_path = *gui->fileDialog_get_selected_path("Select A .obj .jpg or .wav File"); // can only get one time, this clears the dialog's state!
			pathAsString = selected_path.string();

			int pathLength = pathAsString.length(); // Prevents multiple identical function calls
			if (pathLength > 4)						// Impossible path (5 characters is minimum path length)
			{
				std::string extension = pathAsString.substr(pathLength - 3);

				if (extension == "obj")
				{
					std::string label = pathAsString;
					int lastSlashPos = pathAsString.rfind('\\'); // Find the position of the last '/'
					int lastDotPos = pathAsString.rfind('.');	 // Find the position of the last '.'
					if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
						label = pathAsString.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
					std::pair<std::string, std::string> obj = std::make_pair(label, pathAsString);
					assets.objectsPaths.insert(obj); // store the object to be imported fully after texturing
					std::shared_ptr<NetMsgImportObject> msg = std::make_shared<NetMsgImportObject>();
					msg->object = obj;
					if (assets.client != nullptr)
					{
						assets.client->sendNetMsgSynchronousTCP(*msg);
					}
					else
						assets.addNetMessage(msg);
				}
				else if (extension == "wav")
					assets.importAudio(engine, pathAsString.c_str());
				else if (extension == "jpg")
				{
					std::string label = pathAsString;
					int lastSlashPos = pathAsString.rfind('\\'); // Find the position of the last '/'
					int lastDotPos = pathAsString.rfind('.');	 // Find the position of the last '.'
					if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos)
						label = pathAsString.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
					std::pair<std::string, std::string> tex = std::make_pair(label, pathAsString);
					assets.texturePaths.insert(tex); // Store texture to skin object at a later time
					std::shared_ptr<NetMsgImportTexture> msg = std::make_shared<NetMsgImportTexture>();
					msg->texture = tex;
					if (assets.client != nullptr)
						assets.client->sendNetMsgSynchronousTCP(*msg);
					else
						assets.addNetMessage(msg);
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
			if (!assets.categorizedTexturedObjects.empty())
			{
				ImGui::Text(("    Category:"));
				for (std::list<std::pair<std::string, std::set<std::pair<ObjectandTexture, std::pair<int, int>>>>>::const_iterator it = assets.categorizedTexturedObjects.begin(); it != assets.categorizedTexturedObjects.end(); ++it)
				{
					ImGui::Text(("      " + it->first + ":").c_str());
					for (std::set<std::pair<ObjectandTexture, std::pair<int, int>>>::const_iterator items = it->second.begin(); items != it->second.end(); ++items)
					{
						ImGui::Text(("        " + items->first.first.first + "-" + items->first.second.first).c_str());
					}
				}
			}
		}
		if (ImGui::CollapsingHeader("Instanced Assets"))
		{
			if (ImGui::Button("Instance Asset"))
			{
				std::memset(position, 0, 3);
				assets.asset.first.first.first = assets.asset.first.first.second = assets.asset.first.second.first = assets.asset.first.second.second = "";
				std::memset(assets.label, '\0', sizeof(assets.label));
				assets.ShowingInstanceObjectMenu = true;
			}
			if (assets.selectedInstance != nullptr)
			{
				ImGui::NewLine();
				ImGui::Text("Modify Position");
				if (ImGui::InputFloat3(" ", position))
				{
					if (assets.selectedInstance != nullptr)
					{
						assets.selectedInstance->setPosition(Vector(position[0], position[1], position[2]));
						std::shared_ptr<NetMsgModifyPose> msg = std::make_shared<NetMsgModifyPose>();
						msg->label = assets.selectedInstance->getLabel();
						msg->position = assets.selectedInstance->getPosition();
						msg->pose = assets.selectedInstance->getPose();
						if (assets.client != nullptr)
							assets.client->sendNetMsgSynchronousTCP(*msg);
						else
							assets.addNetMessage(msg);
					}
				}
				ImGui::Text("Modify Rotation");
				{
					if (ImGui::Button("45 DEG(Z)") && assets.selectedInstance != nullptr)
					{
						assets.selectedInstance->rotateAboutGlobalZ(45 * DEGtoRAD);
						std::shared_ptr<NetMsgModifyPose> msg = std::make_shared<NetMsgModifyPose>();
						msg->label = assets.selectedInstance->getLabel();
						msg->position = assets.selectedInstance->getPosition();
						msg->pose = assets.selectedInstance->getPose();
						if (assets.client != nullptr)
							assets.client->sendNetMsgSynchronousTCP(*msg);
						else
							assets.addNetMessage(msg);
					}
					ImGui::SameLine();
					if (ImGui::Button("-45 DEG(Z)") && assets.selectedInstance != nullptr)
					{
						assets.selectedInstance->rotateAboutGlobalZ(-45 * DEGtoRAD);
						std::shared_ptr<NetMsgModifyPose> msg = std::make_shared<NetMsgModifyPose>();
						msg->label = assets.selectedInstance->getLabel();
						msg->position = assets.selectedInstance->getPosition();
						msg->pose = assets.selectedInstance->getPose();
						if (assets.client != nullptr)
							assets.client->sendNetMsgSynchronousTCP(*msg);
						else
							assets.addNetMessage(msg);
					}
					ImGui::NewLine();
				}
				/*
				if (ImGui::Button("Clear Selection"))
				assets.selectedInstance = nullptr;
				*/
			}
			if (ImGui::BeginCombo(" ", assets.selectedInstance == nullptr ? "Select Instance" : assets.selectedInstance->getLabel().c_str()))
			{
				for (std::list<WO*>::const_iterator it = assets.WorldObjects.begin(); it != assets.WorldObjects.end(); ++it)
				{
					if ((*it)->getLabel() != "PREVIEW")
					{
						if (ImGui::Selectable((*it)->getLabel().c_str()))
						{
							// Update selected instance and position directly
							assets.selectedInstance = *it;
							Vector pos = (*it)->getPosition();
							position[0] = pos[0];
							position[1] = pos[1];
							position[2] = pos[2];
						}
					}
				}
				ImGui::EndCombo();
			}
		}
		if (assets.ShowingAssetCreatorMenu)
		{
			ImGui::OpenPopup("Asset Creator");
			if (ImGui::BeginPopup("Asset Creator"))
			{
				static std::pair<std::string, std::string> selectedModel = std::make_pair("", "");
				static std::pair<std::string, std::string> selectedTexture = std::make_pair("", "");
				static char category[256] = "";
				ImVec2 popupSize = ImGui::GetWindowSize();
				ImVec2 centerPos = ImVec2((ImGui::GetIO().DisplaySize.x - popupSize.x) * 0.5f, (ImGui::GetIO().DisplaySize.y - popupSize.y) * 0.85f);
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
					{
						selectedModel = *it;
						assets.cancelPreview(worldLst);
					}
				}
				ImGui::NewLine();
				ImGui::Text("Select Texture");
				ImGui::NewLine();
				ImGui::Spacing();
				for (std::set<std::pair<std::string, std::string>>::const_iterator it = assets.texturePaths.begin(); it != assets.texturePaths.end(); ++it)
				{
					ImGui::SameLine();
					if (ImGui::Button((it->first).c_str()))
					{
						selectedTexture = *it;
						assets.cancelPreview(worldLst);
					}
				}
				ImGui::NewLine();
				ImGui::InputText("Category", category, sizeof(category));
				if (selectedModel.first != "" && selectedTexture.first != "" && category[0] != '\0')
				{
					assets.previewAsset(std::make_pair(selectedModel, selectedTexture), worldLst);
					ImGui::Text("Modify Default X and Y Rotation");
					if (ImGui::Button("45 DEG(X)"))
					{
						assets.previewInstance->rotateAboutGlobalX(45 * DEGtoRAD);
						assets.previewXYRotation = std::make_pair(assets.previewXYRotation.first + 45, assets.previewXYRotation.second);
					}
					ImGui::SameLine();
					if (ImGui::Button("-45 DEG(X)"))
					{
						assets.previewInstance->rotateAboutGlobalX(-45 * DEGtoRAD);
						assets.previewXYRotation = std::make_pair(assets.previewXYRotation.first - 45, assets.previewXYRotation.second);
					}
					if (ImGui::Button("45 DEG(Y)"))
					{
						assets.previewInstance->rotateAboutGlobalY(45 * DEGtoRAD);
						assets.previewXYRotation = std::make_pair(assets.previewXYRotation.first, assets.previewXYRotation.second + 45);
					}
					ImGui::SameLine();
					if (ImGui::Button("-45 DEG(Y)"))
					{
						assets.previewInstance->rotateAboutGlobalY(-45 * DEGtoRAD);
						assets.previewXYRotation = std::make_pair(assets.previewXYRotation.first, assets.previewXYRotation.second - 45);
					}
					if ((ImGui::Button("Confirm") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))))
					{
						assets.textureModel(selectedModel, selectedTexture, category, assets.previewXYRotation);

						std::shared_ptr<NetMsgTextureModel> msg = std::make_shared<NetMsgTextureModel>();
						msg->object = selectedModel;
						msg->texture = selectedTexture;
						strncpy(msg->category, category, sizeof(msg->category));
						msg->defaultRotation = assets.previewXYRotation;
						if (assets.client != nullptr)
							assets.client->sendNetMsgSynchronousTCP(*msg);
						else
							assets.addNetMessage(msg);
						assets.cancelPreview(worldLst);
						selectedModel = selectedTexture = std::make_pair("", "");
						std::memset(category, '\0', sizeof(category));
						assets.ShowingAssetCreatorMenu = false;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
					{
						assets.cancelPreview(worldLst);
						selectedModel = selectedTexture = std::make_pair("", "");
						std::memset(category, '\0', sizeof(category));
						assets.ShowingAssetCreatorMenu = false;
						ImGui::CloseCurrentPopup();
					}
				}
				else if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
				{
					assets.cancelPreview(worldLst);
					selectedModel = selectedTexture = std::make_pair("", "");
					std::memset(category, '\0', sizeof(category));
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
				ImGui::InputText("label", assets.label, sizeof(assets.label));
				ImGui::NewLine();
				ImGui::Text("Choose Object");
				ImGui::NewLine();
				if (!assets.categorizedTexturedObjects.empty())
				{
					ImGui::Text(("    Category:"));
					for (std::list<std::pair<std::string, std::set<std::pair<ObjectandTexture, std::pair<int, int>>>>>::const_iterator it = assets.categorizedTexturedObjects.begin(); it != assets.categorizedTexturedObjects.end(); ++it)
					{
						ImGui::Text(("      " + it->first + ":").c_str());
						for (std::set<std::pair<ObjectandTexture, std::pair<int, int>>>::const_iterator items = it->second.begin(); items != it->second.end(); ++items)
						{
							if (ImGui::Button(("        " + items->first.first.first + "-" + items->first.second.first).c_str()))
							{
								assets.asset.first = items->first;
								assets.asset.second = items->second;
							}
						}
					}
				}
				if (ImGui::Button(assets.isTile ? "Tile Selected" : "Non-Tile Selected"))
				{
					assets.isTile = !assets.isTile;
				}
				auto it = std::find_if(worldLst->begin(), worldLst->end(), [&](WO* obj) { return assets.labelMatches(assets.label, obj); });
				if (it == worldLst->end() && assets.asset.first.first.second != "")
				{
					if ((ImGui::Button("Confirm") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))))
					{
							//assets.instanceObject(label, asset.first, asset.second, worldLst, Vector(position[0], position[1], position[2]));
							/*std::shared_ptr<NetMsgInstanceAsset> msg = std::make_shared<NetMsgInstanceAsset>();
							msg->label = label;
							msg->asset = asset.first;
							msg->defaultRotation = asset.second;
							msg->position = position;
							if (assets.client != nullptr)
								assets.client->sendNetMsgSynchronousTCP(*msg);
							else
								assets.addNetMessage(msg);*/
							assets.placingAsset = true;
							assets.ShowingInstanceObjectMenu = false;
							ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
					{
						assets.ShowingInstanceObjectMenu = false;
						ImGui::CloseCurrentPopup();
					}
				}
				else
				{
					if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
					{
						assets.ShowingInstanceObjectMenu = false;
						ImGui::CloseCurrentPopup();
					}
				}
				ImGui::EndPopup();
			}
		}
		if (assets.assetPositionSelected && assets.placingAsset)
		{
			GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
			std::optional<Vector> position = glView->getLastSelectedCoordinate();
			std::optional<WO*> object = glView->getLastSelectedWO();
			if (object.has_value())
			{
				WO* wo = object.value();
				std::list<WO*>::const_iterator query;
				query = std::find_if(assets.WorldObjects.begin(), assets.WorldObjects.end(), [&](WO* woPtr) { return woPtr == object.value(); });
				if (query != assets.WorldObjects.end()) // Placed against an asset with support for chaining
				{
					if (position.has_value())
					{
						VectorT<float> objectPosition = wo->getPosition();
						float objectX = objectPosition.at(0);
						float objectY = objectPosition.at(1);
						float objectZ = objectPosition.at(2);
						float objectXRot = std::acos(wo->getModel()->getRelXDir().dotProduct((1, 0, 0)));
						float objectYRot = std::acos(wo->getModel()->getRelYDir().dotProduct((0, 1, 0)));
						float objectZRot = std::acos(wo->getModel()->getRelZDir().dotProduct((0, 0, 1)));
						if (assets.isTile)
						{
							Vector pos = position.value();
							float diffX = objectX - pos.at(0);
							float diffY = objectY - pos.at(1);
							if (fabs(diffX) > fabs(diffY)) // Place object with an offset in the x (y is flush)
							{
								float seperation = wo->getModel()->getBoundingBox().getlxlylz().at(0)/* * std::cos(objectYRot) * std::cos(objectZRot)*/;
								pos.at(0) = (pos.at(0) > objectX ? objectX + seperation : objectX - seperation);
								pos.at(1) = objectY;
								pos.at(2) = objectZ;
								assets.instanceObject(assets.label, assets.asset.first, assets.asset.second, worldLst, pos);
							}
							else // Place object with an offset in the y (x is flush)
							{
								float seperation = wo->getModel()->getBoundingBox().getlxlylz().at(1)/* * std::cos(objectXRot) * std::cos(objectZRot)*/;
								pos.at(0) = objectX;
								pos.at(1) = (pos.at(1) > objectY ? objectY + seperation : objectY - seperation);
								pos.at(2) = objectZ;
								assets.instanceObject(assets.label, assets.asset.first, assets.asset.second, worldLst, pos);
							}
						}
						else
						{
							
							float objectHeight = wo->getModel()->getBoundingBox().getlxlylz().at(2) * (std::cos(objectXRot) * std::cos(objectYRot));
							Vector pos = position.value();
							pos.at(0) = objectX;
							pos.at(1) = objectY;
							pos.at(2) = objectZ + objectHeight;
							assets.instanceObject(assets.label, assets.asset.first, assets.asset.second, worldLst, pos);
						}
					}
					assets.assetPositionSelected = false;
					assets.placingAsset = false;
				}
				else
				{
					if (position.has_value())
						assets.instanceObject(assets.label, assets.asset.first, assets.asset.second, worldLst, position.value());
					assets.assetPositionSelected = false;
					assets.placingAsset = false;
				}
			}
			if (position.has_value() && !object.value())			
				assets.instanceObject(assets.label, assets.asset.first, assets.asset.second, worldLst, position.value());
			assets.assetPositionSelected = false;
			assets.placingAsset = false;
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
					// std::cout << assets.CurrentBackgroudSound << std::endl;
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
							// std::cout << assets.CurrentBackgroudSound << std::endl;
							engine->play2D(audioIt->second);
						}
					}
					ImGui::NewLine();
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

void AssetMenu::textureModel(const std::pair<std::string, std::string>& object, const std::pair<std::string, std::string>& texture, const char(&category)[256], const std::pair<int, int>& defaultXYRotation)
{
	std::list<std::pair<std::string, std::set<std::pair<ObjectandTexture, std::pair<int, int>>>>>::iterator it;
	for (it = categorizedTexturedObjects.begin(); it != categorizedTexturedObjects.end(); ++it)
	{
		if (it->first == category)
			break;
	}
	if (it != categorizedTexturedObjects.end())
	{
		it->second.insert(make_pair(make_pair(object, texture), defaultXYRotation));
		return;
	}
	else
	{
		std::set<std::pair<ObjectandTexture, std::pair<int, int>>> addedCategorySet;
		addedCategorySet.insert(make_pair(make_pair(object, texture), defaultXYRotation));
		categorizedTexturedObjects.push_back(make_pair(category, addedCategorySet));
		return;
	}
}

void AssetMenu::instanceObject(const std::string& label, ObjectandTexture asset, std::pair<int, int> defaultXYRotation, WorldContainer* worldLst, const Vector& position)
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
			wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(skin); });
	wo->setLabel(label);
	wo->setPosition(position);
	wo->rotateAboutGlobalX(defaultXYRotation.first * DEGtoRAD);
	wo->rotateAboutGlobalY(defaultXYRotation.second * DEGtoRAD);
	WorldObjects.push_back(wo);
	worldLst->push_back(wo);
}

void AssetMenu::importAudio(irrklang::ISoundEngine* engine, const char* soundFileName)
{
	std::string name(soundFileName);
	int lastSlashPos = name.rfind('\\'); // Find the position of the last '/'
	int lastDotPos = name.rfind('.');	 // Find the position of the last '.'
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

void AssetMenu::modifyPose(const std::string& label, const Vector& position, const Mat4& pose)
{
	for (std::list<WO*>::iterator it = WorldObjects.begin(); it != WorldObjects.end(); ++it)
	{
		if (*(&(*it)->getLabel()) == label)
		{
			(*it)->setPosition(position);
			(*it)->setDisplayMatrix(pose);
			return;
		}
	}
}

void AssetMenu::pushAllMessages()
{
	if (client != nullptr)
	{
		if (netMessages.empty())
			return;
		for (std::list<std::shared_ptr<NetMsg>>::const_iterator it = netMessages.begin(); it != netMessages.end(); ++it)
		{
			(*it)->NetMsg::toString();
			client->sendNetMsgSynchronousTCP(**it);
		}
		netMessages.erase(netMessages.begin(), netMessages.end());
	}
}

void AssetMenu::previewAsset(ObjectandTexture asset, WorldContainer* worldLst)
{
	for (int i = 0; i < worldLst->size(); ++i)
	{
		if (worldLst->at(i)->getLabel() == "PREVIEW")
		{
			return;
		}
	}
	GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLViewT<GLViewNewModule>());
	Camera* cam = glView->getCamera();
	auto position = cam->getPosition() + cam->getLookDirection() * 5;

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
			wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(skin); });
	wo->setLabel("PREVIEW");
	wo->setPosition(position);
	previewInstance = wo;
	worldLst->push_back(previewInstance);
}

void AssetMenu::cancelPreview(WorldContainer* worldLst)
{
	if (previewInstance != nullptr)
	{
		previewXYRotation = std::make_pair(0, 0);
		worldLst->eraseViaWOptr(previewInstance);
		previewInstance = nullptr;
	}
}

void AssetMenu::saveAssets()
{
	std::fstream outfile;

	// object files
	outfile.open(ManagerEnvironmentConfiguration::getLMM() + "objects.dat", std::ios::out);
	for (std::set<std::pair<std::string, std::string>>::const_iterator it = objectsPaths.begin(); it != objectsPaths.end();)
	{
		auto curr = it;
		++it;
		if (it == objectsPaths.end())
		{
			outfile << curr->first << std::endl;
			outfile << curr->second;
		}
		else
		{
			outfile << curr->first << std::endl;
			outfile << curr->second << std::endl;
		}
	}
	outfile.close();

	// texture files
	outfile.open(ManagerEnvironmentConfiguration::getLMM() + "textures.dat", std::ios::out);
	for (std::set<std::pair<std::string, std::string>>::const_iterator it = texturePaths.begin(); it != texturePaths.end();)
	{
		auto curr = it;
		++it;
		if (it == texturePaths.end())
		{
			outfile << curr->first << std::endl;
			outfile << curr->second;
		}
		else
		{
			outfile << curr->first << std::endl;
			outfile << curr->second << std::endl;
		}
	}
	outfile.close();

	// stitched assets
	outfile.open(ManagerEnvironmentConfiguration::getLMM() + "stitchedAssets.dat", std::ios::out);
	size_t count = categorizedTexturedObjects.size(); // Get the total number of elements
	size_t current = 0;					   // Track the current element index

	for (std::list<std::pair<std::string, std::set<std::pair<ObjectandTexture, std::pair<int, int>>>>>::const_iterator it = categorizedTexturedObjects.begin(); it != categorizedTexturedObjects.end(); ++it)
	{
		for (std::set<std::pair<ObjectandTexture, std::pair<int, int>>>::const_iterator items = it->second.begin(); items != it->second.end(); ++items)
		{
			outfile << items->first.first.first << std::endl;
			outfile << items->first.first.second << std::endl;
			outfile << items->first.second.first << std::endl;
			outfile << items->first.second.second << std::endl;
			outfile << it->first << std::endl;
			outfile << items->second.first << std::endl;
			outfile << items->second.second;
		}

		if (++current < count)	  // Check if it's not the last element
			outfile << std::endl; // Add newline only if it's not the last element
	}
	outfile.close();
}

void AssetMenu::loadAssets()
{
	std::fstream infile;
	std::ifstream queryFile;

	// object files
	infile.open(ManagerEnvironmentConfiguration::getLMM() + "objects.dat", std::ios::in);
	std::array<std::string, 2> objectFileContents;
	while (!infile.eof())
	{
		for (int i = 0; i < 2; ++i)
		{
			if (infile.eof())
				return;
			std::getline(infile, objectFileContents[i]);
		}
		queryFile.open(objectFileContents[1]);
		if (queryFile.good() && objectFileContents[1].substr(objectFileContents[1].size() - 3) == "obj")
			importObjectPath(std::make_pair(objectFileContents[0], objectFileContents[1]));
		queryFile.close();
	}
	infile.close();

	// texture files
	infile.open(ManagerEnvironmentConfiguration::getLMM() + "textures.dat", std::ios::in);
	std::array<std::string, 2> textureFileContents;
	while (!infile.eof())
	{
		for (int i = 0; i < 2; ++i)
		{
			if (infile.eof())
				return;
			std::getline(infile, textureFileContents[i]);
		}
		queryFile.open(textureFileContents[1]);
		if (queryFile.good() && textureFileContents[1].substr(textureFileContents[1].size() - 3) == "jpg")
			importTexturePath(std::make_pair(textureFileContents[0], textureFileContents[1]));
		queryFile.close();
	}
	infile.close();

	// stitched assets
	infile.open(ManagerEnvironmentConfiguration::getLMM() + "stitchedAssets.dat", std::ios::in);
	std::string fromFile;
	std::array<std::string, 7> stitchedAssetContents;
	char category[256];
	while (!infile.eof())
	{
		for (int i = 0; i < 7; ++i)
		{
			if (infile.eof())
				return;
			std::getline(infile, stitchedAssetContents[i]);
		}
		strncpy(category, stitchedAssetContents[4].c_str(), sizeof(category));
		textureModel(std::make_pair(stitchedAssetContents[0], stitchedAssetContents[1]), std::make_pair(stitchedAssetContents[2], stitchedAssetContents[3]), category, std::make_pair(std::stoi(stitchedAssetContents[5]), std::stoi(stitchedAssetContents[6])));
	}
}

void AssetMenu::setLastSelectedInstance(WO* wo)
{
	std::list<WO*>::const_iterator it;
	it = find(WorldObjects.begin(), WorldObjects.end(), wo);
	if (it != WorldObjects.end())
		selectedInstance = wo;
}
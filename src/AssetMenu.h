#pragma once

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
#include <list>
#include <string>
using namespace Aftr;

typedef std::pair<std::string, irrklang::ISoundSource*> Audio; // Name of audio file, Source for play with 2D and 3D audio
typedef std::pair<std::string, std::list<Audio>> PlayList; // Name of playlist, Audio element
typedef std::pair<std::pair<std::string, std::string>, std::pair<std::string, std::string>> ObjectandTexture;

class AssetMenu
{
public:
	AssetMenu() { loadAssets(); }

	// GUI Functions

	static void AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine, WorldContainer* worldLst);

	// Asset Import/Build Functions

	void importObjectPath(const std::pair<std::string, std::string>& object) { this->objectsPaths.insert(object); }
	void importTexturePath(const std::pair<std::string, std::string>& texture) { this->texturePaths.insert(texture); }
	void modifyPose(const std::string& label, const Vector& position, const Mat4& pose);
	// Import model to be used as a WO
	void textureModel(const std::pair<std::string, std::string>& object, const std::pair<std::string, std::string>& texture, const char (& category)[256], const std::pair<int, int>& defaultXYRotation);
	// Push an object to the rendering worldLst and potentially label it
	void instanceObject(const std::string& label, ObjectandTexture asset, std::pair<int, int> defaultXYRotation, WorldContainer* worldLst, const Vector& position);
	// Import Audio to be used for 2D and 3D audio
	void importAudio(irrklang::ISoundEngine* engine, const char* soundFileName);
	// Add audio to a full asset
	void addAudio(FullAsset& fullAsset, Audio* audio) { fullAsset.second.push_back(audio); }
	void addNetMessage(std::shared_ptr<NetMsg> msg) { netMessages.push_back(msg); }
	void pushAllMessages();
	void previewAsset(ObjectandTexture asset, WorldContainer* worldLst);
	void cancelPreview(WorldContainer* worldLst);
	void saveAssets();
	void loadAssets();

	NetMessengerClient* client = nullptr;

protected:
	std::list<std::shared_ptr<NetMsg>> netMessages;
	std::string currentPlaylist = "";
	irrklang::ISoundSource* CurrentBackgroudSound = nullptr;
	irrklang::ISoundSource* currentPlaylistAudio = nullptr;
	bool ShowingPlaylistCreatorMenu = false;
	bool ShowingAssetCreatorMenu = false;
	bool ShowingInstanceObjectMenu = false;
	std::set<std::pair<std::string, std::string>> objectsPaths; // Label, path
	std::set<std::pair<std::string, std::string>> texturePaths; // Label, path
	std::list<std::pair<std::string, std::set<std::pair<ObjectandTexture, std::pair<int, int>>>>> categorizedTexturedObjects; // List of categories and pointers to textured objects in that category
	WO* selectedInstance = nullptr;
	WO* previewInstance = nullptr;
	std::pair<int, int> previewXYRotation = std::make_pair(0, 0);
	std::list<WO*> WorldObjects;
	std::set<Audio> AudioSources;
	std::list<PlayList> PlayLists;
	std::list<Audio> selectedAudio;
	float masterVolume = 1.0;
};
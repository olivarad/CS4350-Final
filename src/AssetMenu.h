#pragma once

#include "WO.h"
#include "irrklang.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "WOImGui.h"
#include "AftrImGuiIncludes.h"
#include "AftrImgui_Markdown_Renderer.h"
#include <list>
#include <string>
using namespace Aftr;

typedef std::pair<std::string, irrklang::ISoundSource*> Audio; // Name of audio file, Source for play with 2D and 3D audio
typedef std::pair < std::string, std::list<Audio>> PlayList; // Name of playlist, Audio element
typedef std::pair<WO*, std::list<Audio*>> FullAsset; // World object with all of its potential sounds

class AssetMenu
{
public:
	// GUI Functions

	static void AssetMenuGUI(WOImGui* gui, AssetMenu& assets, irrklang::ISoundEngine* engine);

	// Asset Import/Build Functions

	// Import model to be used as a WO
	void importModel(const std::string& path);
	// Import Audio to be used for 2D and 3D audio
	void importAudio(irrklang::ISoundEngine* engine, const char* soundFileName);
	// Creates a full asset with one audio source, more can be added later
	void makeFullAsset(WO* wo, Audio* audio) { FullAssets.push_back(std::make_pair(wo, std::list<Audio*> {audio})); }
	// Add audio to a full asset
	void addAudio(FullAsset& fullAsset, Audio* audio) { fullAsset.second.push_back(audio); }

protected:
	PlayList* currentPlaylist = nullptr;
	irrklang::ISoundSource* currentPlaylistAudio = nullptr;
	bool ShowingPlaylistCreatorMenu = false;
	irrklang::ISoundSource* CurrentBackgroudSound = nullptr;
	std::list<WO*> WorldObjects;
	std::set<Audio> AudioSources;
	std::list<PlayList> PlayLists;
	std::list<FullAsset> FullAssets;
	std::list<Audio> selectedAudio;
};
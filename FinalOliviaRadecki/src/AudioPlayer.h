#pragma once
#include <irrKlang.h>
#include "WOImGui.h"
#include "AssetMenu.h"
#include <list>
using namespace Aftr;

class AudioPlayer
{
public:
	// GUI Functions

	static void AudioPlayerGUI(WOImGui* gui, std::list<Audio>, irrklang::ISoundEngine* engine);

protected:

};
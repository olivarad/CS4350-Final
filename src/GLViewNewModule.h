#pragma once

#include "GLView.h"
#include "irrKlang.h"
#include "AssetMenu.h"
#include "AudioPlayer.h"

namespace Aftr
{
	class Camera;

	/**
	   \class GLViewNewModule
	   \author Scott Nykl
	   \brief A child of an abstract GLView. This class is the top-most manager of the module.

	   Read \see GLView for important constructor and init information.

	   \see GLView

		\{
	*/

	class GLViewNewModule : public GLView
	{
	public:
		static GLViewNewModule* New(const std::vector< std::string >& outArgs);
		virtual ~GLViewNewModule();
		virtual void updateWorld(); ///< Called once per frame
		virtual void loadMap(); ///< Called once at startup to build this module's scene
		virtual void createNewModuleWayPoints();
		virtual void onResizeWindow(GLsizei width, GLsizei height);
		virtual void onMouseDown(const SDL_MouseButtonEvent& e);
		virtual void onMouseUp(const SDL_MouseButtonEvent& e);
		virtual void onMouseMove(const SDL_MouseMotionEvent& e);
		virtual void onKeyDown(const SDL_KeyboardEvent& key);
		virtual void onKeyUp(const SDL_KeyboardEvent& key);
		virtual void moveCamera();
		irrklang::vec3df convertAftrVecToIrrklang(Vector vec) { return irrklang::vec3df(vec.x, vec.y, vec.z); } // Convert from Aftr vector of 3 floats to irrklang::vec3df
		std::string convertIrrklangvec3dfToString(irrklang::vec3df vec) { return  std::to_string(vec.X) + ", " + std::to_string(vec.Y) + ", " + std::to_string(vec.Z); }
	protected:
		GLViewNewModule(const std::vector< std::string >& args);
		virtual void onCreate();
		std::vector< std::string > skyBoxImageNames; //vector to store texture paths
		int currentSky = 0;
		std::vector<WO*> skyptrs;
		bool moveInputStates[6]; // wasdz"space", 1 is pressed and 0 is not pressed LCTRL is down space is up
		std::vector<WO*> GUIobjects; // Store objects to be used by GUI and their x, y, and z rotation values
		int objectSelector = 0; // GUI object selector
		bool rotationScaleSelector = 0; // 0 for local, 1 for global
		float oldLocalRotationValues[3] = { 0.0f, 0.0f, 0.0f };
		float oldGlobalRotationValues[3] = { 0.0f, 0.0f, 0.0f };
		float localRotationValues[3] = { 0.0f, 0.0f, 0.0f };
		float globalRotationValues[3] = { 0.0f, 0.0f, 0.0f };
		int placed = 1; // Number of cubes placed on camera (starts at one because a cube is initialized in the world upon running loadmap()

		irrklang::ISoundEngine* engine;
		std::vector<irrklang::ISound*> sounds;
		int soundSelector = 0;
		float masterVolume = 0.5f;

		AssetMenu assets;
		AudioPlayer audioPlayer;
	};

	/** \} */

} //namespace Aftr

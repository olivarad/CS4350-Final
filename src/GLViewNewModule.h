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

	class GLViewNewModule : public GLView , protected AssetMenu , public NetMsgImportObject
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
		/*
		void importNetMsgObject(const std::pair<std::string, std::string>& object) { objectsPaths.insert(object); }
		void importNetMsgTexture(const std::pair<std::string, std::string>& texture) { texturePaths.insert(texture); }
		void textureModelNetMsg(const std::pair<std::string, std::string>& object, const std::pair<std::string, std::string>& texture) { textureModel(object, texture); }
		void instanceObjectNetMsg(const std::string& label, ObjectandTexture asset, const Vector& position) { instanceObject(label, asset, worldLst, position); }
		void modifyPoseNetMsg(WO* wo, const Mat4& pose) { wo->setPose(pose); }
		*/

		AssetMenu assets;

	protected:
		GLViewNewModule(const std::vector< std::string >& args);
		virtual void onCreate();

		std::vector< std::string > skyBoxImageNames; //vector to store texture paths
		int currentSky = 0;
		std::vector<WO*> skyptrs;

		bool moveInputStates[6]; // wasdz"space", 1 is pressed and 0 is not pressed LCTRL is down space is up

		irrklang::ISoundEngine* engine;

		std::pair<std::string, std::string> currentObject; // Label, Path
	};

	/** \} */

} //namespace Aftr

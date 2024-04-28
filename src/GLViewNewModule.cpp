#include "GLViewNewModule.h"

#include "WorldList.h" //This is where we place all of our WOs
#include "ManagerOpenGLState.h" //We can change OpenGL State attributes with this
#include "Axes.h" //We can set Axes to on/off with this
#include "PhysicsEngineODE.h"

//Different WO used by this module
#include "WO.h"
#include "WOStatic.h"
#include "WOStaticPlane.h"
#include "WOStaticTrimesh.h"
#include "WOTrimesh.h"
#include "WOHumanCyborg.h"
#include "WOHumanCal3DPaladin.h"
#include "WOWayPointSpherical.h"
#include "WOLight.h"
#include "WOSkyBox.h"
#include "WOCar1970sBeater.h"
#include "Camera.h"
#include "CameraStandard.h"
#include "CameraChaseActorSmooth.h"
#include "CameraChaseActorAbsNormal.h"
#include "CameraChaseActorRelNormal.h"
#include "Model.h"
#include "ModelDataShared.h"
#include "ModelMesh.h"
#include "ModelMeshDataShared.h"
#include "ModelMeshSkin.h"
#include "WONVStaticPlane.h"
#include "WONVPhysX.h"
#include "WONVDynSphere.h"
#include "WOImGui.h" //GUI Demos also need to #include "AftrImGuiIncludes.h"
#include "AftrImGuiIncludes.h"
#include "AftrGLRendererBase.h"
#include "irrKlang.h"
#include "AftrImGui_Markdown_Renderer.h"

//Custom
#include "AssetMenu.h"
#include "NetMsgImportObject.h"
#include "NetMsgImportTexture.h"
#include "NetMsgTextureModel.h"
#include "NetMsgInstanceAsset.h"
#include "NetMsgModifyPose.h"
#include "NetMsgDeleteAsset.h"
using namespace Aftr;

GLViewNewModule* GLViewNewModule::New(const std::vector< std::string >& args)
{
	GLViewNewModule* glv = new GLViewNewModule(args);
	glv->init(Aftr::GRAVITY, Vector(0, 0, -1.0f), "aftr.conf", PHYSICS_ENGINE_TYPE::petODE);
	glv->onCreate();
	return glv;
}


GLViewNewModule::GLViewNewModule(const std::vector< std::string >& args) : GLView(args)
{
	//Initialize any member variables that need to be used inside of LoadMap() here.
	//Note: At this point, the Managers are not yet initialized. The Engine initialization
	//occurs immediately after this method returns (see GLViewNewModule::New() for
	//reference). Then the engine invoke's GLView::loadMap() for this module.
	//After loadMap() returns, GLView::onCreate is finally invoked.

	//The order of execution of a module startup:
	//GLView::New() is invoked:
	//    calls GLView::init()
	//       calls GLView::loadMap() (as well as initializing the engine's Managers)
	//    calls GLView::onCreate()

	//GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
}


void GLViewNewModule::onCreate()
{
	//GLViewNewModule::onCreate() is invoked after this module's LoadMap() is completed.
	//At this point, all the managers are initialized. That is, the engine is fully initialized.

	if (this->pe != NULL)
	{
		//optionally, change gravity direction and magnitude here
		//The user could load these values from the module's aftr.conf
		this->pe->setGravityNormalizedVector(Vector(0, 0, -1.0f));
		this->pe->setGravityScalar(Aftr::GRAVITY);
	}
	this->setActorChaseType(STANDARDEZNAV); //Default is STANDARDEZNAV mode
	//this->setNumPhysicsStepsPerRender( 0 ); //pause physics engine on start up; will remain paused till set to 1
}


GLViewNewModule::~GLViewNewModule()
{
	assets.saveAssets();
	//Implicitly calls GLView::~GLView()
}


void GLViewNewModule::updateWorld()
{
	GLView::updateWorld(); //Just call the parent's update world first.
	//If you want to add additional functionality, do it after
	//this call.
	if (moveInputStates[0] == 1 || moveInputStates[1] == 1 || moveInputStates[2] == 1 || moveInputStates[3] == 1 || moveInputStates[4] == 1 || moveInputStates[5] == 1)
	{
		moveCamera();
	}

	engine->setListenerPosition(convertAftrVecToIrrklang(this->cam->getPosition()), convertAftrVecToIrrklang(this->cam->getLookDirection()), convertAftrVecToIrrklang(this->cam->getCameraVelocity()), irrklang::vec3df(0, 1, 0));
}

void GLViewNewModule::moveCamera() {
	if (moveInputStates[0] == 1) // W
	{
		this->cam->moveInLookDirection();
	}
	if (moveInputStates[1] == 1) // A
	{
		this->cam->moveLeft();
	}
	if (moveInputStates[2] == 1) // S
	{
		this->cam->moveRight();
	}
	if (moveInputStates[3] == 1) // D
	{
		this->cam->moveOppositeLookDirection();
	}
	if (moveInputStates[4] == 1) // CTRL
	{
		this->cam->moveRelative(Vector(0.0f, 0.0f, -0.25f));
	}
	if (moveInputStates[5] == 1) // SPACE
	{
		this->cam->moveRelative(Vector(0.0f, 0.0f, 0.25f));
	}
}

void GLViewNewModule::onResizeWindow(GLsizei width, GLsizei height)
{
	GLView::onResizeWindow(width, height); //call parent's resize method.
}

void GLViewNewModule::onMouseDown(const SDL_MouseButtonEvent& e)
{
	if (e.button == SDL_BUTTON_LEFT)
	{
		GLView::onMouseDown(e);
		unsigned int x, y = 0;
		this->mouseHandler.getMouseDownPosition(x, y); //get pixel use clicked on
		this->getCamera()->updateProjectionMatrix(); //onMouseDownSelection may need the latest projection matrix in this camera renderSelect
		this->getCamera()->updateViewMatrix(); //onMouseDownSelection may need the latest view matrix in this camera for renderSelect
		this->onMouseDownSelection(x, y, *this->getCamera()); //see if a WO was drawn on that pixel
		if (assets.getPlacingAsset())
		{
			if (this->getLastSelectedWO() != nullptr)
			{
				assets.setLastSelectedInstance(this->getLastSelectedWO());
				assets.setAssetPositionSelected();
			}
		}
		else 
		{
			if (this->getLastSelectedWO() != nullptr)
			{
				assets.setLastSelectedInstance(this->getLastSelectedWO());
			}
		}
	}
	else if (e.button == SDL_BUTTON_RIGHT)
	{
		assets.resetPlacingAsset();
	}
}

void GLViewNewModule::onMouseUp(const SDL_MouseButtonEvent& e)
{
	GLView::onMouseUp(e);
}

void GLViewNewModule::onMouseMove(const SDL_MouseMotionEvent& e)
{
	GLView::onMouseMove(e);
}


void GLViewNewModule::onKeyDown(const SDL_KeyboardEvent& key)
{
	GLView::onKeyDown(key);
	if (key.keysym.sym == SDLK_0)
		this->setNumPhysicsStepsPerRender(1);
	// Left arrow key pressed
	if (key.keysym.sym == SDLK_LEFT)
	{
		worldLst->eraseViaWOptr(skyptrs[currentSky]);
		currentSky = (currentSky - 1 < 0) ? (skyBoxImageNames.size() - 1) : (currentSky - 1);
		worldLst->push_back(skyptrs[currentSky]);
	}
	if (key.keysym.sym == SDLK_RIGHT)
	{
		worldLst->eraseViaWOptr(skyptrs[currentSky]);
		currentSky = (currentSky + 1 == skyBoxImageNames.size()) ? (0) : (currentSky + 1);
		worldLst->push_back(skyptrs[currentSky]);
	}
	if (key.keysym.sym == SDLK_w)
	{
		moveInputStates[0] = 1;
	}
	if (key.keysym.sym == SDLK_a)
	{
		moveInputStates[1] = 1;
	}
	if (key.keysym.sym == SDLK_d)
	{
		moveInputStates[2] = 1;
	}
	if (key.keysym.sym == SDLK_s)
	{
		moveInputStates[3] = 1;
	}
	if (key.keysym.sym == SDLK_LCTRL)
	{
		moveInputStates[4] = 1;
	}
	if (key.keysym.sym == SDLK_SPACE)
	{
		moveInputStates[5] = 1;
	}
	if (key.keysym.sym == SDLK_LSHIFT)
	{
		this->cam->setPosition(15, 15, 10); // return to original position
		this->cam->setCameraLookAtPoint(Vector(16.0f, 15.0f, 10.0f));
		this->cam->setCameraNormalDirection(Vector(0.0f, 0.0f, 1.0f));
		std::cout << "Normal Direction " << this->cam->getNormalDirection().normalizeMe() << std::endl;
		std::cout << "Looking At " << this->cam->getCameraLookAtPoint() << std::endl;
	}
	if (key.keysym.sym == SDLK_g)
	{
		std::cout << "Engine Camera Position: " << this->cam->getPosition() << std::endl;
		std::cout << "IrrKlang Converted Camera Position: " << convertIrrklangvec3dfToString(convertAftrVecToIrrklang(this->cam->getPosition())) << std::endl;
	}
	if (key.keysym.sym == SDLK_9)
	{
		assets.client = NetMessengerClient::New("127.0.0.1", ManagerEnvironmentConfiguration::getVariableValue("NetServerTransmitPort"));
		assets.pushAllMessages();
	}
}

void GLViewNewModule::onKeyUp(const SDL_KeyboardEvent& key)
{
	GLView::onKeyUp(key);
	if (key.keysym.sym == SDLK_w)
	{
		moveInputStates[0] = 0;
	}
	if (key.keysym.sym == SDLK_a)
	{
		moveInputStates[1] = 0;
	}
	if (key.keysym.sym == SDLK_d)
	{
		moveInputStates[2] = 0;
	}
	if (key.keysym.sym == SDLK_s)
	{
		moveInputStates[3] = 0;
	}
	if (key.keysym.sym == SDLK_LCTRL)
	{
		moveInputStates[4] = 0;
	}
	if (key.keysym.sym == SDLK_SPACE)
	{
		moveInputStates[5] = 0;
	}
}

void Aftr::GLViewNewModule::loadMap()
{

	GLView::subscribe_NetMsg_to_callback<NetMsgImportObject>([this](auto msg) {msg->onMessageArrived(); });
	GLView::subscribe_NetMsg_to_callback<NetMsgImportTexture>([this](auto msg) {msg->onMessageArrived(); });
	GLView::subscribe_NetMsg_to_callback<NetMsgTextureModel>([this](auto msg) {msg->onMessageArrived(); });
	GLView::subscribe_NetMsg_to_callback<NetMsgInstanceAsset>([this](auto msg) {msg->onMessageArrived(); });
	GLView::subscribe_NetMsg_to_callback<NetMsgModifyPose>([this](auto msg) {msg->onMessageArrived(); });
	GLView::subscribe_NetMsg_to_callback<NetMsgDeleteAsset>([this](auto msg) {msg->onMessageArrived(); });

	this->worldLst = new WorldList(); //WorldList is a 'smart' vector that is used to store WO*'s
	std::cout << "WorldLst: " << worldLst << std::endl;
	this->actorLst = new WorldList();
	this->netLst = new WorldList();

	engine = irrklang::createIrrKlangDevice();
	engine->setSoundVolume(1.0);
	ManagerOpenGLState::GL_CLIPPING_PLANE = 1000.0;
	ManagerOpenGLState::GL_NEAR_PLANE = 0.1f;
	ManagerOpenGLState::enableFrustumCulling = false;
	Axes::isVisible = true;
	this->glRenderer->isUsingShadowMapping(false); //set to TRUE to enable shadow mapping, must be using GL 3.2+

	this->cam->setPosition(15, 15, 10);

	std::string grass(ManagerEnvironmentConfiguration::getSMM() + "/models/grassFloor400x400_pp.wrl");

	//SkyBox Textures readily available
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_water+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_dust+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_mountains+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_winter+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/early_morning+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_afternoon+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_cloudy3+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_day2+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_deepsun+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_evening+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_morning2+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_noon+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/sky_warp+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_Hubble_Nebula+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_gray_matter+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_easter+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_hot_nebula+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_ice_field+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_lemon_lime+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_milk_chocolate+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_solar_bloom+6.jpg");
	skyBoxImageNames.push_back(ManagerEnvironmentConfiguration::getSMM() + "/images/skyboxes/space_thick_rb+6.jpg");

	{
		//Create a light
		float ga = 0.1f; //Global Ambient Light level for this module
		ManagerLight::setGlobalAmbientLight(aftrColor4f(ga, ga, ga, 1.0f));
		WOLight* light = WOLight::New();
		light->isDirectionalLight(true);
		light->setPosition(Vector(0, 0, 100));
		//Set the light's display matrix such that it casts light in a direction parallel to the -z axis (ie, downwards as though it was "high noon")
		//for shadow mapping to work, this->glRenderer->isUsingShadowMapping( true ), must be invoked.
		light->getModel()->setDisplayMatrix(Mat4::rotateIdentityMat({ 0, 1, 0 }, 90.0f * Aftr::DEGtoRAD));
		light->setLabel("Light");
		worldLst->push_back(light);
	}

	{
		//Create the SkyBox
		for (int i = 0; i < skyBoxImageNames.size(); i++) {

			WO* wo = WOSkyBox::New(skyBoxImageNames.at(i), this->getCameraPtrPtr());
			wo->setPosition(Vector(0, 0, 0));
			wo->setLabel("Sky Box");
			wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
			skyptrs.push_back(wo);
		}
		worldLst->push_back(skyptrs[0]);
	}

	{
		////Create the infinite grass plane (the floor)
		WO* wo = WO::New(grass, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				ModelMeshSkin& grassSkin = wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0);
				grassSkin.getMultiTextureSet().at(0).setTexRepeats(5.0f);
				grassSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
				grassSkin.setDiffuse(aftrColor4f(1.0f, 1.0f, 1.0f, 1.0f)); //Diffuse color components (ie, matte shading color of this object)
				grassSkin.setSpecular(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Specular color component (ie, how "shiney" it is)
				grassSkin.setSpecularCoefficient(10); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)
			});
		wo->setLabel("Grass");
		worldLst->push_back(wo);
	}

	// Lizard
	/*
	{
		WO* wo = WO::New((ManagerEnvironmentConfiguration::getLMM() + "/models/LizardMage/LizardMage_Lowpoly.obj"), Vector(1, 1, 1));
		wo->setPosition(Vector(0, 10, 2));
		wo->rotateAboutRelZ(90 * Aftr::DEGtoRAD);
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->upon_async_model_loaded([wo]()
			{
				std::string green(ManagerEnvironmentConfiguration::getLMM() + "/models/LizardMage/Body_colors1.jpg");
				ModelMeshSkin lizardSkin(ManagerTex::loadTexAsync(green).value());
				lizardSkin.setMeshShadingType(MESH_SHADING_TYPE::mstAUTO);
				lizardSkin.setAmbient(aftrColor4f(0.4f, 0.4f, 0.4f, 1.0f)); //Color of object when it is not in any light
				lizardSkin.setDiffuse(aftrColor4f(0.6f, 0.6f, 0.6f, 0.6f)); //Diffuse color components (ie, matte shading color of this object)
				lizardSkin.setSpecular(aftrColor4f(0.6f, 0.6f, 0.6f, 1.0f)); //Specular color component (ie, how "shiney" it is)
				lizardSkin.setSpecularCoefficient(1000); // How "sharp" are the specular highlights (bigger is sharper, 1000 is very sharp, 10 is very dull)

				wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0) = std::move(lizardSkin);
			});
		wo->setLabel("Lizard Wizard");
		worldLst->push_back(wo);
		GUIobjects.push_back(wo);
	} 
	*/

	// Create shiny red plastic cube
	/*
	{
		WO* wo = WO::New(shinyRedPlasticCube, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(0, 0, 0));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->setLabel("ShinyRedPlasticCube");
		worldLst->push_back(wo);
		GUIobjects.push_back(wo);
	}
	*/

	/*
	{
		WO* wo = WO::New(wheeledCar, Vector(1, 1, 1), MESH_SHADING_TYPE::mstFLAT);
		wo->setPosition(Vector(10, 10, 10));
		wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
		wo->setLabel("WheeledCar");
		worldLst->push_back(wo);
		GUIobjects.push_back(wo);
	}
	*/

	//{
	//   //Create the infinite grass plane that uses the Open Dynamics Engine (ODE)
	//   WO* wo = WOStatic::New( grass, Vector(1,1,1), MESH_SHADING_TYPE::mstFLAT );
	//   ((WOStatic*)wo)->setODEPrimType( ODE_PRIM_TYPE::PLANE );
	//   wo->setPosition( Vector(0,0,0) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->getModel()->getModelDataShared()->getModelMeshes().at(0)->getSkins().at(0).getMultiTextureSet().at(0)->setTextureRepeats( 5.0f );
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   //Create the infinite grass plane that uses NVIDIAPhysX(the floor)
	//   WO* wo = WONVStaticPlane::New( grass, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0, 0 ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->getModel()->getModelDataShared()->getModelMeshes().at( 0 )->getSkins().at( 0 ).getMultiTextureSet().at( 0 )->setTextureRepeats( 5.0f );
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   //Create the infinite grass plane (the floor)
	//   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0, 50.0f ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	//   WO* wo = WONVPhysX::New( shinyRedPlasticCube, Vector( 1, 1, 1 ), MESH_SHADING_TYPE::mstFLAT );
	//   wo->setPosition( Vector( 0, 0.5f, 75.0f ) );
	//   wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
	//   wo->setLabel( "Grass" );
	//   worldLst->push_back( wo );
	//}

	//{
	   //WO* wo = WONVDynSphere::New( ManagerEnvironmentConfiguration::getVariableValue( "sharedmultimediapath" ) + "/models/sphereRp5.wrl", Vector( 1.0f, 1.0f, 1.0f ), mstSMOOTH );
	   //wo->setPosition( 0, 0, 100.0f );
	   //wo->setLabel( "Sphere" );
	   //this->worldLst->push_back( wo );
	//}

	//{
	//   WO* wo = WOHumanCal3DPaladin::New( Vector( .5, 1, 1 ), 100 );
	//   ((WOHumanCal3DPaladin*)wo)->rayIsDrawn = false; //hide the "leg ray"
	//   ((WOHumanCal3DPaladin*)wo)->isVisible = false; //hide the Bounding Shell
	//   wo->setPosition( Vector( 20, 20, 20 ) );
	//   wo->setLabel( "Paladin" );
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo );
	//   netLst->push_back( wo );
	//   this->setActor( wo );
	//}
	//
	//{
	//   WO* wo = WOHumanCyborg::New( Vector( .5, 1.25, 1 ), 100 );
	//   wo->setPosition( Vector( 20, 10, 20 ) );
	//   wo->isVisible = false; //hide the WOHuman's bounding box
	//   ((WOHuman*)wo)->rayIsDrawn = false; //show the 'leg' ray
	//   wo->setLabel( "Human Cyborg" );
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo ); //Push the WOHuman as an actor
	//   netLst->push_back( wo );
	//   this->setActor( wo ); //Start module where human is the actor
	//}

	//{
	//   //Create and insert the WOWheeledVehicle
	//   std::vector< std::string > wheels;
	//   std::string wheelStr( "../../../shared/mm/models/WOCar1970sBeaterTire.wrl" );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   wheels.push_back( wheelStr );
	//   WO* wo = WOCar1970sBeater::New( "../../../shared/mm/models/WOCar1970sBeater.wrl", wheels );
	//   wo->setPosition( Vector( 5, -15, 20 ) );
	//   wo->setLabel( "Car 1970s Beater" );
	//   ((WOODE*)wo)->mass = 200;
	//   worldLst->push_back( wo );
	//   actorLst->push_back( wo );
	//   this->setActor( wo );
	//   netLst->push_back( wo );
	//}

	//Make a Dear Im Gui instance via the WOImGui in the engine... This calls
	//the default Dear ImGui demo that shows all the features... To create your own,
	//inherit from WOImGui and override WOImGui::drawImGui_for_this_frame(...) (among any others you need).

	WOImGui* gui = WOImGui::New(nullptr);
	gui->setLabel("Olivia's GUI");
	gui->subscribe_drawImGuiWidget(
		[this, gui]() //this is a lambda, the capture clause is in [], the input argument list is in (), and the body is in {}
		{
			assets.AssetMenuGUI(gui, assets, engine, worldLst);
			//audioPlayer.AudioPlayerGUI(gui, assets.AudioSources, engine);
			/*
			{
				ImGui::Text("Object Selector");
				ImGui::SliderInt(("Object: " + std::string(GUIobjects[objectSelector]->getLabel())).c_str(), &objectSelector, 0, GUIobjects.size() - 1);
				ImGui::Checkbox("Global Rotation?", &rotationScaleSelector);
				ImGui::Text(rotationScaleSelector == 0 ? "Local Rotation Selected" : "Global Rotation Selected");
			}
			*/
			/*
			{
				if (ImGui::SliderFloat((rotationScaleSelector == 0 ? "Local x Rotation" : "Global x Rotation"), rotationScaleSelector == 0 ? &localRotationValues[0] : &globalRotationValues[0], -360.0f, 360.0f)) {
					rotationScaleSelector == 0 ? GUIobjects[objectSelector]->rotateAboutRelX((localRotationValues[0] - oldLocalRotationValues[0]) * PI / 180) : GUIobjects[objectSelector]->rotateAboutGlobalX((globalRotationValues[0] - oldGlobalRotationValues[0]) * PI / 180);
					rotationScaleSelector == 0 ? oldLocalRotationValues[0] = localRotationValues[0] : oldGlobalRotationValues[0] = globalRotationValues[0];
				}
				if (ImGui::SliderFloat((rotationScaleSelector == 0 ? "Local y Rotation" : "Global y Rotation"), rotationScaleSelector == 0 ? &localRotationValues[1] : &globalRotationValues[1], -360.0f, 360.0f)) {
					rotationScaleSelector == 0 ? GUIobjects[objectSelector]->rotateAboutRelY((localRotationValues[1] - oldLocalRotationValues[1]) * PI / 180) : GUIobjects[objectSelector]->rotateAboutGlobalY((globalRotationValues[1] - oldGlobalRotationValues[1]) * PI / 180);
					rotationScaleSelector == 0 ? oldLocalRotationValues[1] = localRotationValues[1] : oldGlobalRotationValues[1] = globalRotationValues[1];
				}
				if (ImGui::SliderFloat((rotationScaleSelector == 0 ? "Local z Rotation" : "Global z Rotation"), rotationScaleSelector == 0 ? &localRotationValues[2] : &globalRotationValues[2], -360.0f, 360.0f)) {
					rotationScaleSelector == 0 ? GUIobjects[objectSelector]->rotateAboutRelZ((localRotationValues[2] - oldLocalRotationValues[2]) * PI / 180) : GUIobjects[objectSelector]->rotateAboutGlobalZ((globalRotationValues[2] - oldGlobalRotationValues[2]) * PI / 180);
					rotationScaleSelector == 0 ? oldLocalRotationValues[2] = localRotationValues[2] : oldGlobalRotationValues[2] = globalRotationValues[2];
				}
			}
			*/
			/*
			{
				if (ImGui::Button("Move Negative x")) {
					GUIobjects[objectSelector]->moveRelative(Vector(-1.0f, 0.0f, 0.0f));
				}
				if (ImGui::Button("Move Positive x")) {
					GUIobjects[objectSelector]->moveRelative(Vector(1.0f, 0.0f, 0.0f));
				}
				if (ImGui::Button("Move Negative y")) {
					GUIobjects[objectSelector]->moveRelative(Vector(0.0f, -1.0f, 0.0f));
				}
				if (ImGui::Button("Move Positive y")) {
					GUIobjects[objectSelector]->moveRelative(Vector(0.0f, 1.0f, 0.0f));
				}
				if (ImGui::Button("Move Negative z")) {
					GUIobjects[objectSelector]->moveRelative(Vector(0.0f, 0.0f, -1.0f));
				}
				if (ImGui::Button("Move Positive z")) {
					GUIobjects[objectSelector]->moveRelative(Vector(0.0f, 0.0f, 1.0f));
				}
			}
			*/
			/*
			{
				if (ImGui::SliderFloat("Master Volume", &masterVolume, 0.0f, 1.0f)) {
					engine->setSoundVolume(masterVolume);
				}
				ImGui::SliderInt("Audio Selector", &soundSelector, 0, sounds.size() - 1);
				if (ImGui::Button("Play/Pause")) {
					sounds[soundSelector]->getIsPaused() == true ? sounds[soundSelector]->setIsPaused(false) : sounds[soundSelector]->setIsPaused(true);
				}
			}
			*/
			/*
			{
				static std::filesystem::path selected_path;
				static std::string origin_fileDialog;
				static AftrImGui_Markdown_Renderer md_render = Aftr::make_default_MarkdownRenderer();

				//if (ImGui::Button("Select a directory!"))
				//	gui->fileDialog_show_SelectDirectory("Select Directory"); //The window title is a key, that must match fileDialog_has/get...( title );
				//if (gui->fileDialog_has_selected_path("Select Directory")) {
				//	selected_path = *gui->fileDialog_get_selected_path("Select Directory"); //can only get one time, this clears the dialog's state!
				//	origin_fileDialog = "Select Directory: ";
				//}

				if (ImGui::Button("Open a file!"))
					gui->fileDialog_show_Open("Choose File##MyUniqueLabel_Open");
				if (gui->fileDialog_has_selected_path("Choose File##MyUniqueLabel_Open")) {
					 selected_path = *gui->fileDialog_get_selected_path("Choose File##MyUniqueLabel_Open"); //can only get one time, this clears the dialog's state!
					origin_fileDialog = "Choose File##MyUniqueLabel_Open: ";
					if (origin_fileDialog.length() < 3) {
						std::cout << "String is too short to extract file extension" << std::endl;
					}
					std::string extension = selected_path.string().substr(selected_path.string().length() - 3);
					if (extension == "wav") {
						irrklang::ISound* sound = engine->play2D((selected_path.string().c_str()), true, true);
						sounds.push_back(sound);
					}
					else if (extension == "obj") {
						
						WO* wo = WO::New((selected_path.string()), Vector(1, 1, 1));
						wo->setPosition(Vector(0, 0, 0));
						wo->renderOrderType = RENDER_ORDER_TYPE::roOPAQUE;
						std::string pathToString = selected_path.string();

						int lastSlashPos = pathToString.rfind('\\'); // Find the position of the last '/'
						int lastDotPos = pathToString.rfind('.'); // Find the position of the last '.'
						if (lastSlashPos != std::string::npos && lastDotPos != std::string::npos && lastSlashPos < lastDotPos) {
							pathToString = pathToString.substr(lastSlashPos + 1, lastDotPos - lastSlashPos - 1);
						}
						
						wo->setLabel(pathToString);
						worldLst->push_back(wo);
						GUIobjects.push_back(wo);

					}
				}

				//if (ImGui::Button("Save a file!"))
				//	gui->fileDialog_show_Save("Choose File###labelSave");
				//if (gui->fileDialog_has_selected_path("Choose File###labelSave")) {
				//	selected_path = *gui->fileDialog_get_selected_path("Choose File###labelSave"); //can only get one time, this clears the dialog's state!
				//	origin_fileDialog = "Choose File###labelSave: ";
				//}

				//This prints the selected path and it is prepended with which file dialog title was used
				//ImGui::Text("Selected Path is : '%s'", selected_path.string().c_str());
			}
			*/
			//ImGui::ShowDemoWindow(); //Displays the default ImGui demo from C:/repos/aburn/engine/src/imgui_implot/implot_demo.cpp
			//WOImGui::draw_AftrImGui_Demo( gui ); //Displays a small Aftr Demo from C:/repos/aburn/engine/src/aftr/WOImGui.cpp
			//ImPlot::ShowDemoWindow(); //Displays the ImPlot demo using ImGui from C:/repos/aburn/engine/src/imgui_implot/implot_demo.cpp
		});
	this->worldLst->push_back(gui);

	createNewModuleWayPoints();
}

void GLViewNewModule::createNewModuleWayPoints()
{
	//Create a waypoint with a radius of 3, a frequency of 5 seconds, activated by GLView's camera, and is visible.
	WayPointParametersBase params(this);
	params.frequency = 5000;
	params.useCamera = true;
	params.visible = false;
	WOWayPointSpherical* wayPt = WOWayPointSpherical::New(params, 3);
	wayPt->setPosition(Vector(50, 0, 3));
	worldLst->push_back(wayPt);
}

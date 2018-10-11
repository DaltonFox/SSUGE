#include <stdafx.h>
#include <application.h>
#include <game_object.h>
#include <game_object_manager.h>
#include <math.h>

// The template-specialization to declare the singleton variable for the Application class
template<> ssuge::Application* ssuge::Singleton<ssuge::Application>::msSingleton = NULL;


ssuge::Application::Application()  : OgreBites::ApplicationContext("ssuge", false), mLogManager(NULL), mGameObjectManager(NULL), mScriptManager(NULL), 
																			 mSoundManager(NULL), mInputManager(NULL), mGUIManager(NULL)
{
	
}


ssuge::Application::~Application()
{
	// We'll do most of our clean-up in shutdown
}



void ssuge::Application::setup()
{
	// do not forget to call the base first
	OgreBites::ApplicationContext::setup();

	// The reason I create the input manager here (rather than with our other managers below)
	// is because when the window is created, a resize-event is sent to our application.  The input
	// manager needs to intercept this and call some OgreBites::ApplicationContext functions.
	mInputManager = new InputManager(mSDLWindow);

	// get a pointer to some already-created objects and create a scene manager
	mRoot = getRoot();
	mSceneManager = mRoot->createSceneManager(Ogre::ST_GENERIC);
	
	mWindow = getRenderWindow();

	// register our scene with the RTSS
	Ogre::RTShader::ShaderGenerator* shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
	shadergen->addSceneManager(mSceneManager);

	// Create a light source (this will change once we have a LightComponent)
	Ogre::Light* light = mSceneManager->createLight("MainLight");
	Ogre::SceneNode* lightNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
	lightNode->setPosition(0, 1200, 500);
	lightNode->attachObject(light);

	// This will also change once we have a CameraComponent
	Ogre::SceneNode* camNode = mSceneManager->getRootSceneNode()->createChildSceneNode();
	camNode->setPosition(0, 25, 65);
	camNode->lookAt(Ogre::Vector3(0, 0, 0), Ogre::Node::TS_PARENT);
	Ogre::Camera* cam = mSceneManager->createCamera("myCam");
	cam->setNearClipDistance(0.1f);
	cam->setFarClipDistance(1000.0f);
	cam->setAutoAspectRatio(true);
	camNode->attachObject(cam);
	mWindow->addViewport(cam);

	// Setup shadows
	mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_STENCIL_MODULATIVE);
	mSceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

	// Set up the overlay system
	//mOverlaySystem = new Ogre::OverlaySystem();
	mSceneManager->addRenderQueueListener(getOverlaySystem());

	// Create our "managers"
	mLogManager = new LogManager("ssuge.log", 40);
	mScriptManager = new ScriptManager();
	mGameObjectManager = new GameObjectManager();
	mSoundManager = new SoundManager();
	mCollisionManager = new CollisionManager();
	mGUIManager = new GUIManager();
	
	// Miscellaneous setup
	createDebugPanel();

	// Load our setup python script
	SCRIPT_MANAGER->loadScript("..\\Media\\init.py");
}


void ssuge::Application::shutdown()
{
	// Do any cleanup of our objects
	// (empty for now)

	// Delete our managers
	if (mGameObjectManager)				// Done first to remove all components
		delete mGameObjectManager;
	if (mGUIManager)
		delete mGUIManager;
	if (mInputManager)
		delete mInputManager;
	if (mSoundManager)
		delete mSoundManager;
	if (mCollisionManager)
		delete mCollisionManager;		
	if (mScriptManager)
		delete mScriptManager;
	if (mLogManager)
		delete mLogManager;

	// Let the ApplicationContext do its thing
	OgreBites::ApplicationContext::shutdown();
}



bool ssuge::Application::frameStarted(const Ogre::FrameEvent & evt)
{
	// Update the stats panel
	float spacing = 16.0f;
	if (mDebugOverlay && mDebugOverlay->isVisible())
	{
		Ogre::RenderTarget::FrameStats stats = mWindow->getStatistics();

		Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
		Ogre::TextAreaOverlayElement * text = (Ogre::TextAreaOverlayElement*)mgr->getOverlayElement("DebugStatsGUI\\Text0");
		text->setPosition(mgr->getViewportWidth() - 35.0f, 0.0f);
		text->setCharHeight(spacing);
	}
	LOG_MANAGER->update(evt.timeSinceLastFrame);

	// Update the GOM, which will in turn update all game objects
	GAME_OBJECT_MANAGER->update(evt.timeSinceLastFrame);

	// Update the Input manager, which will process all input.  This method will return false
	// if the window was closed or true if it wasn't.
	return INPUT_MANAGER->update();
}



void ssuge::Application::createDebugPanel()
{
	Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
	mDebugOverlay = mgr->create("DebugStatsGUI");

	// Create a panel to hold all text elements
	Ogre::OverlayContainer * panel = (Ogre::OverlayContainer*)(mgr->createOverlayElement("Panel", "DebugStatsGUI\\Panel"));
	mDebugOverlay->add2D(panel);

	// Create the text elements
	for (unsigned int i = 0; i < 2; i++)
	{
		Ogre::TextAreaOverlayElement * text = (Ogre::TextAreaOverlayElement*)mgr->createOverlayElement("TextArea", "DebugStatsGUI\\Text" + std::to_string(i));
		text->setMetricsMode(Ogre::GMM_PIXELS);
		text->setFontName("SdkTrays/Value");
		panel->addChild(text);
	}

	// Show the overlay
	mDebugOverlay->show();
}



void ssuge::Application::resizeWindow(int w, int h)
{
	mWindow->resize(w, h);
	windowResized(mWindow);
}



void ssuge::Application::startShutdown()
{
	//closeApp();
	getRoot()->queueEndRendering();
	//ApplicationContext::shutdown();
}
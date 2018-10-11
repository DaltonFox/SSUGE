#include <stdafx.h>
#include <gui_manager.h>

// The template-specialization to declare the singleton variable for the GUIManager class
template<> ssuge::GUIManager* ssuge::Singleton<ssuge::GUIManager>::msSingleton = NULL;

ssuge::GUIManager::GUIManager()
{
	Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
	mOverlay = mgr->create("GUIManager");

	mContainer = static_cast<Ogre::OverlayContainer*>(mgr->createOverlayElement("Panel", "GUIPanel"));
	mOverlay->add2D(mContainer);
	mOverlay->show();
}



ssuge::GUIManager::~GUIManager()
{
	Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
	mgr->destroy(mOverlay);
}
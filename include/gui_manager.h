#pragma once
#include <stdafx.h>
#include <singleton.h>

#define GUI_MANAGER ssuge::GUIManager::getSingletonPtr()

namespace ssuge
{
	class GUIComponent;

	class GUIManager : public Singleton<GUIManager>
	{
	// @@@@@ ATTRIBUTES @@@@@
	protected:
		/// The Ogre overlay used to display all elements
		/// Note to self: do we want to break this up a bit more?
		Ogre::Overlay * mOverlay;

		/// The ancestor of all elements
		Ogre::OverlayContainer * mContainer;

		/// The panel

	// @@@@@ CONSTRUCTOR / DESTRUCTOR @@@@@
	public:
		/// Constructor
		GUIManager();

		/// Destructor
		~GUIManager();

	// @@@@@ METHODS @@@@@
	public:
		/// 


	friend class GUIComponent;
	};
}

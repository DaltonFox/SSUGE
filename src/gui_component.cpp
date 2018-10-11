#include <stdafx.h>
#include <gui_component.h>
#include <game_object.h>
#include <gui_manager.h>

ssuge::GUIComponent::GUIComponent(GameObject * owner, GUIType type, const ssuge::Rectangle & rect) : ssuge::Component(owner), mGUIType(type)
{
	Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
	if (type == GUIType::TEXT)
	{
		Ogre::TextAreaOverlayElement * text = (Ogre::TextAreaOverlayElement*)mgr->createOverlayElement("TextArea", mOwner->getName() + "_text");
		text->setMetricsMode(Ogre::GMM_RELATIVE);
		text->setFontName("SdkTrays/Value");			// default text
		text->setCharHeight(rect.mHeight);

		mElement = text;
	}
	else
	{
		Ogre::OverlayManager * mgr = Ogre::OverlayManager::getSingletonPtr();
		mElement = (Ogre::OverlayElement*)mgr->createOverlayElement("Panel", mOwner->getName() + "_image");
	}

	mElement->setPosition(rect.mX, rect.mY);
	mElement->setDimensions(rect.mWidth, rect.mHeight);

	GUI_MANAGER->mContainer->addChild(mElement);
}



ssuge::GUIComponent::~GUIComponent()
{
	// I'm going to be lazy here -- I was running into a lot of dependency problems with 
	// the following line -- if a child were deleted (by the GOM) first, this will crash.
	// Instead, I'll just hide it and let ogre clean it up upon shutdown
	//GUI_MANAGER->mContainer->removeChild(mElement->getName());
	mElement->hide();
}



void ssuge::GUIComponent::setMaterial(std::string mat_name)
{
	if (mGUIType == GUIType::IMAGE)
		mElement->setMaterialName(mat_name);
}

		

void ssuge::GUIComponent::setText(std::string text)
{
	if (mGUIType == GUIType::TEXT)
	{
		Ogre::TextAreaOverlayElement * text_elem = (Ogre::TextAreaOverlayElement*)mElement;
		text_elem->setCaption(text);
	}
}


void ssuge::GUIComponent::setAlignment(HorizontalAlignmentType horiz, VerticalAlignmentType vert)
{
	switch (horiz)
	{
	case HorizontalAlignmentType::LEFT:
		mElement->setHorizontalAlignment(Ogre::GuiHorizontalAlignment::GHA_LEFT);
		break;
	case HorizontalAlignmentType::RIGHT:
		mElement->setHorizontalAlignment(Ogre::GuiHorizontalAlignment::GHA_RIGHT);
		break;
	case HorizontalAlignmentType::CENTER:
		mElement->setHorizontalAlignment(Ogre::GuiHorizontalAlignment::GHA_CENTER);
		break;
	}

	switch (vert)
	{
	case VerticalAlignmentType::TOP:
		mElement->setVerticalAlignment(Ogre::GuiVerticalAlignment::GVA_TOP);
		break;
	case VerticalAlignmentType::BOTTOM:
		mElement->setVerticalAlignment(Ogre::GuiVerticalAlignment::GVA_BOTTOM);
		break;
	case VerticalAlignmentType::CENTER:
		mElement->setVerticalAlignment(Ogre::GuiVerticalAlignment::GVA_CENTER);
		break;
	}
}



void ssuge::GUIComponent::setGUITextColor(const Ogre::ColourValue & top, const Ogre::ColourValue & bottom)
{
	if (mGUIType == GUIType::TEXT)
	{
		Ogre::TextAreaOverlayElement * text = (Ogre::TextAreaOverlayElement*)mElement;
		text->setColourTop(top);
		text->setColourBottom(bottom);
	}
}


		

void ssuge::GUIComponent::setVisible(bool is_visible) 
{
	if (is_visible)
		mElement->show();
	else
		mElement->hide();
}



void ssuge::GUIComponent::update(float dt)
{
	// Ogre has a weird centering mode -- the setAlignment commands above set the reference point,
	// but you still have to adjust the left and right using the size of the object.  Kind of defeats
	// the purpose of having a center code.  My workaround: every frame adjust the position.  This feels
	// a little wasteful, but I don't want them to have to set the centering code before adjusting text.
	/*if (mGUIType == GUIType::TEXT)
	{
		Ogre::TextAreaOverlayElement * text = (Ogre::TextAreaOverlayElement*)mElement;

		if (mElement->getHorizontalAlignment() == Ogre::GHA_CENTER)
			mElement->setLeft(-text->getSpaceWidth() / 2.0f);
		else if (mElement->getHorizontalAlignment() == Ogre::GHA_RIGHT)
			mElement->setLeft(-text->getSpaceWidth());

		// There doesn't seem to be an equivalent for the height...do we just use charHeight?  I'll assume so.
		if (mElement->getVerticalAlignment() == Ogre::GVA_CENTER)
			mElement->setTop(-text->getCharHeight() / 2.0f);
		else if (mElement->getHorizontalAlignment() == Ogre::GHA_RIGHT)
			mElement->setTop(-text->getCharHeight());
	}*/
}
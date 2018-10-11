#pragma once
#include <stdafx.h>
#include <component.h>

namespace ssuge
{
	class GameObject;

	/// The type of gui element
	enum class GUIType { IMAGE, TEXT };

	/// For aligning elements horizontally
	enum class HorizontalAlignmentType { CENTER, LEFT, RIGHT };

	/// For aligning elements vertically
	enum class VerticalAlignmentType { BOTTOM, CENTER, TOP };

	/// A simple rectangle
	class Rectangle
	{
	public:
		float mX;
		float mY;
		float mWidth;
		float mHeight;

		Rectangle(float x, float y, float w, float h) : mX(x), mY(y), mWidth(w), mHeight(h) {}
	};

	class GUIComponent : public Component
	{
	// @@@@@ ATTRIBUTES @@@@@
	protected:
		/// The type of element
		GUIType mGUIType;

		/// The font name (for TEXT types) or material name (for IMAGE types)
		std::string mSetting;

		/// The Ogre overlay element
		Ogre::OverlayElement * mElement;

	// @@@@@ CONSTRUCTORS / DESTRUCTORS @@@@@
	public:
		/// constructor
		GUIComponent(GameObject * owner, GUIType type, const ssuge::Rectangle & rect);

		/// Destructor
		~GUIComponent();

	// @@@@@ METHODS @@@@@
	public:
		/// Updates the material (only has an effect if this is a IMAGE element)
		void setMaterial(std::string mat_name);

		/// Updates the caption of a text element (only has an effect if this is a TEXT element)
		void setText(std::string text);

		/// Sets the horizontal and vertical alignment of this element (mainly used for text)
		void setAlignment(HorizontalAlignmentType horiz, VerticalAlignmentType vert);

		/// Sets the text color
		void setGUITextColor(const Ogre::ColourValue & top, const Ogre::ColourValue & bottom);
		

	// @@@@@ OVERRIDES from COMPONENT @@@@@
	public:
		/// Returns the type of this component.
		ComponentType getType() { return ComponentType::GUI; }

		/// Makes this mesh not render (if is_visible is false)
		void setVisible(bool is_visible) override;

		/// Updates this component
		virtual void update(float dt);
	};
}

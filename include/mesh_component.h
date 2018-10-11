#pragma once
#include <stdafx.h>
#include <component.h>

namespace ssuge
{
	class GameObject;

	/// The MeshComponent component-type is used to load and display a .mesh file instance
	class MeshComponent : public Component
	{
	// @@@@@ ATTRIBUTES @@@@@
	protected:
		/// The ogre entity.
		Ogre::Entity * mEntity;

	// @@@@@ CONSTRUCTORS / DESTRUCTORS @@@@@
	public:
		/// The constructor
		MeshComponent(std::string fname, GameObject * owner);

		/// Destructor
		~MeshComponent();

	// @@@@@ METHODS @@@@@
	public:
		/// Sets the material for a sub-entity
		void setMaterial(int sub_entity, std::string mat_name);

	// @@@@@ OVERRIDES from COMPONENT @@@@@
	public:
		/// Returns the type of this component.
		ComponentType getType() { return ComponentType::MESH; }
		
		/// Makes this mesh not render (if is_visible is false)
		void setVisible(bool is_visible) override { mEntity->setVisible(is_visible); }
	};
}

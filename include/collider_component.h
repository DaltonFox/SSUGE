#pragma once

#include <stdafx.h>
#include <component.h>

namespace ssuge
{
	// Forward reference to the game object class (which we include in the .cpp file to avoid a circular dependency)
	class GameObject;

	/// The types of collider primitives
	enum class ColliderType { SPHERE, BOX, PLANE };

	/// The ColliderComponent class is used to create a bounding volume around a game object (currently spheres and
	/// oriented boxes are supported).
	class ColliderComponent : public Component
	{
	// @@@@@ ATTRIBUTES @@@@@
	protected:
		/// The type of collider
		ColliderType mColliderType;

		/// Are we active (visible?) or not
		bool mActive;

		/// The data used to define the primitive.  
		//     For spheres: 0 = radius
		//     For boxes: 0,1,2 = half-extents
		//     For planes: 0,1,2 = normal (unit-length), 3=d-value 
		float mColliderData[4];
		

	// @@@@@ CONSTRUCTOR / DESTRUCTORS @@@@@
	public:
		/// Constructor.  This constructor auto-deduces the radius / box-entents from the parent scene node
		ColliderComponent(GameObject * owner, ColliderType tp);

		/// Creates a sphere-collider
		ColliderComponent(GameObject * owner, float radius) : Component(owner), mActive(true) { setSphereExtents(radius); }

		/// Creates a box-collider
		ColliderComponent(GameObject * owner, Ogre::Vector3 half_extents) : Component(owner), mActive(true) { setBoxExtents(half_extents); }

		/// Creates a plane-collider
		ColliderComponent(GameObject * owner, Ogre::Vector3 normal, float d) : Component(owner), mActive(true) { setPlaneData(normal, d); }

		/// Destructor
		~ColliderComponent();

	// @@@@@ OVERRIDES from Component class @@@@@
	public:
		/// Tells the caller what type we are.
		ComponentType getType() override { return ComponentType::COLLIDER; }

		/// Makes this component inactive if is_visible is set to true
		void setVisible(bool is_visible) override { mActive = is_visible; }

	// @@@@@ METHODS @@@@@
	protected:
		/// Sets the box-extents (and changes the type to box)
		void setBoxExtents(Ogre::Vector3 half_extents);

		/// Sets the sphere-extents (and changes the type to sphere)
		void setSphereExtents(float radius);

		/// Sets the plane data (and changes the type to plane)
		void setPlaneData(const Ogre::Vector3 & normal, float d);

		/// Implements getRayCollision for spheres
		void getRayCollisions_sphere(const Ogre::Ray & R, std::vector<float> & hit_distances);

		/// Implements getRayCollision for boxes
		void getRayCollisions_box(const Ogre::Ray & R, std::vector<float> & hit_distances);

		/// Implements getRayCollision for planes
		void getRayCollisions_plane(const Ogre::Ray & R, std::vector<float> & hit_distances);

	public:
		/// Gets the collision distances of this Ray with this object.  The vector is appended with
		/// ray collision distances 
		void getRayCollisions(const Ogre::Ray & R, std::vector<float> & hit_distances);
		
	};
}

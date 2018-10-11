#pragma once
#include <stdafx.h>
#include <singleton.h>

#define COLLISION_MANAGER ssuge::CollisionManager::getSingletonPtr()

namespace ssuge
{
	// Forward references
	class GameObject;
	class ColliderComponent;

	/// When a Collider object is created it is automatically added to this
	/// manager (and removed when the Collider component is destroyed).  This 
	/// manager is used to find and report collisions between objects in the same
	/// group and to find ray-cast hits within a group
	class CollisionManager : public Singleton<CollisionManager>
	{
	protected:
		std::map<int, std::vector<ColliderComponent*>> mGroups;

	public:
		CollisionManager();

		~CollisionManager();

		void addCollider(int group, ColliderComponent* comp);

		void removeCollider(int group, ColliderComponent * comp);

		/// Finds any collisions between components and notifies the parent game objects (with an onCollide) message
		void update(float dt);

		void rayCast(int group, const Ogre::Ray& ray, std::vector<GameObject*> & hit_objects, std::vector<Ogre::Vector3> & hit_pts);
	};
}

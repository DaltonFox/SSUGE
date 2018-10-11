#include <stdafx.h>
#include <collision_manager.h>
#include <game_object.h>
#include <collider_component.h>

// The template-specialization to declare the singleton variable for the LogManager class
template<> ssuge::CollisionManager* ssuge::Singleton<ssuge::CollisionManager>::msSingleton = NULL;



ssuge::CollisionManager::CollisionManager()
{

}



ssuge::CollisionManager::~CollisionManager()
{

}



void ssuge::CollisionManager::addCollider(int group, ColliderComponent* comp)
{
	// Make sure we don't already have it in our lists
	std::map<int, std::vector<ColliderComponent*>>::iterator it = mGroups.find(group);
	if (it != mGroups.end())
	{
		// Look in the sub-vector
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			if (it->second[i] == comp)
				return;
		}
	}

	mGroups[group].push_back(comp);
}



void ssuge::CollisionManager::removeCollider(int group, ColliderComponent * comp)
{
	std::map<int, std::vector<ColliderComponent*>>::iterator it = mGroups.find(group);
	if (it != mGroups.end())
	{
		// Look in the sub-vector
		std::vector<ColliderComponent*>::iterator it2 = it->second.begin();
		while (it2 != it->second.end())
		{
			if (*it2 == comp)
			{
				it2 = it->second.erase(it2);
				return;
			}
			else
				++it2;
		}
	}
}



void ssuge::CollisionManager::update(float dt)
{
	// FINISH ME!
}

void ssuge::CollisionManager::rayCast(int group, const Ogre::Ray& ray, std::vector<GameObject*> & hit_objects, std::vector<Ogre::Vector3> & hit_pts)
{
	std::map<int, std::vector<ColliderComponent*>>::iterator it = mGroups.find(group);
	
	hit_objects.clear();
	hit_pts.clear();

	std::vector<GameObject*>all_hit_objects;
	std::vector<float>all_hit_distances;

	if (it != mGroups.end())
	{
		for (unsigned int i = 0; i < it->second.size(); i++)
		{
			std::vector<float> hit_distances;
			it->second[i]->getRayCollisions(ray, hit_distances);

			for (unsigned int j = 0; j < hit_distances.size(); j++)
			{
				all_hit_objects.push_back(it->second[i]->getOwner());
				all_hit_distances.push_back(hit_distances[j]);
			}
		}
	}

	// There's probably a std:: function to sort two lists simultaneously, but I'm lazy -- I'll
	// just do a bubble sort
	//std::sort(hit_distances.begin(), hit_distances.end());

	bool sorted;
	for (unsigned int i = 0; i < all_hit_distances.size(); i++)
	{
		sorted = true;
		for (unsigned int j = 0; j < all_hit_distances.size() - i - 1; j++)
		{
			if (all_hit_distances[j] > all_hit_distances[j + 1])
			{
				GameObject * temp_gobj = all_hit_objects[j];
				float temp_dist = all_hit_distances[j];
				all_hit_objects[j] = all_hit_objects[j + 1];
				all_hit_distances[j] = all_hit_distances[j + 1];
				all_hit_objects[j + 1] = temp_gobj;
				all_hit_distances[j + 1] = temp_dist;
				sorted = false;
			}
		}
		if (sorted)
			break;
	}

	// Now, copy our temp values over to the vectors they gave us
	for (unsigned int i = 0; i < all_hit_distances.size(); i++)
	{
		hit_objects.push_back(all_hit_objects[i]);
		hit_pts.push_back(ray.getPoint(all_hit_distances[i]));
	}
}
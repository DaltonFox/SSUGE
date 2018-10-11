#include <stdafx.h>
#include <collider_component.h>
#include <game_object.h>
#include <collision_manager.h>
#include <exception.h>

ssuge::ColliderComponent::ColliderComponent(ssuge::GameObject * owner, ssuge::ColliderType tp) : ssuge::Component(owner), mActive(true)
{
	// Make sure we have up-to-date world bounds
	mOwner->getSceneNode()->_update(true, true);

	Ogre::AxisAlignedBox bbox = mOwner->getSceneNode()->_getWorldAABB();
	Ogre::Vector3 half_extents = bbox.getHalfSize();

	if (tp == ColliderType::BOX)
		setBoxExtents(half_extents);
	else if (tp == ColliderType::SPHERE)
		setSphereExtents((half_extents.x + half_extents.y + half_extents.z) / 3.0f);
	// Not sure how to deduce this for a plane...just assume y-axis normal and get d-value from position
	else if (tp == ColliderType::PLANE)
		setPlaneData(Ogre::Vector3::UNIT_Y, mOwner->getSceneNode()->_getDerivedPosition().y);
	else
		EXCEPTION("You forgot to add a new case here for a new enum...");
}



ssuge::ColliderComponent::~ColliderComponent()
{
	COLLISION_MANAGER->removeCollider(mOwner->getGroupID(), this);
}



void ssuge::ColliderComponent::setBoxExtents(Ogre::Vector3 half_extents)
{
	mColliderType = ColliderType::BOX;
	mColliderData[0] = half_extents.x;
	mColliderData[1] = half_extents.y;
	mColliderData[2] = half_extents.z;

	COLLISION_MANAGER->addCollider(mOwner->getGroupID(), this);
}

	

void ssuge::ColliderComponent::setSphereExtents(float radius)
{
	mColliderType = ColliderType::SPHERE;
	mColliderData[0] = radius;

	COLLISION_MANAGER->addCollider(mOwner->getGroupID(), this);
}



void ssuge::ColliderComponent::setPlaneData(const Ogre::Vector3 & normal, float d)
{
	mColliderType = ColliderType::PLANE;
	Ogre::Vector3 normal_hat = normal.normalisedCopy();
	mColliderData[0] = normal_hat.x;
	mColliderData[1] = normal_hat.y;
	mColliderData[2] = normal_hat.z;
	mColliderData[3] = d;

	COLLISION_MANAGER->addCollider(mOwner->getGroupID(), this);
}



void ssuge::ColliderComponent::getRayCollisions_sphere(const Ogre::Ray & R, std::vector<float> & hit_distances)
{
	Ogre::Vector3 C = mOwner->getSceneNode()->_getDerivedPosition();
	Ogre::Vector3 sfactor = mOwner->getSceneNode()->_getDerivedScale();
	float rad = mColliderData[0] * (sfactor.x + sfactor.y + sfactor.z) / 3.0f;
	float rad2 = rad * rad;

	Ogre::Vector3 E = C - R.getOrigin();
	Ogre::Vector3 D = R.getDirection().normalisedCopy();
	float para = E.dotProduct(D);
	float para_sq = para * para;
	float e_mag_sq = E.dotProduct(E);
	float perp_sq = e_mag_sq - para_sq;
	float rad_sq = rad * rad;
	if (perp_sq > rad_sq)
		return;					// Ray misses the sphere

	float offset = sqrtf(rad_sq - perp_sq);
	float t1 = para + offset;
	float t2 = para - offset;

	if (t1 >= 0)
		hit_distances.push_back(t1);
	if (t2 >= 0 && t1 != t2)
		hit_distances.push_back(t2);
}




void ssuge::ColliderComponent::getRayCollisions_box(const Ogre::Ray & R, std::vector<float> & hit_distances)
{
	Ogre::Vector3 plane_normals[6];

	Ogre::Quaternion orient = mOwner->getSceneNode()->_getDerivedOrientation();
	Ogre::Vector3 pos = mOwner->getSceneNode()->_getDerivedPosition();
	plane_normals[0] = orient.xAxis().normalisedCopy();
	plane_normals[1] = -plane_normals[0];
	plane_normals[2] = orient.yAxis().normalisedCopy();
	plane_normals[3] = -plane_normals[2];
	plane_normals[4] = orient.zAxis().normalisedCopy();
	plane_normals[5] = -plane_normals[4];

	Ogre::Vector3 D = R.getDirection().normalisedCopy();

	for (int i = 0; i < 6; i++)
	{
		float mid_dval = pos.dotProduct(plane_normals[i]);
		float dval = mid_dval + mColliderData[i / 2];

		// Do the ray-plane intersection
		float denom = D.dotProduct(plane_normals[i]);
		if (denom == 0.0)
			continue;

		float t = (dval - R.getOrigin().dotProduct(plane_normals[i])) / denom;
		if (t <= 0)
			continue;


		// Get the point at which it would collide and project that onto all 6 axes.
		Ogre::Vector3 pt = R.getPoint(t);
		bool disqualify = false;
		for (int j = 0; j < 6; j++)
		{
			float proj = (pt - pos).dotProduct(plane_normals[j]);
			float max_val = mColliderData[j / 2];
			if (proj < -max_val - 0.0001f || proj > max_val + 0.0001f)
			{
				disqualify = true;
				break;
			}
		}

		if (!disqualify)
		{
			// We have a hit!
			hit_distances.push_back(t);
		}
	}
}



void ssuge::ColliderComponent::getRayCollisions_plane(const Ogre::Ray & R, std::vector<float> & hit_distances)
{
	Ogre::Vector3 D = R.getDirection().normalisedCopy();
	Ogre::Vector3 N = Ogre::Vector3(mColliderData[0], mColliderData[1], mColliderData[2]);
	float denom = D.dotProduct(N);
	if (denom == 0.0)
		return;				// Ray is parallel to this plane -- no hits possible

	float t = (mColliderData[3] - R.getOrigin().dotProduct(N)) / denom;
	if (t <= 0)
		return;				// Ray is pointed away from plane -- no hit.
	hit_distances.push_back(t);
}

	

void ssuge::ColliderComponent::getRayCollisions(const Ogre::Ray & R, std::vector<float> & hit_distances)
{
	switch (mColliderType)
	{
	case ColliderType::BOX:
		getRayCollisions_box(R, hit_distances);
		break;
	case ColliderType::SPHERE:
		getRayCollisions_sphere(R, hit_distances);
		break;
	case ColliderType::PLANE:
		getRayCollisions_plane(R, hit_distances);
		break;
	}
}
#include <stdafx.h>
#include <mesh_component.h>
#include <game_object.h>
#include <application.h>

ssuge::MeshComponent::MeshComponent(std::string fname, GameObject * owner) : Component(owner)
{
	mEntity = APPLICATION->getSceneManager()->createEntity(owner->getName() + "_entity" + std::to_string(owner->getNumComponents(ComponentType::MESH)), fname);
	owner->getSceneNode()->attachObject(mEntity);
}


ssuge::MeshComponent::~MeshComponent()
{
	if (mEntity)
	{
		if (mEntity->getParentSceneNode())
			mEntity->getParentSceneNode()->detachObject(mEntity);
		APPLICATION->getSceneManager()->destroyEntity(mEntity);
	}
}


void ssuge::MeshComponent::setMaterial(int sub_entity, std::string mat_name)
{
	Ogre::SubEntity * se = mEntity->getSubEntity(sub_entity);
	se->setMaterialName(mat_name);
}
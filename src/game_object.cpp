#include <stdafx.h>
#include <game_object.h>
#include <script_manager.h>
#include <application.h>
#include <game_object_manager.h>


ssuge::GameObject::GameObject(std::string name, int group_id) : mName(name), mGroupID(group_id), mScriptTwin(NULL)
{
	mSceneNode = APPLICATION->getSceneManager()->getRootSceneNode()->createChildSceneNode();
}


ssuge::GameObject::~GameObject()
{
	// Release the python "twin" if there is one
	if (mScriptTwin && SCRIPT_MANAGER != NULL)
	{
		// It feels like I should do this...but, when I do, it makes the ref-count go negative
		// when the script manager does its Py_Finalize call.  
		//Py_DECREF(mScriptTwin);
		mScriptTwin = NULL;
	}

	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.begin();
	while (iter != mComponents.end())
	{
		for (unsigned int i = 0; i < iter->second.size(); i++)
			delete iter->second[i];
		iter->second.clear();
		iter++;
	}
	mComponents.clear();

	if (mSceneNode)
	{
		mSceneNode->detachAllObjects();
		if (mSceneNode->getParentSceneNode())
			mSceneNode->getParentSceneNode()->removeChild(mSceneNode);
		APPLICATION->getSceneManager()->destroySceneNode(mSceneNode);
	}
	
	mComponents.clear();
}



void ssuge::GameObject::update(float dt)
{
	// Call the onUpdate method of script-aware objects
	if (mScriptTwin)
	{
		PyObject * args = PyTuple_New(1);
		PyTuple_SetItem(args, 0, PyFloat_FromDouble((double)dt));
		callScriptMethod("onUpdate", args);
		Py_DECREF(args);
	}
	
	// Update the components
	std::map<ComponentType, std::vector<Component*>>::iterator it = mComponents.begin();
	while (it != mComponents.end())
	{
		for (unsigned int j = 0; j < it->second.size(); j++)
			it->second[j]->update(dt);
		++it;
	}
}



void ssuge::GameObject::setVisible(float is_visible)
{
	std::map<ComponentType, std::vector<Component*>>::iterator it = mComponents.begin();
	while (it != mComponents.end())
	{
		for (unsigned int j = 0; j < it->second.size(); j++)
			it->second[j]->setVisible(is_visible);
		++it;
	}
}


ssuge::MeshComponent * ssuge::GameObject::createMeshComponent(std::string fname)
{
	ssuge::MeshComponent * mc = new ssuge::MeshComponent(fname, this);
	mComponents[ComponentType::MESH].push_back(mc);
	return mc;
}


ssuge::LightComponent * ssuge::GameObject::createLightComponent(ssuge::LightType t)
{
	ssuge::LightComponent * lc = new ssuge::LightComponent(t, this);
	mComponents[ComponentType::LIGHT].push_back(lc);
	return lc;
}


ssuge::CameraComponent * ssuge::GameObject::createCameraComponent()
{
	ssuge::CameraComponent * cc = new ssuge::CameraComponent(this);
	mComponents[ComponentType::CAMERA].push_back(cc);
	return cc;
}


ssuge::SoundComponent * ssuge::GameObject::createSoundComponent(std::string fname, bool is_3d)
{
	ssuge::SoundComponent * sc = new ssuge::SoundComponent(this, fname, is_3d);
	mComponents[ComponentType::SOUND].push_back(sc);
	return sc;
}


ssuge::ColliderComponent * ssuge::GameObject::createColliderComponent(float radius)
{
	ssuge::ColliderComponent * cc = new ssuge::ColliderComponent(this, radius);
	mComponents[ComponentType::COLLIDER].push_back(cc);
	return cc;
}



ssuge::ColliderComponent * ssuge::GameObject::createColliderComponent(const Ogre::Vector3 & half_extents)
{
	ssuge::ColliderComponent * cc = new ssuge::ColliderComponent(this, half_extents);
	mComponents[ComponentType::COLLIDER].push_back(cc);
	return cc;
}



ssuge::ColliderComponent * ssuge::GameObject::createColliderComponent(const Ogre::Vector3 & normal, float d)
{
	ssuge::ColliderComponent * cc = new ssuge::ColliderComponent(this, normal, d);
	mComponents[ComponentType::COLLIDER].push_back(cc);
	return cc;
}



ssuge::ColliderComponent * ssuge::GameObject::createColliderComponent(ssuge::ColliderType type)
{
	ssuge::ColliderComponent * cc = new ssuge::ColliderComponent(this, type);
	mComponents[ComponentType::COLLIDER].push_back(cc);
	return cc;
}


ssuge::GUIComponent * ssuge::GameObject::createGUIComponent(ssuge::GUIType type, const ssuge::Rectangle & rect)
{
	ssuge::GUIComponent * gc = new ssuge::GUIComponent(this, type, rect);
	mComponents[ComponentType::GUI].push_back(gc);
	return gc;
}



ssuge::MeshComponent * ssuge::GameObject::getMeshComponent(unsigned int index)
{
	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.find(ComponentType::MESH);
	if (iter != mComponents.end() && iter->second.size() > index)
		return (ssuge::MeshComponent*)iter->second[index];
	else
		return NULL;
}


ssuge::LightComponent * ssuge::GameObject::getLightComponent(unsigned int index)
{
	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.find(ComponentType::LIGHT);
	if (iter != mComponents.end() && iter->second.size() > index)
		return (ssuge::LightComponent*)iter->second[index];
	else
		return NULL;
}


ssuge::CameraComponent * ssuge::GameObject::getCameraComponent(unsigned int index)
{
	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.find(ComponentType::CAMERA);
	if (iter != mComponents.end() && iter->second.size() > index)
		return (ssuge::CameraComponent*)iter->second[index];
	else
		return NULL;
}


ssuge::SoundComponent * ssuge::GameObject::getSoundComponent(unsigned int index)
{
	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.find(ComponentType::SOUND);
	if (iter != mComponents.end() && iter->second.size() > index)
		return (ssuge::SoundComponent*)iter->second[index];
	else
		return NULL;
}


ssuge::GUIComponent * ssuge::GameObject::getGUIComponent(unsigned int index)
{
	std::map<ComponentType, std::vector<Component*>>::iterator iter = mComponents.find(ComponentType::GUI);
	if (iter != mComponents.end() && iter->second.size() > index)
		return (ssuge::GUIComponent*)iter->second[index];
	else
		return NULL;
}



std::string ssuge::GameObject::getGroupName()
{
	return GAME_OBJECT_MANAGER->getGroupName(this->mGroupID);
}



void ssuge::GameObject::setParent(GameObject* parent, bool keep_in_same_world_spot)
{
	Ogre::SceneNode * parent_node = mSceneNode->getParentSceneNode();
	if (parent_node != NULL)
	{
		Ogre::SceneNode * new_parent_node = parent->getSceneNode();

		if (keep_in_same_world_spot)
		{
			// Get the world-space orientation, position, and scale
			Ogre::Quaternion q = getWorldOrientation();
			Ogre::Vector3 v = getWorldPosition();
			Ogre::Vector3 s = getWorldScale();

			
			new_parent_node->_update(false, true);			// Make sure local-to-world will work
			Ogre::Quaternion nq = new_parent_node->convertWorldToLocalOrientation(q);
			Ogre::Vector3 nv = new_parent_node->convertWorldToLocalPosition(v);
			Ogre::Vector3 ps = new_parent_node->_getDerivedScale();
			Ogre::Vector3 ns = Ogre::Vector3(s.x / ps.x, s.y / ps.y, s.z / ps.z);

			mSceneNode->setOrientation(nq);
			mSceneNode->setPosition(nv);
			mSceneNode->setScale(ns);
		}

		parent_node->removeChild(mSceneNode);
		new_parent_node->addChild(mSceneNode);
	}
}



void ssuge::GameObject::attachScriptObject(PyObject * sobj)
{ 
	mScriptTwin = sobj; 
}


void ssuge::GameObject::callScriptMethod(std::string method_name, PyObject * args)
{
	if (mScriptTwin == NULL)
		return;					// Not script-aware!

	if (PyObject_HasAttrString(mScriptTwin, method_name.c_str()))
	{
		// Call the specificed method with args as the only parameter (besides self, which is passed automatically)
		PyObject * result = PyObject_CallMethodObjArgs(mScriptTwin, PyUnicode_FromString(method_name.c_str()), args, NULL);

		if (result == NULL)
			SCRIPT_MANAGER->handleError();
		else
		{
			// Eventually we probably want to return this to the caller somehow.  For now, though, just dec-ref it
			Py_DECREF(result);
		}
	}
}



void ssuge::GameObject::onAction(std::string action_name, bool is_starting) 
{
	if (mScriptTwin)
	{
		PyObject * args = PyTuple_New(2);
		PyTuple_SetItem(args, 0, PyUnicode_FromString(action_name.c_str()));
		PyTuple_SetItem(args, 1, PyBool_FromLong(is_starting ? 1 : 0));

		callScriptMethod("onAction", args);

		Py_DECREF(args);
	}
}


void ssuge::GameObject::onAxis(std::string name, unsigned int num, float new_val) 
{
	if (mScriptTwin)
	{
		PyObject * args = PyTuple_New(3);
		PyTuple_SetItem(args, 0, PyUnicode_FromString(name.c_str()));
		PyTuple_SetItem(args, 1, PyLong_FromLong(num));
		PyTuple_SetItem(args, 2, PyFloat_FromDouble((double)new_val));

		callScriptMethod("onAxis", args);

		Py_DECREF(args);
	}
}


void ssuge::GameObject::onTouch(unsigned int device, float x, float y, float xrel, float yrel)
{
	if (mScriptTwin)
	{
		PyObject * args = PyTuple_New(5);
		PyTuple_SetItem(args, 0, PyFloat_FromDouble(device));
		PyTuple_SetItem(args, 1, PyFloat_FromDouble(x));
		PyTuple_SetItem(args, 2, PyFloat_FromDouble(y));
		PyTuple_SetItem(args, 3, PyFloat_FromDouble(xrel));
		PyTuple_SetItem(args, 4, PyFloat_FromDouble(yrel));

		callScriptMethod("onTouch", args);

		Py_DECREF(args);
	}
}


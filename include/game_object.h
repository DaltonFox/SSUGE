#pragma once
#include <stdafx.h>
#include <component.h>
#include <mesh_component.h>
#include <light_component.h>
#include <camera_component.h>
#include <sound_component.h>
#include <collider_component.h>
#include <gui_component.h>
#include <input_manager.h>

namespace ssuge
{
	/// A THING in our game (bullet, player, etc.) Contains a collection of components that give it functionality.
	/// By itself, the game object is basically a node within a scene graph.
	class GameObject : public InputManager::Listener
	{
	// @@@@@ ATTRIBUTES @@@@@
	protected:
		/// The name of this object -- kind of duplicates info in the GOM, but handy for debugging.
		std::string mName;

		/// The Ogre Scene Node storing our position / orientation / scale and spot within the hierarchy
		Ogre::SceneNode * mSceneNode;

		/// If not NULL, this is the python GameObject that is connected to this c++ game object.
		PyObject * mScriptTwin;

		/// The integer group number (within the GOM).  I chose to store this instead of the group name to save a little memory.
		int mGroupID;

		/// The collection of components.  Note: you can have multiple components of a given type.
		std::map<ComponentType, std::vector<Component*>> mComponents;

	// @@@@@ CONSTRUCTORS / DESTRUCTORS @@@@@
	protected:
		/// Constructor
		GameObject(std::string name, int group_id);

		/// Destructor
		virtual ~GameObject();

	// @@@@@ BROADCAST methods -- used to call a similar method in the components @@@@@
	public:
		/// Update all components
		virtual void update(float dt);

		/// Tell all components to use this visibility setting
		virtual void setVisible(float is_visible);

	// @@@@@ COMPONENT GETTERS @@@@@
	public:
		/// Return the number of components of a given type.
		unsigned int getNumComponents(ComponentType t) { return mComponents[t].size(); }

		/// Get the mesh component at the given index
		MeshComponent * getMeshComponent(unsigned int index = 0);

		/// Get the light component at the given index
		LightComponent * getLightComponent(unsigned int index = 0);

		/// Get the camera component at the given index
		CameraComponent * getCameraComponent(unsigned int index = 0);

		/// Get the sound component at the given index
		SoundComponent * getSoundComponent(unsigned int index = 0);

		/// Get the gui component at the given index
		GUIComponent * getGUIComponent(unsigned int index = 0);

	// @@@@@ COMPONENT CREATORS @@@@@
	public:
		/// Create a mesh component and add it to our master list
		MeshComponent * createMeshComponent(std::string fname);
		
		/// Create a light component and add it to our master list
		LightComponent * createLightComponent(ssuge::LightType t);
		
		/// Create a camera component and add it to our master list
		CameraComponent * createCameraComponent();
		
		/// Create a 3d sound component add add it to our master list
		SoundComponent * createSoundComponent(std::string fname, bool is_3d);

		/// Creates a collider component (sphere)
		ColliderComponent * createColliderComponent(float radius);

		/// Creates a collider component (box)
		ColliderComponent * createColliderComponent(const Ogre::Vector3 & half_extents);

		/// Creates a collider component (plane)
		ColliderComponent * createColliderComponent(const Ogre::Vector3 & normal, float d);

		/// Create collider component and attempt to deduce dimensions / data
		ColliderComponent * createColliderComponent(ColliderType type);

		/// Creates a GUI Component
		GUIComponent * createGUIComponent(GUIType type, const ssuge::Rectangle & rect);
		
	// @@@@@ TRANSFORMATION (ABSOLUTE) SETTERS @@@@@
	public:
		/// Makes this game object a child of the given game object, detaching it from its current parent (if any).
		/// If keep_in_same_world_spot, the game object's relative offset from the new parent will be adjusted so it appears
		/// in the same spot, with the same orientation and scale as it inherited from its previous acestors.
		void setParent(GameObject* parent, bool keep_in_same_world_spot);

		/// Set the positional offset (relative to parent game object) 
		void setPosition(float x, float y, float z) { mSceneNode->setPosition(Ogre::Vector3(x, y, z)); }

		/// Set the positional offset (relative to parent game object) 
		void setPosition(const Ogre::Vector3& v) { mSceneNode->setPosition(v); }

		/// Set the positional offset (relative to world) 
		void setPositionWorld(float x, float y, float z) { mSceneNode->setPosition(mSceneNode->getParentSceneNode()->convertWorldToLocalPosition(Ogre::Vector3(x, y, z))); }

		/// Set the positional offset (relative to world) 
		void setPositionWorld(const Ogre::Vector3& v) { mSceneNode->setPosition(mSceneNode->getParentSceneNode()->convertWorldToLocalPosition(v)); }

		/// Sets the scale offset (relative to the parent game object)
		void setScale(float sx, float sy, float sz) { mSceneNode->setScale(Ogre::Vector3(sx, sy, sz)); }

		/// Sets the scale offset (relative to the parent game object)
		void setScale(const Ogre::Vector3& s) { mSceneNode->setScale(s); }

		/// Sets the rotational offset (relative to the parent game object)
		void setOrientation(float degrees, float vx, float vy, float vz) { mSceneNode->setOrientation(Ogre::Quaternion(Ogre::Degree(degrees), Ogre::Vector3(vx, vy, vz))); }

		/// Sets the rotational offset (relative to the parent game object)
		void setOrientation(const Ogre::Quaternion& q) { mSceneNode->setOrientation(q); }

		/// Makes this object's negative z-axis point towards the given world space position
		void lookAt(float x, float y, float z) { mSceneNode->lookAt(Ogre::Vector3(x, y, z), Ogre::Node::TS_WORLD); }

	// @@@@@ TRANSFORMATION (RELATIVE) SETTERS @@@@@
	public:
		/// Adjusts the rotational offset (relative to the parent game object) -- the passed rotation information is relative to the world axes
		void rotateWorld(float degrees, float vx, float vy, float vz) { mSceneNode->rotate(Ogre::Quaternion(Ogre::Degree(degrees), Ogre::Vector3(vx, vy, vz)), Ogre::Node::TS_WORLD); }

		/// Adjusts the rotational offset (relative to the parent game object) -- the passed rotation information is relative to the world axes
		void rotateWorld(const Ogre::Quaternion& q) { mSceneNode->rotate(q, Ogre::Node::TS_WORLD); }

		/// Adjusts the rotational offset (relative to the parent game object) -- the passed rotation information is relative to the local axes
		void rotateLocal(float degrees, float vx, float vy, float vz) { mSceneNode->rotate(Ogre::Quaternion(Ogre::Degree(degrees), Ogre::Vector3(vx, vy, vz)), Ogre::Node::TS_LOCAL); }

		/// Adjusts the rotational offset (relative to the parent game object) -- the passed rotation information is relative to the local axes
		void rotateLocal(const Ogre::Quaternion& q) { mSceneNode->rotate(q, Ogre::Node::TS_LOCAL); }

		/// Adjusts the positional offset (relative to the parent game object) -- the passed translation information is relative to the world axes
		void translateWorld(float tx, float ty, float tz) { mSceneNode->translate(Ogre::Vector3(tx, ty, tz), Ogre::Node::TS_WORLD); }

		/// Adjusts the positional offset (relative to the parent game object) -- the passed translation information is relative to the world axes
		void translateWorld(const Ogre::Vector3& v) { mSceneNode->translate(v, Ogre::Node::TS_WORLD); }

		/// Adjusts the positional offset (relative to the parent game object) -- the passed translation information is relative to the local axes
		void translateLocal(float tx, float ty, float tz) { mSceneNode->translate(Ogre::Vector3(tx, ty, tz), Ogre::Node::TS_LOCAL); }

		/// Adjusts the positional offset (relative to the parent game object) -- the passed translation information is relative to the local axes
		void translateLocal(const Ogre::Vector3& v) { mSceneNode->translate(v, Ogre::Node::TS_LOCAL); }

		/// Adjusts the scale offset (relative to the parent game object)
		void scale(float sx, float sy, float sz) { mSceneNode->scale(Ogre::Vector3(sx, sy, sz)); }

		/// Adjusts the scale offset (relative to the parent game object)
		void scale(const Ogre::Vector3& s) { mSceneNode->scale(s); }

	// @@@@@ TRANSFORMATION GETTERS @@@@@
	public:
		/// Gets the position of this object relative to the world axes.
		Ogre::Vector3 getWorldPosition() { return mSceneNode->_getDerivedPosition(); }

		/// Gets the position of this object relative to the parent game object's axes.
		Ogre::Vector3 getPosition() { return mSceneNode->getPosition(); }

		/// Gets the orientation of this object relative to the world axes.
		Ogre::Quaternion getWorldOrientation() { return mSceneNode->_getDerivedOrientation(); }

		/// Gets the orientation of this object relative to the parent game object's axes.
		Ogre::Quaternion getOrientation() { return mSceneNode->getOrientation(); }

		/// Gets the scale of this object relative to the world axes.
		Ogre::Vector3 getWorldScale() { return mSceneNode->_getDerivedScale(); }

		/// Gets the scale offset from the parent game object
		Ogre::Vector3 getScale() { return mSceneNode->getScale(); }

	// @@@@@ OTHER GETTERS @@@@@
	public:
		/// Gets the ogre scene node this object is based upon.
		Ogre::SceneNode* getSceneNode() { return mSceneNode; }

		/// Gets the name of this game object
		std::string getName() { return mName; }

		/// Gets the group name (as a string) of this game object in the GOM
		std::string getGroupName();

		/// Gets the group id of this game object in the GOM
		int getGroupID() { return mGroupID; }

		/// Gets the script twin (NULL if there is none)
		PyObject * getScriptTwin() { return mScriptTwin; }


	// @@@@@ SCRIPT-RELATED METHODS @@@@@
	public:
		/// Makes this object script-aware.  The PyObject should be a (python) ssuge.GameObject-derived class.
		void attachScriptObject(PyObject* sobj);

		/// This method attempts to call a method of the "script-twin".  If this game object is not script-aware or if the connected python
		/// class doesn't contain the given named method, this function does nothing other than log a warning.
		void callScriptMethod(std::string method_name, PyObject * args = NULL);

	// @@@@@ INPUT-LISTENERS METHODS @@@@@
	public:
		/// Called by the InputManager when an action is initiated or stopped
		virtual void onAction(std::string action_name, bool is_starting) override;

		/// Called by the InputManager when an axis changes state.
		virtual void onAxis(std::string name, unsigned int num, float new_val) override;

		/// Called by the InputManager when a touch-device moves.  Currently only mouse (device = 0) is supported
		virtual void onTouch(unsigned int device, float x, float y, float xrel, float yrel) override;

	// @@@@@ FRIENDS @@@@@
		friend class GameObjectManager;
	};
}

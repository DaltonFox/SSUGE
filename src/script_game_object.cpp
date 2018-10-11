#include <stdafx.h>
#include <script_game_object.h>
#include <structmember.h>
#include <game_object.h>
#include <application.h>
#include <game_object_manager.h>
#include <gui_component.h>

extern PyTypeObject script_GameObjectType;

PyObject * script_GameObject_new(PyTypeObject * type, PyObject * args, PyObject * kwargs)
{
	script_GameObject * obj = (script_GameObject*)type->tp_alloc(type, 0);
	obj->mGameObject = NULL;
	return (PyObject*)obj;
}


int script_GameObject_init(script_GameObject * self, PyObject* args, PyObject * kwargs)
{
	// The caller must provide two strings in args: the name of the group and the name of the object
	// They can, if they wish, provide extra arguments which will be forwarded to the onCreate method (if present)
	if (!PyTuple_Check(args) || PyTuple_Size(args) < 2 || !PyUnicode_Check(PyTuple_GetItem(args, 0)) ||
		!PyUnicode_Check(PyTuple_GetItem(args, 1)))
	{
		PyErr_SetString(PyExc_ValueError, "You must provide at least a group name (string) and object name (string) to this constructor");
		return NULL;
	}

	char * group_name = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	char * object_name = PyUnicode_AsUTF8(PyTuple_GetItem(args, 1));

	// First check to see if the given game object already exists (it might, if loaded from a lvl-type script)
	ssuge::GameObject * gobj = GAME_OBJECT_MANAGER->getGameObject(std::string(group_name), std::string(object_name));

	// If it doesn't already exist, create the c++ game object.
	if (gobj == NULL)
		gobj = GAME_OBJECT_MANAGER->createGameObject(std::string(group_name), std::string(object_name));

	// In either case make the python object's pointer point to it.
	self->mGameObject = gobj;

	// If the class used to instantiate this isn't ssuge.GameObject (but is a sub-type of it), make the c++ game object "script-aware".
	// Note: if this isn't true, it likely means the user was just creating a normal ssuge.GameObject (and probably doesn't want
	// it to be script aware)
	PyObject * self_type = PyObject_Type((PyObject*)self);
	if (PyType_IsSubtype((PyTypeObject*)self_type, &script_GameObjectType) && (void*)&script_GameObjectType != (void*)self_type)
	{
		gobj->attachScriptObject((PyObject*)self);

		// Pick out any extra elements in args, package as tuple (which is empty if only the group name and object name were passed)
		// and call the onCreate method of the sub-class (if there is one)
		PyObject * meth_args = PyTuple_GetSlice(args, 2, PyTuple_Size(args));
		gobj->callScriptMethod("onCreate", meth_args);
		Py_DECREF(meth_args);
	}
	Py_DECREF(self_type);

	

	return 0;
}



PyObject * script_GameObject_str(script_GameObject * self)
{
	std::string temps = "<GameObject:" + self->mGameObject->getName() + ">";
	return PyUnicode_FromString(temps.c_str());
}



void script_GameObject_dealloc(script_GameObject * self)
{
	// Make the c++ game object script un-aware
	if (self->mGameObject != NULL)
		self->mGameObject = NULL;
}



PyObject* script_GameObject_addChild(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyObject_IsInstance(PyTuple_GetItem(args, 0), (PyObject*)&script_GameObjectType))
	{
		PyErr_SetString(PyExc_TypeError, "You must pass a ssuge.GameObject (or derived class) instance to this method");
		return NULL;
	}

	script_GameObject * child = (script_GameObject*)PyTuple_GetItem(args, 0);

	child->mGameObject->setParent(self->mGameObject, true);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* script_GameObject_createMeshComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a mesh file name");
		return NULL;
	}

	char * fname = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	self->mGameObject->createMeshComponent(std::string(fname));
	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_createCameraComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyLong_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a viewport index#");
		return NULL;
	}

	int vport_num = PyLong_AsLong(PyTuple_GetItem(args, 0));

	ssuge::CameraComponent * cc = self->mGameObject->createCameraComponent();
	cc->connectToViewport(APPLICATION->getViewport(vport_num));

	Py_INCREF(Py_None);
	return Py_None;
}




PyObject * script_GameObject_createColliderComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) < 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass at least a string indicating the type of component ('plane', 'box', 'sphere')");
		return NULL;
	}

	char * comp_type = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	if (!strcmp(comp_type, "plane"))
	{
		// Construct a plane object.  First, see if they are asking us to deduce dimensions
		if (PyTuple_Size(args) == 1)
			self->mGameObject->createColliderComponent(ssuge::ColliderType::PLANE);
		else if (PyTuple_Size(args) == 5 && PyNumber_Check(PyTuple_GetItem(args, 1)) && PyNumber_Check(PyTuple_GetItem(args, 2)) &&
			PyNumber_Check(PyTuple_GetItem(args, 3)) && PyNumber_Check(PyTuple_GetItem(args, 4)))
		{
			PyObject * nx = PyNumber_Float(PyTuple_GetItem(args, 1));
			PyObject * ny = PyNumber_Float(PyTuple_GetItem(args, 2));
			PyObject * nz = PyNumber_Float(PyTuple_GetItem(args, 3));
			PyObject * d = PyNumber_Float(PyTuple_GetItem(args, 4));
			float cnx = (float)PyFloat_AsDouble(nx);
			float cny = (float)PyFloat_AsDouble(ny);
			float cnz = (float)PyFloat_AsDouble(nz);
			float cd = (float)PyFloat_AsDouble(d);
			Py_DECREF(nx);
			Py_DECREF(ny);
			Py_DECREF(nz);
			Py_DECREF(d);

			self->mGameObject->createColliderComponent(Ogre::Vector3(cnx, cny, cnz), cd);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError, "You must pass a normal and a d-value");
			return NULL;
		}
	}
	else if (!strcmp(comp_type, "box"))
	{
		// Construct a box object.  First see if they're asking us to deduce dimensions
		if (PyTuple_Size(args) == 1)
			self->mGameObject->createColliderComponent(ssuge::ColliderType::BOX);
		else if (PyTuple_Size(args) == 4 && PyNumber_Check(PyTuple_GetItem(args, 1)) && PyNumber_Check(PyTuple_GetItem(args, 2)) &&
			PyNumber_Check(PyTuple_GetItem(args, 3)))
		{
			PyObject * ex = PyNumber_Float(PyTuple_GetItem(args, 1));
			PyObject * ey = PyNumber_Float(PyTuple_GetItem(args, 2));
			PyObject * ez = PyNumber_Float(PyTuple_GetItem(args, 3));
			float cex = (float)PyFloat_AsDouble(ex);
			float cey = (float)PyFloat_AsDouble(ey);
			float cez = (float)PyFloat_AsDouble(ez);
			Py_DECREF(ex);
			Py_DECREF(ey);
			Py_DECREF(ez);

			self->mGameObject->createColliderComponent(Ogre::Vector3(cex, cey, cez));
		}
		else
		{
			PyErr_SetString(PyExc_ValueError, "You must pass 3 half-extent values");
			return NULL;
		}
	}
	else if (!strcmp(comp_type, "sphere"))
	{
		// Construct a sphere bounding volume.  First, see if they're asking us to deduce dimensions
		if (PyTuple_Size(args) == 1)
			self->mGameObject->createColliderComponent(ssuge::ColliderType::SPHERE);
		else if (PyTuple_Size(args) == 2 && PyNumber_Check(PyTuple_GetItem(args, 1)))
		{
			PyObject * r = PyNumber_Float(PyTuple_GetItem(args, 1));
			float cr = (float)PyFloat_AsDouble(r);
			Py_DECREF(r);

			self->mGameObject->createColliderComponent(cr);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError, "You must pass a radius");
			return NULL;
		}
	}

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject * script_GameObject_createGUIComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 5 || !PyUnicode_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)) || 
		!PyNumber_Check(PyTuple_GetItem(args, 3)) || !PyNumber_Check(PyTuple_GetItem(args, 4)))
	{
		PyErr_SetString(PyExc_ValueError, "The first argument must be either 'text' or 'image', next 4 must be (x, y, width, height) in relative coordinates");
		return NULL;
	}

	char * type_string = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 2));
	PyObject * w = PyNumber_Float(PyTuple_GetItem(args, 3));
	PyObject * h = PyNumber_Float(PyTuple_GetItem(args, 4));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cw = (float)PyFloat_AsDouble(w);
	float ch = (float)PyFloat_AsDouble(h);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(w);
	Py_DECREF(h);

	if (!strcmp(type_string, "text"))
		self->mGameObject->createGUIComponent(ssuge::GUIType::TEXT, ssuge::Rectangle(cx, cy, cw, ch));
	else if (!strcmp(type_string, "image"))
		self->mGameObject->createGUIComponent(ssuge::GUIType::IMAGE, ssuge::Rectangle(cx, cy, cw, ch));
	else
	{
		PyErr_SetString(PyExc_ValueError, "First argument must be either 'text' or 'image'");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_createLightComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	// The user should pass a string: "direction", "spot", or "point" and 6 values
	//      1, 2, 3 = diffuse color (0.0 - 1.0 range)
	//      4, 5, 6 = specular color (0.0 - 1.0 range)
	// The user can optionally pass an inner and outer angle for spotlights (in degrees):

	if (!PyTuple_Check(args) || PyTuple_Size(args) < 7 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a string ('direction', 'point', or 'specular') and 6 number values (diffuse and specular colors, 0.0 - 1.0 range)");
		return NULL;
	}

	char * light_type_str = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	ssuge::LightType lt_type;
	if (!strcmp(light_type_str, "direction"))
		lt_type = ssuge::LightType::DIRECTIONAL;
	else if (!strcmp(light_type_str, "point"))
		lt_type = ssuge::LightType::POINT;
	else if (!strcmp(light_type_str, "spot"))
		lt_type = ssuge::LightType::SPOT;
	else
	{
		PyErr_SetString(PyExc_ValueError, "You must pass one of these strings for the first parameter: 'direction', 'point', or 'spot'");
		return NULL;
	}

	for (int i = 1; i < PyTuple_Size(args); i++)
	{
		if (!PyNumber_Check(PyTuple_GetItem(args, i)))
		{
			PyErr_SetString(PyExc_ValueError, "You must pass a number for all but the first argument");
			return NULL;
		}
	}

	PyObject * dr = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * dg = PyNumber_Float(PyTuple_GetItem(args, 2));
	PyObject * db = PyNumber_Float(PyTuple_GetItem(args, 3));
	PyObject * sr = PyNumber_Float(PyTuple_GetItem(args, 4));
	PyObject * sg = PyNumber_Float(PyTuple_GetItem(args, 5));
	PyObject * sb = PyNumber_Float(PyTuple_GetItem(args, 6));
	float cdr = (float)PyFloat_AsDouble(dr);
	float cdg = (float)PyFloat_AsDouble(dg);
	float cdb = (float)PyFloat_AsDouble(db);
	float csr = (float)PyFloat_AsDouble(dr);
	float csg = (float)PyFloat_AsDouble(dg);
	float csb = (float)PyFloat_AsDouble(db);
	Py_DECREF(dr);
	Py_DECREF(dg);
	Py_DECREF(db);
	Py_DECREF(sr);
	Py_DECREF(sg);
	Py_DECREF(sb);


	ssuge::LightComponent * LC = self->mGameObject->createLightComponent(lt_type);
	LC->setDiffuseColor(cdr, cdg, cdb);
	LC->setSpecularColor(csr, csg, csb);

	if (PyTuple_Size(args) == 9)
	{
		PyObject * inner = PyNumber_Float(PyTuple_GetItem(args, 7));
		PyObject * outer = PyNumber_Float(PyTuple_GetItem(args, 8));
		float cinner = (float)PyFloat_AsDouble(inner);
		float couter = (float)PyFloat_AsDouble(outer);
		Py_DECREF(inner);
		Py_DECREF(outer);
		LC->setSpotlightParams(cinner, couter);
	}

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject * script_GameObject_createSoundComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) < 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a filename of a sound and optional a boolean variable (for is_3d)");
		return NULL;
	}

	char * fname = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	bool is_3d = true;

	if (PyTuple_Size(args) == 2 && !PyObject_IsTrue(PyTuple_GetItem(args, 1)))
		is_3d = false;

	self->mGameObject->createSoundComponent(std::string(fname), is_3d);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject * script_GameObject_getCameraRay(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 2 || !PyFloat_Check(PyTuple_GetItem(args, 0)) ||
		!PyFloat_Check(PyTuple_GetItem(args, 1)))
	{
		PyErr_SetString(PyExc_ValueError, "Pass two normalized screen coordinates (as float)");
		return NULL;
	}

	if (self->mGameObject->getNumComponents(ssuge::ComponentType::CAMERA) > 0)
	{
		float nx = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 0));
		float ny = (float)PyFloat_AsDouble(PyTuple_GetItem(args, 1));

		// Just default to first camera -- not sure why we'd want to allow multiple cameras...
		ssuge::CameraComponent * cc = self->mGameObject->getCameraComponent(0);
		Ogre::Ray ray = cc->getScreenRay(Ogre::Vector2(nx, ny));
		Ogre::Vector3 origin = ray.getOrigin();
		Ogre::Vector3 direction = ray.getDirection();

		PyObject * rval = PyTuple_New(6);
		PyTuple_SetItem(rval, 0, PyFloat_FromDouble((double)origin.x));
		PyTuple_SetItem(rval, 1, PyFloat_FromDouble((double)origin.y));
		PyTuple_SetItem(rval, 2, PyFloat_FromDouble((double)origin.z));
		PyTuple_SetItem(rval, 3, PyFloat_FromDouble((double)direction.x));
		PyTuple_SetItem(rval, 4, PyFloat_FromDouble((double)direction.y));
		PyTuple_SetItem(rval, 5, PyFloat_FromDouble((double)direction.z));
		return rval;
	}
	else
	{
		Py_INCREF(Py_None);
		return Py_None;
	}

}




PyObject* script_GameObject_getGroupName(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This method takes no arguments (besides self)");
		return NULL;
	}

	std::string s = self->mGameObject->getGroupName();

	return PyUnicode_FromString(s.c_str());
}



PyObject * script_GameObject_getName(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	return PyUnicode_FromString(self->mGameObject->getName().c_str());
}



PyObject* script_GameObject_getParentPosition(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This method takes no arguments (besides self)");
		return NULL;
	}

	PyObject * rval = PyTuple_New(3);
	Ogre::Vector3 v = self->mGameObject->getPosition();
	PyTuple_SetItem(rval, 0, PyFloat_FromDouble((double)v.x));
	PyTuple_SetItem(rval, 1, PyFloat_FromDouble((double)v.y));
	PyTuple_SetItem(rval, 2, PyFloat_FromDouble((double)v.z));

	return rval;
}



PyObject* script_GameObject_getWorldPosition(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This method takes no arguments (besides self)");
		return NULL;
	}

	PyObject * rval = PyTuple_New(3);
	Ogre::Vector3 v = self->mGameObject->getWorldPosition();
	PyTuple_SetItem(rval, 0, PyFloat_FromDouble((double)v.x));
	PyTuple_SetItem(rval, 1, PyFloat_FromDouble((double)v.y));
	PyTuple_SetItem(rval, 2, PyFloat_FromDouble((double)v.z));

	return rval;
}



PyObject * script_GameObject_lookAt(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "This method takes 3 number arguments (look at position)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	self->mGameObject->lookAt(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* script_GameObject_playSoundComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::SOUND) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "You must have a sound component on this game object before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This argument takes no parameters");
		return NULL;
	}

	ssuge::SoundComponent * sc = self->mGameObject->getSoundComponent(0);
	sc->play();

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_rotateWorld(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 4 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)) || !PyNumber_Check(PyTuple_GetItem(args, 3)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 4 number parameters: degrees, x,y,z (axis)");
		return NULL;
	}

	PyObject * deg = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 2));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 3));
	float cdeg = (float)PyFloat_AsDouble(deg);
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(deg);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->rotateWorld(cdeg, cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setGUIAligmnent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::GUI) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "This game object must have a GUI component attached before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || !PyUnicode_Check(PyTuple_GetItem(args, 0)) || !PyUnicode_Check(PyTuple_GetItem(args, 1)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass two strings to this method");
		return NULL;
	}

	char * horiz_align = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	char * vert_align = PyUnicode_AsUTF8(PyTuple_GetItem(args, 1));

	ssuge::HorizontalAlignmentType htype;
	ssuge::VerticalAlignmentType vtype;

	if (!strcmp(horiz_align, "left"))
		htype = ssuge::HorizontalAlignmentType::LEFT;
	else if (!strcmp(horiz_align, "center"))
		htype = ssuge::HorizontalAlignmentType::CENTER;
	else if (!strcmp(horiz_align, "right"))
		htype = ssuge::HorizontalAlignmentType::RIGHT;
	else
	{
		PyErr_SetString(PyExc_ValueError, "Invalid horizontal alignment type");
		return NULL;
	}

	if (!strcmp(vert_align, "top"))
		vtype = ssuge::VerticalAlignmentType::TOP;
	else if (!strcmp(vert_align, "center"))
		vtype = ssuge::VerticalAlignmentType::CENTER;
	else if (!strcmp(vert_align, "bottom"))
		vtype = ssuge::VerticalAlignmentType::BOTTOM;
	else
	{
		PyErr_SetString(PyExc_ValueError, "Invalid vertical alignment type");
		return NULL;
	}

	ssuge::GUIComponent * elem = self->mGameObject->getGUIComponent(0);
	elem->setAlignment(htype, vtype);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setGUIColor(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::GUI) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "This game object must have a GUI component attached before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 6 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 3)) || !PyNumber_Check(PyTuple_GetItem(args, 4)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 5)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 6 numbers (3 for the top color, 3 for the bottom color, each 0.0-1.0 range)");
		return NULL;
	}

	PyObject * rt = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * gt = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * bt = PyNumber_Float(PyTuple_GetItem(args, 2));
	PyObject * rb = PyNumber_Float(PyTuple_GetItem(args, 3));
	PyObject * gb = PyNumber_Float(PyTuple_GetItem(args, 4));
	PyObject * bb = PyNumber_Float(PyTuple_GetItem(args, 5));
	float crt = (float)PyFloat_AsDouble(rt);
	float cgt = (float)PyFloat_AsDouble(gt);
	float cbt = (float)PyFloat_AsDouble(bt);
	float crb = (float)PyFloat_AsDouble(rb);
	float cgb = (float)PyFloat_AsDouble(gb);
	float cbb = (float)PyFloat_AsDouble(bb);
	Py_DECREF(rt);
	Py_DECREF(gt);
	Py_DECREF(bt);
	Py_DECREF(rb);
	Py_DECREF(gb);
	Py_DECREF(bb);

	self->mGameObject->getGUIComponent(0)->setGUITextColor(Ogre::ColourValue(crt, cgt, cbt), Ogre::ColourValue(crb, cgb, cbb));

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setGUIMaterial(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::GUI) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "This game object must have a GUI component attached before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a (string) material name");
		return NULL;
	}

	char * mat_name = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	self->mGameObject->getGUIComponent(0)->setMaterial(std::string(mat_name));

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setGUIVisible(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::GUI) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "This method only works if the game object has a GUI component");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a single boolean value");
		return NULL;
	}

	ssuge::GUIComponent * guic = self->mGameObject->getGUIComponent(0);		// Get index from user?
	guic->setVisible(PyObject_IsTrue(PyTuple_GetItem(args, 0)) ? true : false);
	
	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setMeshMaterial(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) < 2 || !PyLong_Check(PyTuple_GetItem(args, 0)) || !PyUnicode_Check(PyTuple_GetItem(args, 1)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass the sub-entity (integer) and material name (string) and an optional mesh-component number (defaults to 0)");
		return NULL;
	}
	

	int sub_ent = PyLong_AsLong(PyTuple_GetItem(args, 0));
	char * mat_name = PyUnicode_AsUTF8(PyTuple_GetItem(args, 1));
	int mesh_index = 0;
	if (PyTuple_Size(args) == 3 && PyLong_Check(PyTuple_GetItem(args, 2)))
		mesh_index = PyLong_AsLong(PyTuple_GetItem(args, 2));

	if ((int)self->mGameObject->getNumComponents(ssuge::ComponentType::MESH) < mesh_index + 1)
	{
		PyErr_SetString(PyExc_ValueError, "This object must have a mesh component before calling this");
		return NULL;
	}

	ssuge::MeshComponent * mc = self->mGameObject->getMeshComponent(mesh_index);
	mc->setMaterial(sub_ent, std::string(mat_name));

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_setOrientation(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 4 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 3)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 4 number parameters: degrees,x,y,z (quaternion)");
		return NULL;
	}

	PyObject * deg = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 2));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 3));
	float cdeg = (float)PyFloat_AsDouble(deg);
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(deg);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->setOrientation(cdeg, cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* script_GameObject_setPosition(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 3 number parameters: x,y,z (parent-position-offset)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->setPosition(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* script_GameObject_setPositionWorld(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 3 number parameters: x,y,z (world position)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->setPositionWorld(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_setScale(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 3 number parameters: x,y,z (scale factor)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->setScale(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_stopSoundComponent(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::SOUND) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "You must have a sound component on this game object before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "This argument takes no parameters");
		return NULL;
	}

	ssuge::SoundComponent * sc = self->mGameObject->getSoundComponent(0);
	sc->stop();

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_setSoundLooping(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::SOUND) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "You must have a sound component on this object before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1)
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a single boolean argument to this method");
		return NULL;
	}

	ssuge::SoundComponent * sc = self->mGameObject->getSoundComponent(0);
	bool val = false;
	if (PyObject_IsTrue(PyTuple_GetItem(args, 0)))
		val = true;
	sc->setLooping(val);

	Py_INCREF(Py_None);
	return Py_None;
}



PyObject* script_GameObject_translateLocal(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 3 number parameters: x,y,z (loca translation vector)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->translateLocal(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject* script_GameObject_translateWorld(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 3 || !PyNumber_Check(PyTuple_GetItem(args, 0)) ||
		!PyNumber_Check(PyTuple_GetItem(args, 1)) || !PyNumber_Check(PyTuple_GetItem(args, 2)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 3 number parameters: x,y,z (world translation vector)");
		return NULL;
	}

	PyObject * x = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * y = PyNumber_Float(PyTuple_GetItem(args, 1));
	PyObject * z = PyNumber_Float(PyTuple_GetItem(args, 2));
	float cx = (float)PyFloat_AsDouble(x);
	float cy = (float)PyFloat_AsDouble(y);
	float cz = (float)PyFloat_AsDouble(z);
	Py_DECREF(x);
	Py_DECREF(y);
	Py_DECREF(z);

	script_GameObject * sobj = (script_GameObject*)self;
	sobj->mGameObject->translateWorld(cx, cy, cz);

	Py_INCREF(Py_None);
	return Py_None;
}


PyObject * script_GameObject_updateGUIText(script_GameObject * self, PyObject * args, PyObject * kwargs)
{
	if (self->mGameObject->getNumComponents(ssuge::ComponentType::GUI) == 0)
	{
		PyErr_SetString(PyExc_ValueError, "This game object must have a GUI component attached before calling this method");
		return NULL;
	}
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a single string to this method");
		return NULL;
	}

	char * text = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	self->mGameObject->getGUIComponent(0)->setText(std::string(text));

	Py_INCREF(Py_None);
	return Py_None;
}



PyMemberDef script_GameObject_members[] =
{
	{ NULL }
};


PyMethodDef script_GameObject_methods[] =
{
	{"addChild", (PyCFunction)script_GameObject_addChild, METH_VARARGS, "adds the passed child game object as a child of this game object."},
	{"createMeshComponent", (PyCFunction)script_GameObject_createMeshComponent, METH_VARARGS, "creates a mesh component on this game object"},
	{"createCameraComponent", (PyCFunction)script_GameObject_createCameraComponent, METH_VARARGS, "creates a camera component and attaches it to the given viewport #"},
	{"createColliderComponent", (PyCFunction)script_GameObject_createColliderComponent, METH_VARARGS, "creates a physics component and attached it to this object" },
	{"createGUIComponent", (PyCFunction)script_GameObject_createGUIComponent, METH_VARARGS, "creates a gui component" },
	{"createLightComponent", (PyCFunction)script_GameObject_createLightComponent, METH_VARARGS, "creates a light component"},
	{"createSoundComponent", (PyCFunction)script_GameObject_createSoundComponent, METH_VARARGS, "creates a sound component"},
	{"getGroupName", (PyCFunction)script_GameObject_getGroupName, METH_VARARGS, "gets the group name (in the GOM) that this object belongs to"},
	{"getCameraRay", (PyCFunction)script_GameObject_getCameraRay, METH_VARARGS, "gets a ray (6 numbers, 3 for origin, 3 for direction) from the camera component through the given normalized screen coordinates (nx, ny)"},
	{"getName", (PyCFunction)script_GameObject_getName, METH_VARARGS, "gets the name of this game object"},
	{"getParentPosition", (PyCFunction)script_GameObject_getParentPosition, METH_VARARGS, "gets this objects position relative to its parent" },
	{"getWorldPosition", (PyCFunction)script_GameObject_getWorldPosition, METH_VARARGS, "gets world-space position"},
	{"lookAt", (PyCFunction)script_GameObject_lookAt, METH_VARARGS, "makes this object's negative z-axis point towards the given point (in world space)"},
	{"playSoundComponent", (PyCFunction)script_GameObject_playSoundComponent, METH_VARARGS, "(restarts and) plays the first sound component on this game object"},
	{"rotateWorld", (PyCFunction)script_GameObject_rotateWorld, METH_VARARGS, "rotates the game object (in world space)"},
	{"setGUIAlignment", (PyCFunction)script_GameObject_setGUIAligmnent, METH_VARARGS, "sets the gui alignment mode for the attached gui element"},
	{"setGUIColor", (PyCFunction)script_GameObject_setGUIColor, METH_VARARGS, "sets the color (top and bottom) of a gui (text) element" },
	{"setGUIMaterial", (PyCFunction)script_GameObject_setGUIMaterial, METH_VARARGS, "sets the material for an (image) gui element component"},
	{"setGUIVisible", (PyCFunction)script_GameObject_setGUIVisible, METH_VARARGS, "sets the gui element to visible if True is passed"},
	{"setMeshMaterial", (PyCFunction)script_GameObject_setMeshMaterial, METH_VARARGS, "sets the mesh component (if there is one) to the given material"},
	{"setOrientation", (PyCFunction)script_GameObject_setOrientation, METH_VARARGS, "sets the orientation of this game object"},
	{"setPositionParent", (PyCFunction)script_GameObject_setPosition, METH_VARARGS, "sets the position of this object (relative to the parent)"},
	{"setPositionWorld", (PyCFunction)script_GameObject_setPositionWorld, METH_VARARGS, "sets the position of this object (relative to the world)"},
	{"setScale", (PyCFunction)script_GameObject_setScale, METH_VARARGS, "scales this object (relative to current scale)"},
	{"setSoundLooping", (PyCFunction)script_GameObject_setSoundLooping, METH_VARARGS, "sets the looping state of the sound"},
	{"stopSoundComponent", (PyCFunction)script_GameObject_stopSoundComponent, METH_VARARGS, "stops this sound component playback (if there is one)"},
	{"translateLocal", (PyCFunction)script_GameObject_translateLocal, METH_VARARGS, "translates this object (relative to its local axes)"},
	{"translateWorld", (PyCFunction)script_GameObject_translateWorld, METH_VARARGS, "translates this object (relative to the world axes)" },
	{"updateGUIText", (PyCFunction)script_GameObject_updateGUIText, METH_VARARGS, "sets the text of a gui element attached to this game object"},
	{ NULL, NULL, 0, NULL }
};



PyTypeObject script_GameObjectType =
{
	PyVarObject_HEAD_INIT(NULL, 0)					// Initializes the header-stuff
	"python GameObject class",						// tp_name
	sizeof(script_GameObject),						// tp_basicsize
	0,												// tp_itemsize
	(destructor)script_GameObject_dealloc,			// tp_dealloc
	0,												// tp_print
	0,												// tp_getattr
	0,												// tp_setattr
	0,												// tp_reserved
	0,												// tp_repr
	0,												// tp_as_number
	0,												// tp_as_sequence
	0, 												// tp_as_mapping
	0,												// tp_hash
	0,												// tp_call
	(reprfunc)script_GameObject_str,				// tp_str
	0,												// tp_getattro
	0,												// tp_setattro
	0,												// tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,		// tp_flags
	"ssuge GameObject wrapper",						// tp_doc
	0,												// tp_traverse
	0,												// tp_clear
	0,												// tp_richcompare
	0,												// tp_weaklistoffset
	0,												// tp_iter
	0,												// tp_iternext
	script_GameObject_methods,						// tp_methods
	script_GameObject_members,						// tp_members
	0,												// tp_getset
	0,												// tp_base
	0,												// tp_dict
	0,												// tp_descr_get
	0,												// tp_descr_set
	0,												// tp_dictoffset
	(initproc)script_GameObject_init,				// tp_init
	0,												// tp_alloc
	script_GameObject_new,							// tp_new
													// (there are more, we just needed to get down to tp_new...)
};
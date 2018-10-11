#include <stdafx.h>
#include <log_manager.h>
#include <game_object_manager.h>
#include <script_game_object.h>
#include <script_manager.h>
#include <input_manager.h>
#include <application.h>
#include <collision_manager.h>

extern PyTypeObject script_GameObjectType;


static PyObject * createGroup(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with one string argument.");
		return NULL;
	}

	char * cstr = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	GAME_OBJECT_MANAGER->createGroup(std::string(cstr));

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * getAxis(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 2 || !PyUnicode_Check(PyTuple_GetItem(args, 0)) ||
		!PyLong_Check(PyTuple_GetItem(args, 1)))
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with one string argument [the name of the axis] and an integer [which axis].");
		return NULL;
	}

	// Get the name of the axis
	std::string axis_name = std::string(PyUnicode_AsUTF8(PyTuple_GetItem(args, 0)));
	int axis_num = PyLong_AsLong(PyTuple_GetItem(args, 1));

	return PyFloat_FromDouble((float)INPUT_MANAGER->getAxis(axis_name, axis_num));
}


static PyObject * getMouseNormalized(PyObject * module, PyObject * args)
{
	Ogre::Vector2 mpos = INPUT_MANAGER->getMousePosition();

	PyObject * rval = PyTuple_New(2);
	PyTuple_SetItem(rval, 0, PyFloat_FromDouble((double)mpos.x));
	PyTuple_SetItem(rval, 1, PyFloat_FromDouble((double)mpos.y));
	return rval;
}


static PyObject * loadIni(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with one string argument.");
		return NULL;
	}

	char * cstr = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	// Actually load the group
	if (GAME_OBJECT_MANAGER->loadGroup(std::string(cstr)))
		LOG_MANAGER->log("Error loading ini file '" + std::string(cstr) + "'", SCRIPT_MANAGER->msTimeToShowScriptErrors);

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * loadScript(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with one string argument.");
		return NULL;
	}

	char * cstr = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	// Actually run the script
	PyObject * result = SCRIPT_MANAGER->loadScript(std::string(cstr));
	if (result == NULL)
		LOG_MANAGER->log("Error loading script file from python: '" + std::string(cstr) + "'", SCRIPT_MANAGER->msTimeToShowScriptErrors);

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * log(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) < 1 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with at least one string argument.");
		return NULL;
	}

	char * cstr = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));

	if (PyTuple_Size(args) == 2 && PyNumber_Check(PyTuple_GetItem(args, 1)))
	{
		PyObject * num = PyNumber_Float(PyTuple_GetItem(args, 1));
		double cnum = PyFloat_AsDouble(num);
		Py_DECREF(num);

		LOG_MANAGER->log("[SCRIPT] " + std::string(cstr), (float)cnum);		// display on-screen too
	}
	else
		LOG_MANAGER->log("[SCRIPT] " + std::string(cstr));

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * rayCast(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 7 || !PyUnicode_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass 7 values: first is a group name, next 3 are the ray's origin, last 3 are the ray's direction");
		return NULL;
	}
	else
	{
		bool ok = true;
		for (int i = 1; i < 7; i++)
		{
			if (!PyNumber_Check(PyTuple_GetItem(args, i)))
			{
				PyErr_SetString(PyExc_ValueError, "You must pass 7 values: first is a group name, next 3 are the ray's origin, last 3 are the ray's direction");
				return NULL;
			}
		}
	}

	// Get the group id (and make sure we have a valid group name)
	char * group_name = PyUnicode_AsUTF8(PyTuple_GetItem(args, 0));
	int group_id = GAME_OBJECT_MANAGER->getGroupID(std::string(group_name));
	if (group_id < 0)
	{
		// Invalid group-name
		PyErr_SetString(PyExc_ValueError, "Invalid group name");
		return NULL;
	}

	// Get number values
	float nums[6];
	for (int i = 1; i < 7; i++)
	{
		PyObject * temp = PyNumber_Float(PyTuple_GetItem(args, i));
		nums[i - 1] = (float)PyFloat_AsDouble(temp);
		Py_DECREF(temp);
	}

	// Do the raycast.  Note: I'm only returning script-aware game objects
	std::vector<ssuge::GameObject*> hit_objects;
	std::vector<Ogre::Vector3> hit_pts;
	Ogre::Ray ray(Ogre::Vector3(nums[0], nums[1], nums[2]), Ogre::Vector3(nums[3], nums[4], nums[5]));
	COLLISION_MANAGER->rayCast(group_id, ray, hit_objects, hit_pts);

	// Pick out the script-aware game objects and build a return tuple out of them. 
	// Question for self: do we want to include non-script aware game objects here too (perhaps by name?)
	int num = 0;
	for (unsigned int i = 0; i < hit_objects.size(); i++)
	{
		if (hit_objects[i]->getScriptTwin() != NULL)
			++num;
	}
	PyObject * rval = PyTuple_New(num);
	int index = 0;
	for (unsigned int i = 0; i < hit_objects.size(); i++)
	{
		PyObject * twin = hit_objects[i]->getScriptTwin();
		if (twin != NULL)
		{
			Py_INCREF(twin);				// PyTuple_SetItem will steal a reference
			PyObject * sub_tuple = PyTuple_New(4);
			PyTuple_SetItem(sub_tuple, 0, twin);
			PyTuple_SetItem(sub_tuple, 1, PyFloat_FromDouble(hit_pts[i].x));
			PyTuple_SetItem(sub_tuple, 2, PyFloat_FromDouble(hit_pts[i].y));
			PyTuple_SetItem(sub_tuple, 3, PyFloat_FromDouble(hit_pts[i].z));

			PyTuple_SetItem(rval, index, sub_tuple);
			index++;
		}
	}
	return rval;
}


static PyObject * registerInputListener(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyObject_IsInstance(PyTuple_GetItem(args, 0), (PyObject*)&script_GameObjectType))
	{
		PyErr_SetString(PyExc_TypeError, "You must pass a ssuge.GameObject-derived object to this function");
		return NULL;
	}

	script_GameObject * gobj = (script_GameObject*)PyTuple_GetItem(args, 0);

	INPUT_MANAGER->registerListener(gobj->mGameObject);

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * setAudioListener(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyObject_IsInstance(PyTuple_GetItem(args, 0), (PyObject*)&script_GameObjectType))
	{
		PyErr_SetString(PyExc_ValueError, "You must pass a ssuge.GameObject as the only parameter");
		return NULL;
	}

	script_GameObject * gobj = (script_GameObject*)PyTuple_GetItem(args, 0);

	SOUND_MANAGER->setActiveListener(gobj->mGameObject);

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * shutdown(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 0)
	{
		PyErr_SetString(PyExc_ValueError, "You must call this method with no arguments.");
		return NULL;
	}

	APPLICATION->startShutdown();

	Py_INCREF(Py_None);
	return Py_None;
}



static PyObject * setMouseVisibility(PyObject * module, PyObject * args)
{
	if (!PyTuple_Check(args) || PyTuple_Size(args) != 1 || !PyBool_Check(PyTuple_GetItem(args, 0)))
	{
		PyErr_SetString(PyExc_ValueError, "Pass a boolean value");
		return NULL;
	}

	if (PyObject_IsTrue(PyTuple_GetItem(args, 0)))
		INPUT_MANAGER->setMouseVisible(true);
	else
		INPUT_MANAGER->setMouseVisible(false);

	Py_INCREF(Py_None);
	return Py_None;
}


static PyObject * MoveTowards(PyObject * module, PyObject * args)
{
	// Found this in Unity's Vector3.MoveTowards() converted from C#
	PyObject * c = PyTuple_GetItem(args, 0);
	PyObject * t = PyTuple_GetItem(args, 1);

	float cx = (float)PyFloat_AsDouble((PyTuple_GetItem(c, 0)));
	float cy = (float)PyFloat_AsDouble((PyTuple_GetItem(c, 1)));
	float cz = (float)PyFloat_AsDouble((PyTuple_GetItem(c, 2)));

	float tx = (float)PyFloat_AsDouble((PyTuple_GetItem(t, 0)));
	float ty = (float)PyFloat_AsDouble((PyTuple_GetItem(t, 1)));
	float tz = (float)PyFloat_AsDouble((PyTuple_GetItem(t, 2)));

	float maxDistanceDelta = (float)PyFloat_AsDouble(PyNumber_Float(PyTuple_GetItem(args, 2)));

	Ogre::Vector3 current(cx, cy, cz);
	Ogre::Vector3 target(tx, ty, tz);

	Ogre::Vector3 a = target - current;
	float magnitude = a.length();

	PyObject * out = PyTuple_New(3);
	if (magnitude <= maxDistanceDelta || magnitude == 0.0f)
	{
		PyTuple_SetItem(out, 0, PyFloat_FromDouble((double)cx));
		PyTuple_SetItem(out, 1, PyFloat_FromDouble((double)cy));
		PyTuple_SetItem(out, 2, PyFloat_FromDouble((double)cz));
		return out;
	}

	Ogre::Vector3 towards = current + a / magnitude * maxDistanceDelta;
	PyTuple_SetItem(out, 0, PyFloat_FromDouble((double)towards.x));
	PyTuple_SetItem(out, 1, PyFloat_FromDouble((double)towards.y));
	PyTuple_SetItem(out, 2, PyFloat_FromDouble((double)towards.z));

	return out;
}

static PyObject * getRandomInt(PyObject * module, PyObject * args)
{
	PyObject * min = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * max = PyNumber_Float(PyTuple_GetItem(args, 1));
	float cmin = (float)PyFloat_AsDouble(min);
	float cmax = (float)PyFloat_AsDouble(max);
	int output = cmin + (rand() % static_cast<int>(cmax - cmin + 1));

	return PyFloat_FromDouble((float)output);
}

static PyObject * getRandomFloat(PyObject * module, PyObject * args)
{
	PyObject * min = PyNumber_Float(PyTuple_GetItem(args, 0));
	PyObject * max = PyNumber_Float(PyTuple_GetItem(args, 1));
	float cmin = (float)PyFloat_AsDouble(min);
	float cmax = (float)PyFloat_AsDouble(max);
	float output = ((float(rand()) / float(RAND_MAX)) * (cmax - cmin)) + cmin;

	return PyFloat_FromDouble(output);
}


PyMethodDef ssuge_functions[] =
{
	{ "log", log, METH_VARARGS, "logs a message to the ssuge-log (using the C++ log manager" },
	{ "getAxis", getAxis, METH_VARARGS, "gets a named axis from an input device" },
	{ "createGroup", createGroup, METH_VARARGS, "Creates a game object manager group with the given name." },
	{ "getMouseNormalized", getMouseNormalized, METH_VARARGS, "Returns the mouse coordinates as a tuple, each element in the range 0.0-1.0"},
	{ "loadIni", loadIni, METH_VARARGS, "Loads an ini-style script through the GameObjectManager" },
	{ "loadScript", loadScript, METH_VARARGS, "Loads and runs a python script" },
	{ "rayCast", rayCast, METH_VARARGS, "Pass a ray (3 nums for position, 3 for direction) -- returns a tuple of pairs (game-object, hit-point)"},
	{ "registerInputListener", registerInputListener, METH_VARARGS, "registers a game object as an input listener" },
	{ "setAudioListener", setAudioListener, METH_VARARGS, "sets the audio listener to the passed game object"},
	{ "setMouseVisibility", setMouseVisibility, METH_VARARGS, "changes the mouse visibility" },
	{ "shutdown", shutdown, METH_VARARGS, "Shuts down the application" },
	{ "getRandomFloat", getRandomFloat, METH_VARARGS, "Gets a random float in a range" },
	{ "getRandomInt", getRandomInt, METH_VARARGS, "Gets a random int in a range" },
	{ "MoveTowards", MoveTowards, METH_VARARGS, "Shuts down the application" },
	{ NULL, NULL, 0, NULL }
};
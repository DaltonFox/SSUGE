#include <stdafx.h>

// A hint telling the linker to look in another C file for this variable
extern PyTypeObject script_GameObjectType;
extern PyMethodDef ssuge_functions[];


struct PyModuleDef ssuge_definition =
{
	PyModuleDef_HEAD_INIT,
	"python ssuge module",
	"our embedded python ssuge scripting module",
	-1,
	ssuge_functions
};

PyMODINIT_FUNC PyInit_ssuge(void)
{
	PyObject * mod = PyModule_Create(&ssuge_definition);
	if (mod == NULL)
		return NULL;

	// Add the game object "class" to our module
	if (PyType_Ready(&script_GameObjectType) < 0)
		return NULL;
	Py_INCREF(&script_GameObjectType);
	PyModule_AddObject(mod, "GameObject", (PyObject*)&script_GameObjectType);

	return mod;
}

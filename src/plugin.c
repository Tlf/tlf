#include <stdbool.h>
#include <glib.h>

#include <Python.h>

#include "tlf.h"
#include "log_utils.h"
#include "utils.h"

// 	python3-dev

static PyObject *pModule, *pDict;

//== define pf_X pointer and plugin_has_X function
#define PLUGIN_FUNC(name) \
    static PyObject *pf_##name; \
    bool plugin_has_##name() { return (pf_##name != NULL); }
//==

PLUGIN_FUNC(setup)
PLUGIN_FUNC(add_qso)
PLUGIN_FUNC(is_multi)
PLUGIN_FUNC(nr_of_mults)


//=============
// example callback into C code
static PyObject *say_hello(PyObject *self, PyObject *args) {
    const char *name;

    if (!PyArg_ParseTuple(args, "s", &name))
	return NULL;

    printf("TLF: Hello %s!\n", name);

    Py_RETURN_NONE;
}

static  PyMethodDef tlf_methods[] = {
    { "say_hello", say_hello, METH_VARARGS, "Say hello to arg" },
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef tlf_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "tlf",
    .m_size = -1,
    .m_methods = tlf_methods
};

static PyMODINIT_FUNC PyModInit_tlf(void) {
    printf("# in PyModInit_tlf\n");
    PyObject *tlf = PyModule_Create(&tlf_module);
    PyModule_AddIntMacro(tlf, CWMODE);
    PyModule_AddIntMacro(tlf, SSBMODE);
    PyModule_AddIntConstant(tlf, "BAND_ANY", BANDINDEX_ANY);
    //...

    return tlf;
}

static PyStructSequence_Desc qso_descr = {
    .name = "qso",
    .doc = "QSO data",
    .fields = (PyStructSequence_Field[]) {
	{.name = "band"},
	{.name = "call"},
	{.name = "mode"},
        // utc, comment/exchange, ...
	{.name = NULL}  // guard
    },
    .n_in_sequence = 3
};

static PyTypeObject *qso_type;

//=============
// forward declarations, to be removed
void plugin_setup(); 
void plugin_add_qso(const char *logline);
int plugin_nr_of_mults(int band);
bool plugin_is_multi(int band, const char *call, int mode);
//=============

static void lookup_function(const char *name, PyObject **pf_ptr) {
    // pf_ptr is a borrowed reference
    *pf_ptr = PyDict_GetItemString(pDict, name);
    if (*pf_ptr != NULL && !PyCallable_Check(*pf_ptr)) {
	Py_DECREF(*pf_ptr);
	*pf_ptr = NULL;
    }

    if (*pf_ptr == NULL) { //remove this
	printf("  no %s\n", name);
    }
}

void plugin_init(const char *name) {
    // determine directory containing the plugin
    char *py_name = g_strdup_printf("rules/%s.py", name);
    char *path = find_available(py_name);
    g_free(py_name);
    printf("path=%s\n", path);
    if (*path == 0) {
	g_free(path);
	// no plugin to be loaded
	return;
    }
    char *last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
	g_free(path);
	// "Unable to load ...
	return; // should not happen
    }
    *last_slash = 0;    // keep directory only
    printf("dir =%s\n", path);

    char *set_path = g_strdup_printf(
			 "import sys\n"
			 "sys.path.insert(0, '%s')\n"
			 "print(sys.path)\n"
			 , path);
    g_free(path);

    // Initialize the Python Interpreter
    PyImport_AppendInittab("tlf", &PyModInit_tlf); // declare tlf module
    Py_Initialize();

    PyRun_SimpleString(set_path);   // set module search path
    g_free(set_path);

    printf("path set\n");

    // Load the module object
    PyObject *pName = PyUnicode_FromString(name);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
	printf("plugin not found\n");
	PyErr_Print(); //? show exception
	return;
    }

    // pDict is a borrowed reference
    pDict = PyModule_GetDict(pModule);

    PyObject *pf_init;
    lookup_function("init", &pf_init);
    lookup_function("setup", &pf_setup);
    lookup_function("add_qso", &pf_add_qso);
    lookup_function("is_multi", &pf_is_multi);
    lookup_function("nr_of_mults", &pf_nr_of_mults);

    if (pf_setup == NULL) {
	printf("ERROR: missing setup\n");
	return;
    }

    // call init if available
    if (pf_init != NULL) {
	PyObject *pValue = PyObject_CallObject(pf_init, NULL);
	// ...check exception....
	Py_XDECREF(pValue);
    }


    // check add_qso ?

    // build qso type
    qso_type = PyStructSequence_NewType(&qso_descr);

#if 0
    plugin_setup();

    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  OK1AY          599  599  003                    0   3540.0");
    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  OK1Z           599  599  003                    0   3540.0");
    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  HA8QSY         599  599  003                    0   3540.0");

    int n = plugin_nr_of_mults(BANDINDEX_ANY);
    printf("n=%d\n", n);

    printf("S51Z multi: %d\n", plugin_is_multi(1, "S51Z", CWMODE));
    printf("9A1A multi: %d\n", plugin_is_multi(1, "9A1A", CWMODE));

    exit(0);
#endif
}

void plugin_close() {
    // Clean up
    Py_DECREF(pModule);
    //FIXME check other pointers

    // Finish the Python Interpreter
    Py_Finalize();
}

void plugin_setup() {
    PyObject *pValue = PyObject_CallObject(pf_setup, NULL);
    printf("after pf_setup, pValue %s NULL\n", pValue == NULL ? "is" : "not");
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	// FIXME: action?
    }
}

static PyObject *create_py_qso(int band, const char *call, int mode) {
    PyObject *qso = PyStructSequence_New(qso_type);
    PyStructSequence_SetItem(qso, 0, Py_BuildValue("i", band));
    PyStructSequence_SetItem(qso, 1, Py_BuildValue("s", call));
    PyStructSequence_SetItem(qso, 2, Py_BuildValue("i", mode));
    return qso;
}

//bool plugin_has_add_qso() {
//    return (pf_add_qso != NULL);
//}

void plugin_add_qso(const char *logline) {
//    if (pf_add_qso == NULL) {
//        return;
//    }
//    printf("--- in add_qso |%s|\n", logline);
// parse: call, ...
    char call[14];
    g_strlcpy(call, logline + 29, sizeof(call));
    g_strchomp(call);
    int mode = log_get_mode(logline);
    int band = log_get_band(logline); //atoi(logline);

    PyObject *qso = create_py_qso(band, call, mode);

    // call add_qso
    PyObject *arg = Py_BuildValue("(O)", qso);
    PyObject *pValue = PyObject_CallObject(pf_add_qso, arg);
    Py_DECREF(arg);
    Py_DECREF(qso);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	//FIXME: action?
    }
    Py_XDECREF(pValue);
}

bool plugin_is_multi(int band, const char *call, int mode) {
    // call is_multi
    PyObject *qso = create_py_qso(band, call, mode);
    PyObject *args = Py_BuildValue("(O)", qso);
    PyObject *pValue = PyObject_CallObject(pf_is_multi, args);
    Py_DECREF(args);
    Py_DECREF(qso);

    bool result = false;
    if (pValue != NULL) {
	result =  PyLong_AsLong(pValue);
    }
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	sleep(2);
	//exit(1);
    }
    return result;
}

int plugin_nr_of_mults(int band) {
    PyObject *args = Py_BuildValue("(i)", band);
    PyObject *pValue = PyObject_CallObject(pf_nr_of_mults, args);
    Py_DECREF(args);

    int result = 0;
    if (pValue != NULL) {
	result = PyLong_AsLong(pValue);
    }
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
    }
    return result;
}


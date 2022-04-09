#include <stdbool.h>
#include <glib.h>

#include <config.h>

#ifdef HAVE_PYTHON
#include <Python.h>
#else
#define PyObject    void
#endif

#include "tlf.h"
#include "globalvars.h"
#include "log_utils.h"
#include "startmsg.h"
#include "utils.h"
#include "qrb.h"

// hacks:
#define PARSE_OK    0
#define PARSE_ERROR 1
#define BANDINDEX_ANY   (-1)

// 	python3-dev

#ifdef HAVE_PYTHON
static PyObject *pModule, *pDict, *pTlf;
#endif

//== define pf_X pointer and plugin_has_X function
#define PLUGIN_FUNC(name) \
    static PyObject *pf_##name; \
    bool plugin_has_##name() { return (pf_##name != NULL); }
//==

PLUGIN_FUNC(setup)
PLUGIN_FUNC(score)

PLUGIN_FUNC(is_multi)
PLUGIN_FUNC(nr_of_mults)
PLUGIN_FUNC(get_multi)

#ifdef HAVE_PYTHON
static PyStructSequence_Desc qso_descr = {
    .name = "qso",
    .doc = "QSO data",
    .fields = (PyStructSequence_Field[]) {
	{.name = "band"},
	{.name = "call"},
	{.name = "mode"},
	{.name = "exchange"},
	// utc, ...
	{.name = NULL}  // guard
    },
    .n_in_sequence = 4
};

static PyStructSequence_Desc qrb_descr = {
    .name = "qrb",
    .doc = "QRB data",
    .fields = (PyStructSequence_Field[]) {
	{.name = "distance"},
	{.name = "bearing"},
	{.name = NULL}  // guard
    },
    .n_in_sequence = 2
};

static PyTypeObject *qso_type;
static PyTypeObject *qrb_type;

static PyObject *py_get_qrb_for_locator(PyObject *self, PyObject *args) {
    const char *locator;

    if (!PyArg_ParseTuple(args, "s", &locator))
	return NULL;

    double distance, bearing;
    bool ok = get_qrb_for_locator(locator, &distance, &bearing);

    if (!ok) {
	Py_RETURN_NONE;
    }

    //return Py_BuildValue("{s:d,s:d}", "distance", distance, "bearing", bearing);
    PyObject *py_qrb = PyStructSequence_New(qrb_type);
    PyStructSequence_SetItem(py_qrb, 0, Py_BuildValue("d", distance));
    PyStructSequence_SetItem(py_qrb, 1, Py_BuildValue("d", bearing));
    return py_qrb;
}

static PyMethodDef tlf_methods[] = {
    { "get_qrb_for_locator", py_get_qrb_for_locator, METH_VARARGS, "Get QRB for given locator" },
    { NULL, NULL, 0, NULL }
};

static struct PyModuleDef tlf_module = {
    PyModuleDef_HEAD_INIT,
    .m_name = "tlf",
    .m_size = -1,
    .m_methods = tlf_methods
};

PyMODINIT_FUNC PyModInit_tlf(void) {
    PyObject *tlf = PyModule_Create(&tlf_module);
    PyModule_AddIntMacro(tlf, CWMODE);
    PyModule_AddIntMacro(tlf, SSBMODE);
    PyModule_AddIntConstant(tlf, "BAND_ANY", BANDINDEX_ANY);
    PyModule_AddObject(tlf, "MY_LAT", PyFloat_FromDouble(my.Lat));
    PyModule_AddObject(tlf, "MY_LONG", PyFloat_FromDouble(my.Long));
    //...

    return tlf;
}
#endif

#ifdef HAVE_PYTHON
static void lookup_function(const char *name, PyObject **pf_ptr) {
    // pf_ptr is a borrowed reference
    *pf_ptr = PyDict_GetItemString(pDict, name);
    if (*pf_ptr != NULL && !PyCallable_Check(*pf_ptr)) {
	Py_DECREF(*pf_ptr);
	*pf_ptr = NULL;
    }
}
#endif

int plugin_init(const char *name) {
#ifdef HAVE_PYTHON
    // determine directory containing the plugin
    char *py_name = g_strdup_printf("rules/%s.py", name);
    char *path = find_available(py_name);
    g_free(py_name);
    if (*path == 0) {
	g_free(path);
	return PARSE_OK; // no plugin to be loaded
    }
    char *last_slash = strrchr(path, '/');
    if (last_slash == NULL) {
	g_free(path);
	// "Unable to load ...
	return PARSE_ERROR; // should not happen
    }
    *last_slash = 0;    // keep directory only

    char *set_path = g_strdup_printf(
			 "import sys\n"
			 "sys.path.insert(0, '%s')\n"
			 , path);
    g_free(path);

    // Initialize the Python Interpreter
    PyImport_AppendInittab("tlf", &PyModInit_tlf); // declare tlf module
    Py_Initialize();

    pTlf = PyImport_ImportModule("tlf");
    if (pTlf == NULL) {
	PyErr_Print();
	showmsg("Error: could not import module 'tlf'");
	return PARSE_ERROR;
    }

    PyRun_SimpleString(set_path);   // set module search path
    g_free(set_path);

    // Load the module object
    PyObject *pName = PyUnicode_FromString(name);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
	showmsg("Plugin not found");
	PyErr_Print(); //? show exception
	return PARSE_ERROR;
    }

    PyModule_AddObject(pModule, "tlf", pTlf);

    // pDict is a borrowed reference
    pDict = PyModule_GetDict(pModule);

    PyObject *pf_init;
    lookup_function("init", &pf_init);
    lookup_function("setup", &pf_setup);
    lookup_function("score", &pf_score);
    lookup_function("is_multi", &pf_is_multi);
    lookup_function("get_multi", &pf_get_multi);

    if (pf_setup == NULL) {
	showmsg("ERROR: missing setup() in plugin");
	return PARSE_ERROR;
    }

    // call init if available
    if (pf_init != NULL) {
	PyObject *pValue = PyObject_CallObject(pf_init, NULL);
	// ...check exception....
	Py_XDECREF(pValue);
    }


    // check add_qso ?

    // build interface types
    qso_type = PyStructSequence_NewType(&qso_descr);
    qrb_type = PyStructSequence_NewType(&qrb_descr);

#if 0
    plugin_setup();

    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  OK1AY          599  599  003                    0   3540.0");
    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  OK1Z           599  599  003                    0   3540.0");
    plugin_add_qso(" 80CW  04-Jan-21 16:30 0001  HA8QSY         599  599  003                    0   3540.0");

    int n = plugin_nr_of_mults(BANDINDEX_ANY);
    printf("n=%d\n", n);

    printf("S51Z multi: %d\n", plugin_is_multi(1, "S51Z", CWMODE));
    printf("9A1A multi: %d\n", plugin_is_multi(1, "9A1A", CWMODE));

    char *m = plugin_get_multi("YT1W", CWMODE);
    printf("multi: |%s|\n", m);
    g_free(m);

    exit(0);
#endif

    showmsg("Loaded plugin");

#endif
    return PARSE_OK;
}

void plugin_close() {
#ifdef HAVE_PYTHON
    // Clean up
    Py_DECREF(pModule);
    //FIXME check other pointers

    // Finish the Python Interpreter
    Py_Finalize();
#endif
}

void plugin_setup() {
#ifdef HAVE_PYTHON
    PyObject *pValue = PyObject_CallObject(pf_setup, NULL);
    //printf("after pf_setup, pValue %s NULL\n", pValue == NULL ? "is" : "not");
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	// FIXME: action?
    }
#endif
}

#ifdef HAVE_PYTHON
static PyObject *create_py_qso(struct qso_t *qso) {
    PyObject *py_qso = PyStructSequence_New(qso_type);
    PyStructSequence_SetItem(py_qso, 0, Py_BuildValue("i", qso->band));
    PyStructSequence_SetItem(py_qso, 1, Py_BuildValue("s", qso->call));
    PyStructSequence_SetItem(py_qso, 2, Py_BuildValue("i", qso->mode));
    PyStructSequence_SetItem(py_qso, 3, Py_BuildValue("s", qso->comment));
    return py_qso;
}
#endif

int plugin_score(struct qso_t *qso) {
    int result = 0;
#ifdef HAVE_PYTHON
    PyObject *py_qso = create_py_qso(qso);
    PyObject *args = Py_BuildValue("(O)", py_qso);
    PyObject *pValue = PyObject_CallObject(pf_score, args);
    Py_DECREF(args);
    Py_DECREF(py_qso);

    if (pValue != NULL) {
	result =  PyLong_AsLong(pValue);
    }
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	sleep(2);
	//exit(1);
    }
#endif
    return result;
}

bool plugin_is_multi(int band, const char *call, int mode) {
#ifdef HAVE_PYTHON
    // call is_multi
    struct qso_t qso;
    qso.band = band;
    qso.call = call;
    qso.mode = mode;
    qso.comment = "";

    PyObject *py_qso = create_py_qso(&qso);
    PyObject *args = Py_BuildValue("(O)", py_qso);
    PyObject *pValue = PyObject_CallObject(pf_is_multi, args);
    Py_DECREF(args);
    Py_DECREF(py_qso);

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
#else
    return false;
#endif
}

// result has to be g_freed()'s
char *plugin_get_multi(struct qso_t *qso) {
#ifdef HAVE_PYTHON
    PyObject *py_qso = create_py_qso(qso);
    PyObject *args = Py_BuildValue("(O)", py_qso);
    PyObject *pValue = PyObject_CallObject(pf_get_multi, args);
    Py_DECREF(args);
    Py_DECREF(py_qso);

    char *result = NULL;
    if (pValue != NULL) {
	result = g_strdup(PyUnicode_AsUTF8(pValue));
    }
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	sleep(2);
	//exit(1);
    }
    return result;
#else
    return NULL;
#endif
}

int plugin_nr_of_mults(int band) {
#ifdef HAVE_PYTHON
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
#else
    return 0;
#endif
}


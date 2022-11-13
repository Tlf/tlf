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

#include "parse_logcfg.h"   // for PARSE_OK/PARSE_ERROR

/*
TODO:
    - add mutex to plugin_* functions
    - add PLUGIN_CONFIG parameter
    - test plugin execution
*/

//  needs python3-dev

#ifdef HAVE_PYTHON
static PyObject *pModule, *pDict, *pTlf;
#endif

//== define pf_X pointer and plugin_has_X function
#define PLUGIN_FUNC(name) \
    static PyObject *pf_##name; \
    bool plugin_has_##name() { return (pf_##name != NULL); }
//==

PLUGIN_FUNC(init)
PLUGIN_FUNC(setup)
PLUGIN_FUNC(score)
PLUGIN_FUNC(check_exchange)

#ifdef HAVE_PYTHON
static PyStructSequence_Desc qso_descr = {
    .name = "Qso",
    .doc = "QSO data",
    .fields = (PyStructSequence_Field[]) {
	{.name = "band"},
	{.name = "call"},
	{.name = "mode"},
	{.name = "exchange"},
	{.name = "utc"},
	// ...
	{.name = NULL}  // guard
    },
    .n_in_sequence = 4
};

static PyStructSequence_Desc qrb_descr = {
    .name = "Qrb",
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

PyMODINIT_FUNC pyModInit_tlf(void) {
    PyObject *tlf = PyModule_Create(&tlf_module);
    PyModule_AddIntMacro(tlf, CWMODE);
    PyModule_AddIntMacro(tlf, SSBMODE);
    PyModule_AddIntMacro(tlf, DIGIMODE);
    PyModule_AddObject(tlf, "MY_LAT", PyFloat_FromDouble(my.Lat));
    PyModule_AddObject(tlf, "MY_LONG", PyFloat_FromDouble(my.Long));
    PyModule_AddType(tlf, qso_type);
    PyModule_AddType(tlf, qrb_type);

    return tlf;
}

static void lookup_function(const char *name, PyObject **pf_ptr) {
    // pf_ptr is a borrowed reference
    *pf_ptr = PyDict_GetItemString(pDict, name);
    if (*pf_ptr != NULL && !PyCallable_Check(*pf_ptr)) {
	Py_DECREF(*pf_ptr);
	*pf_ptr = NULL;
    }
}

//
// copy a string out of a Python dict:
//  *dest = dict[name]
//
// if *dest is NULL then it is allocated accordingly
// and must be freed by the caller
//
static void copy_dict_string(PyObject *dict, const char *name,
			     char **dest, int size) {
    // po is a borrowed reference
    PyObject *po = PyDict_GetItemString(dict, name);
    if (po == NULL) {
	return;     // no such entry
    }
    if (*dest == NULL) {
	*dest = g_malloc0(size);
    }
    g_strlcpy(*dest, PyUnicode_AsUTF8(po), size);
}

#endif

int parse_version(const char *version, int *major, int *minor) {
    char *v = g_strdup(version);
    g_strstrip(v);

    if (v[0] == 0) {    // version is empty
	g_free(v);
	*major = 0;
	*minor = 0;
	return PARSE_OK;
    }

    int result = PARSE_ERROR;   // default: failure

    // format: <digit(s)>.<digit(s)>[ignored_trailing_chars]
    GRegex *regex = g_regex_new("^(\\d+)\\.(\\d+)", 0, 0, NULL);
    GMatchInfo *match_info;
    g_regex_match(regex, v, 0, &match_info);
    if (g_match_info_matches(match_info)) {
	char *major_str = g_match_info_fetch(match_info, 1);
	char *minor_str = g_match_info_fetch(match_info, 2);
	*major = atoi(major_str);
	*minor = atoi(minor_str);
	result = PARSE_OK;  // success
	g_free(major_str);
	g_free(minor_str);
    }
    g_match_info_free(match_info);
    g_regex_unref(regex);
    g_free(v);

    return result;
}


#define ERROR_WRONG_VERSION1    10
#define ERROR_WRONG_VERSION2    20
// returns -1/0/+1, or one of the above error codes
int compare_versions(const char *version1, const char *version2) {
    int v1_major, v1_minor;
    int v2_major, v2_minor;

    if (parse_version(version1, &v1_major, &v1_minor) != PARSE_OK) {
	return ERROR_WRONG_VERSION1;
    }
    if (parse_version(version2, &v2_major, &v2_minor) != PARSE_OK) {
	return ERROR_WRONG_VERSION2;
    }

    if (v1_major > v2_major) {
	return 1;
    }
    if (v1_major < v2_major) {
	return -1;
    }
    if (v1_minor > v2_minor) {
	return 1;
    }
    if (v1_minor < v2_minor) {
	return -1;
    }
    return 0;
}

#ifdef HAVE_PYTHON
// call init if available
static int call_init() {
    if (pf_init == NULL) {
	return PARSE_OK;
    }

    PyObject *args = Py_BuildValue("(s)", "");  //FIXME configure argument
    PyObject *pValue = PyObject_CallObject(pf_init, args);
    Py_DECREF(args);

    if (NULL != PyErr_Occurred()) {
	showmsg("Error: ");
	PyErr_Print();
	return PARSE_ERROR;
    }

    if (pValue == NULL || pValue == Py_None)  {
	// no check needed
    } else if (PyUnicode_Check(pValue)) {
	const char *required_version = PyUnicode_AsUTF8(pValue);
	int rc = compare_versions(VERSION, required_version);
	if (rc == ERROR_WRONG_VERSION2) {
	    showstring("Error: Unparseable required version:", required_version);
	    return PARSE_ERROR;
	} else if (rc == ERROR_WRONG_VERSION1) {    // should not happen
	    showmsg("Error: Internal error parsing version: " VERSION);
	    return PARSE_ERROR;
	} else if (rc < 0) {
	    char *msg = g_strdup_printf("Plugin requires version %s but you are running "
					VERSION ". Please upgrade TLF.",
					required_version);
	    showmsg(msg);
	    g_free(msg);
	    return PARSE_ERROR;
	}
    } else {
	showmsg("Wrong return type for init()");
	return PARSE_ERROR;
    }

    Py_XDECREF(pValue);
    return PARSE_OK;
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
    PyImport_AppendInittab("tlf", &pyModInit_tlf); // declare tlf module
    Py_Initialize();

    // build interface types
    qso_type = PyStructSequence_NewType(&qso_descr);
    qrb_type = PyStructSequence_NewType(&qrb_descr);

    pTlf = PyImport_ImportModule("tlf");
    if (pTlf == NULL) {
	PyErr_Print();
	showmsg("Error: could not import module 'tlf'");
	return PARSE_ERROR;
    }

    PyRun_SimpleString(set_path);   // set module search path
    g_free(set_path);

    // Load the plugin module object
    PyObject *pName = PyUnicode_FromString(name);
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);
    if (pModule == NULL) {
	showmsg("Plugin not found");
	PyErr_Print(); //? show exception
	return PARSE_ERROR;
    }

    PyModule_AddObject(pModule, "tlf", pTlf);   // for 'import tlf'

    // pDict is a borrowed reference
    pDict = PyModule_GetDict(pModule);

    lookup_function("init", &pf_init);
    lookup_function("setup", &pf_setup);
    lookup_function("score", &pf_score);
    lookup_function("check_exchange", &pf_check_exchange);

    int rc = call_init();
    if (rc != PARSE_OK) {
	return rc;
    }

    showmsg("Loaded plugin");

#endif
    return PARSE_OK;
}

void plugin_close() {
#ifdef HAVE_PYTHON
    if (pModule != NULL) {
	// Finish the Python Interpreter
	Py_Finalize();
    }
#endif
}

void plugin_setup() {
#ifdef HAVE_PYTHON
    PyObject *pValue = PyObject_CallObject(pf_setup, NULL);
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	sleep(2);
	exit(1);
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
    PyStructSequence_SetItem(py_qso, 4, Py_BuildValue("i", qso->timestamp));
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
	result = PyLong_AsLong(pValue);
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

void plugin_check_exchange(struct qso_t *qso) {
#ifdef HAVE_PYTHON
    PyObject *py_qso = create_py_qso(qso);
    PyObject *args = Py_BuildValue("(O)", py_qso);
    PyObject *pValue = PyObject_CallObject(pf_check_exchange, args);
    Py_DECREF(args);
    Py_DECREF(py_qso);

    if (pValue != NULL && PyDict_Check(pValue)) {
	copy_dict_string(pValue, "mult1_value", &qso->mult1_value, MULT_SIZE);
	copy_dict_string(pValue, "normalized_exchange", &qso->normalized_comment,
			 COMMENT_SIZE);
    }
    Py_XDECREF(pValue);

    if (NULL != PyErr_Occurred()) {
	PyErr_Print();
	sleep(2);
	exit(1);
    }
#endif
}


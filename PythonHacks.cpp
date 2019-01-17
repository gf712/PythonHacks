#include "Python.h"
#include <unordered_map>

static std::unordered_map<PyFunctionObject*, PyObject*> repr_tracker;

static char module_docstring[] =
    "Module to try out different Python backend hacks";

static PyObject* changeFunctionName(PyObject *self, PyObject *args) {

	PyObject *type, *old_name, *new_name;

	if (!PyArg_ParseTuple(args, "OOO", &type, &old_name, &new_name)) {
		PyErr_SetString(PyExc_ValueError, "Unexpected number of args");
        goto FAIL;
    }

#if PY_MAJOR_VERSION >= 3
	if (!PyUnicode_Check(old_name) || !PyUnicode_Check(new_name))
#else
	if (!PyString_Check(old_name) || !PyString_Check(new_name))
#endif
		{
			PyErr_SetString(PyExc_TypeError, "'old_name' and 'new_name' need to be strings");
			goto FAIL;
		}
	if (PyType_Check(type)) {
		PyTypeObject *pytype = (PyTypeObject *)type;
		PyObject* obj = PyDict_GetItem(pytype->tp_dict, old_name);
		if (obj == NULL) {
			PyErr_SetString(PyExc_ValueError, "object definition does not exist in the given type!");
			goto FAIL;
		}
		PyDict_SetItem(pytype->tp_dict, new_name, obj);
		PyDict_DelItem(pytype->tp_dict, old_name);
	}

  	else if ( PyModule_Check(type)) {
	  	PyObject* module = PyModule_GetDict(type);
	 	PyObject* obj = PyDict_GetItem(module, old_name);
		if (obj == NULL) {
			PyErr_SetString(PyExc_ValueError, "object definition does not exist in the given module!");
			goto FAIL;
		}
		PyDict_SetItem(module, new_name, obj);
		PyDict_DelItem(module, old_name);
	}
	else {
		PyErr_SetString(PyExc_ValueError, "'type' is neither a module or a type");
		goto FAIL;
	}
	Py_RETURN_NONE;
	FAIL:
		return NULL;
}

// definition from CPython funcobject.c
static PyObject*
func_repr(PyFunctionObject *op)
{
#if PY_MAJOR_VERSION >= 3
    return PyUnicode_FromFormat("<function %s at %p>",
                               (char*)PyUnicode_DATA(op->func_name),
                               op);
#else
    return PyString_FromFormat("<function %s at %p>",
                           PyString_AsString(op->func_name),
                           op);
#endif
}

static PyObject* test_repr(PyFunctionObject* self) {
	if (repr_tracker.find(self) != repr_tracker.end()) {
		PyObject* result = PyObject_CallFunctionObjArgs(repr_tracker[self], self, NULL);
		if (!result) {
			return func_repr(self);
		}
#if PY_MAJOR_VERSION >= 3
		return PyUnicode_FromFormat("%s", 
			(char*)PyUnicode_DATA(result));
#else
		return PyString_FromFormat("%s", 
			PyString_AsString(result));
#endif
	}
	return func_repr(self);
}

static PyObject* changeFunctionReprToLambda(PyObject *self, PyObject *args) {

	PyObject *function, *new_repr, *test_result;
	PyTypeObject* function_type = NULL;


	if (!PyArg_ParseTuple(args, "OO", &function, &new_repr)) {
		PyErr_SetString(PyExc_ValueError, "Unexpected number of args");
        goto FAIL;
    }

	if (!PyFunction_Check(function)) {
		PyErr_SetString(PyExc_TypeError, "expected a function");
		goto FAIL;
	}

	if (!PyFunction_Check(new_repr)) {
		PyErr_SetString(PyExc_TypeError, "expected 'new_repr' to be a function");
		goto FAIL;
	}

	test_result = PyObject_CallFunctionObjArgs(new_repr, (PyFunctionObject*) function, NULL);
	if (!test_result){
		PyErr_SetString(PyExc_TypeError, "Wrong function signature. Expected something like lambda x: x");
		goto FAIL;		
	}

#if PY_MAJOR_VERSION >= 3
	if (!PyUnicode_Check(test_result))
#else
	if (!PyString_Check(test_result))
#endif
	{
		PyErr_SetString(PyExc_TypeError, "'new_repr' is expected to return a Python string");
		goto FAIL;
	}

	if (repr_tracker.find((PyFunctionObject*) function) != repr_tracker.end())
		Py_XDECREF(repr_tracker[(PyFunctionObject*) function]);

	function_type = Py_TYPE(function_type);

	Py_INCREF(new_repr);

	repr_tracker[(PyFunctionObject*) function] = new_repr;

	Py_TYPE(function)->tp_repr = (reprfunc)test_repr;

    Py_RETURN_NONE;

    FAIL:
    	return NULL;
}


static PyMethodDef module_methods[] = {
    {"change_function_name", changeFunctionName, METH_VARARGS, "change function name"},
    {"change_function_repr", changeFunctionReprToLambda, METH_VARARGS, "replace __repr__ slot with a lambda"},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef cModPyDef =
{
    PyModuleDef_HEAD_INIT,
    "python_hacks", /* name of module */
    module_docstring,          /* module documentation, may be NULL */
    -1,          /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
    module_methods
};
#define INITERROR return NULL
PyMODINIT_FUNC PyInit_python_hacks(void)
#else
#define INITERROR return
void initpython_hacks(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
    PyObject *m = PyModule_Create(&cModPyDef);
#else
    PyObject *m = Py_InitModule3("python_hacks", module_methods, module_docstring);
#endif
    if (!m)
    	INITERROR;
#if PY_MAJOR_VERSION >= 3
    return m;
#endif
}
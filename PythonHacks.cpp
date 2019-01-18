#include "Python.h"
#include <unordered_map>

static std::unordered_map<PyFunctionObject*, PyObject*> repr_tracker;

static char module_docstring[] =
		"Module to try out different Python backend hacks";

static PyObject* changeFunctionName(PyObject *self, PyObject *args) {

	PyObject *type = NULL, 
			 *old_name = NULL,
			 *new_name = NULL,
			 *dict = NULL,
			 *func_obj = NULL;

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
		dict = pytype->tp_dict;
		func_obj = PyDict_GetItem(dict, old_name);
		if (func_obj == NULL) {
			PyErr_SetString(PyExc_ValueError, "object definition does not exist in the given type!");
			goto FAIL;
		}
	}

	else if ( PyModule_Check(type)) {
		dict = PyModule_GetDict(type);
		func_obj = PyDict_GetItem(dict, old_name);
		if (func_obj == NULL) {
			PyErr_SetString(PyExc_ValueError, "object definition does not exist in the given module!");
			goto FAIL;
		}
	}
	else {
		PyErr_SetString(PyExc_ValueError, "'type' is neither a module or a type");
		goto FAIL;
	}

	if (PyDict_GetItem(dict, new_name))
	{
		PyErr_SetString(PyExc_ValueError, "new_name already exists");
		goto FAIL;
	}
	PyDict_SetItem(dict, new_name, func_obj);
	PyDict_DelItem(dict, old_name);
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
		(char*)PyUnicode_DATA(op->func_name), op);
#else
	return PyString_FromFormat("<function %s at %p>",
		PyString_AsString(op->func_name), op);
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

	PyObject *function = NULL, 
			 *new_repr = NULL, 
			 *test_result = NULL;
	PyTypeObject* function_type = NULL;
	PyFunctionObject* function_obj = NULL;

	if (!PyArg_ParseTuple(args, "OO", &function, &new_repr)) {
		PyErr_SetString(PyExc_ValueError, "Unexpected number of args");
		goto FAIL;
	}

	if (!PyFunction_Check(function)) {
		PyErr_SetString(PyExc_TypeError, "expected a function");
		goto FAIL;
	}

	function_obj = (PyFunctionObject*) function;
	
	if (new_repr == Py_None) {
		if (repr_tracker.find(function_obj) != repr_tracker.end()) {
			Py_DECREF(repr_tracker[function_obj]);
			repr_tracker.erase(function_obj);
		}
		Py_TYPE(function)->tp_repr = (reprfunc)func_repr;
		Py_RETURN_NONE;
	}

	if (!PyFunction_Check(new_repr)) {
		PyErr_SetString(PyExc_TypeError, "expected 'new_repr' to be a function");
		goto FAIL;
	}

	test_result = PyObject_CallFunctionObjArgs(new_repr, function_obj, NULL);
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

	if (repr_tracker.find(function_obj) != repr_tracker.end())
		Py_XDECREF(repr_tracker[(PyFunctionObject*) function]);

	function_type = Py_TYPE(function_type);

	Py_INCREF(new_repr);

	repr_tracker[function_obj] = new_repr;

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
PyMODINIT_FUNC initpython_hacks(void)
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
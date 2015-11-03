#include <Python.h>

// g++ -I/usr/include/python2.7 -Wall callpython.cpp -lpython2.7
int main(int argc, char *argv[])
{
    Py_Initialize();
    if (!Py_IsInitialized()) return 1;

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");


    PyObject* pModule = PyImport_ImportModule("id_verify");
    if (!pModule) {
        printf("can not import module id_verify\n");
        return 1;
    }

    PyObject* pDict = PyModule_GetDict(pModule);
    if (!pDict) {
        printf("can not get module dict\n");
        return 1;
    }
    PyObject* func = PyDict_GetItemString(pDict, "id_verify");
    PyObject* rest =  PyObject_CallFunction(func, "s", (char *)argv[1]);
    Py_DECREF(func);

    if (rest == NULL) {
        printf("can not get result\n");
        return 1;
    }
    if (PyBool_Check(rest) && rest == Py_False) {
        printf(" Failure\n");
        return 1;
    }
    Py_DECREF(rest);
    printf(" OK\n");

    return 0;
}

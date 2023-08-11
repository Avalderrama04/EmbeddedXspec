
#include <python3.9/Python.h>
#include <numpy/arrayobject.h>
#include <vector>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <string>
#include "xsTypes.h"

using namespace std;

class WrapperCalc {
public:
    WrapperCalc() {
        Py_Initialize();
        initPython();
        PyRun_SimpleString("import scipy.stats");
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append(\"/scratch1/avalderr/pyspectrum\")"); //replace with path to your directory 
        pyModule = PyImport_ImportModule("pyapec_script"); //replace with name of python script, ignoring extension
        if (!pyModule) {
            PyErr_Print();
            std::cerr << "Failed to import python script module\n";
            return;
        }
    }

    // RealArray is defined as an std::valarray<Real>, while Real is defined as a double in xsTypes.h
    void caller(const RealArray& energy, const RealArray& params, RealArray& flux) {
      if (energy.size() == 0 || params.size() == 0) {
        std::cerr << "Error: Input arrays must be not be empty." << std::endl;
        return;
    }
      

      PyObject* pyFunc = PyObject_GetAttrString(pyModule, "pyapec"); //follows name of the function listed in python script, may change
        if (!pyFunc || !PyCallable_Check(pyFunc)) {
            Py_XDECREF(pyFunc);
            Py_DECREF(pyModule);
            std::cerr << "python function not found or not callable\n";
            return;
        }

        // Create new Python lists with the same size as the RealArray vectors
        PyObject* pyEngs = PyList_New(energy.size());
        PyObject* pyParams = PyList_New(params.size());
        PyObject* pyFlux = PyList_New(flux.size());

        // Loop through each list and set each element with the corresponding value from its vector
        for (size_t i = 0; i < energy.size(); ++i) {
            PyList_SetItem(pyEngs, i, PyFloat_FromDouble(energy[i]));
        }

        for (size_t i = 0; i < params.size(); ++i) {
            PyList_SetItem(pyParams, i, PyFloat_FromDouble(params[i]));
        }

        for (size_t i = 0; i < flux.size(); ++i) {
            PyList_SetItem(pyFlux, i, PyFloat_FromDouble(flux[i]));
        }

        // Create a new tuple object to pass the arguments to pyFunc
        PyObject* pyArgs = PyTuple_New(3);
        PyTuple_SetItem(pyArgs, 0, pyEngs);
        PyTuple_SetItem(pyArgs, 1, pyParams);
        PyTuple_SetItem(pyArgs, 2, pyFlux);

        // Call pyFunc in pyapec_script with the arguments contained in the python tuple pyArgs, return the result in pyResult
        PyObject* pyResult = PyObject_CallObject(pyFunc, pyArgs);
        if (!pyResult) {
            PyErr_Print();  // Print Python error information
            Py_XDECREF(pyFunc);
            Py_DECREF(pyModule);
            Py_DECREF(pyArgs);
            std::cerr << "Error occurred during python function call\n";
            return;
        }

        // Extract the results from the pyFlux list (output of pyapec function) and store it in the C++ "flux" array
        for (size_t i = 0; i < flux.size(); ++i) {
            flux[i] = PyFloat_AsDouble(PyList_GetItem(pyFlux, i));
        }

        Py_DECREF(pyResult);
        Py_DECREF(pyFunc);
        Py_DECREF(pyModule);
        Py_DECREF(pyArgs);
	// Py_Finalize();
    }

    ~WrapperCalc() { }

private:
    bool initPython() {
        import_array();
        if (PyErr_Occurred()) {
            PyErr_Print();
            PyErr_SetString(PyExc_ImportError, "Failed to import numpy.core.multiarray");
            return false;
        }
        return true;
    }

    PyObject* pyModule = nullptr;
};

extern "C" void pyapecInfo(const RealArray& energyArray, const RealArray& params, int spectrumNumber, RealArray& fluxArray, RealArray& fluxErrArray, const string& initString) {
    size_t N(energyArray.size());
    fluxArray.resize(N-1);
    fluxErrArray.resize(0);
    WrapperCalc calc;
    calc.caller(energyArray, params, fluxArray);
}


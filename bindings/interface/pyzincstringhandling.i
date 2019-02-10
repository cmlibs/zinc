/**
 * pyzincstringhandling.i
 *
 */
/*
 * OpenCMISS-Zinc Library
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


%typemap(newfree) char * {
free($1);
}

%newobject *::getName(); 

%newobject *::getComponentName(int componentNumber);

%newobject *::evaluateString;

%newobject *::getProperty(const char* property);

%newobject *::getLabelText(int labelNumber);

%newobject *::getSolutionReport();

%typemap(in) (const void *buffer, unsigned int buffer_length)
{
    if (PyBytes_Check($input))
    {
        $1 = PyBytes_AsString($input);
        $2 = PyBytes_Size($input);
    }
    else if (PyString_Check($input))
    {
        $1 = PyString_AsString($input);
        $2 = PyString_Size($input);
    }
    else if (PyUnicode_Check($input))
    {
%#if PY_VERSION_HEX < 0x03030000
        PyObject *utf8_obj = PyUnicode_AsUTF8String($input);
        $1 = PyString_AsString(utf8_obj);
        $2 = PyString_Size(utf8_obj);
%#else
        Py_ssize_t len = 0;
        $1 = PyUnicode_AsUTF8AndSize($input, &len);
        $2 = (int)len;
%#endif
    }
    else
    {
        PyErr_SetString(PyExc_TypeError,"List may only contain string");
        return NULL;
    }
}

%typemap(in, numinputs=0) (const void **buffer_out, unsigned int *buffer_length_out)
{
	char *memoryBuffer = 0;
	$1 = (void **)&memoryBuffer;
	unsigned int size = 0;
	$2 = &size;
};

%typemap(argout)(const void **buffer_out, unsigned int *buffer_length_out)
{
	const char *mystring = (char *)(*$1);
	PyObject *o = Py_None;
	if (mystring)
	{
	   o = PyBytes_FromString(mystring);
	   if (o == 0)
	       o = PyString_FromString(mystring);
	}
	if ((!$result) || ($result == Py_None))
	{
		$result = o;
	}
	else
	{
		// note: code considers that tuples are supposed to be immutable
		// should only modify if you have only reference to them!
		if (!PyTuple_Check($result))
		{
			PyObject *previousResult = $result;
			$result = PyTuple_New(2);
			PyTuple_SET_ITEM($result, 0, previousResult); // steals reference
			PyTuple_SET_ITEM($result, 1, o); // steals reference
		}
		else
		{
			PyObject *previousResult = $result;
			PyObject *addResult = PyTuple_New(1);
			PyTuple_SET_ITEM(addResult, 0, o); // steals reference
			$result = PySequence_Concat(previousResult, addResult);
			Py_DECREF(previousResult);
			Py_DECREF(addResult);
		}
	}
};

%typemap(in) (int valuesCount, const char **valuesIn)
{
	/* Applying in array of variable size typemap */
	if (PyString_Check($input))
	{
		$1 = 1;
		$2 = new char*[1];
		PyObject *o = $input;
		
		char *temp_string = PyString_AsString(o);
		int length = strlen(temp_string);
		$2[0] = new char[length + 1];
		strcpy($2[0], temp_string);
	}
	else if (PyUnicode_Check($input))
	{
		$1 = 1;
		$2 = new char*[1];
		PyObject *o = $input;
%#if PY_VERSION_HEX < 0x03030000
		PyObject *utf8_obj = PyUnicode_AsUTF8String(o);
		char *temp_string = PyString_AsString(utf8_obj);
		Py_ssize_t length = PyString_Size(utf8_obj);
%#else
		Py_ssize_t length;
		const char* temp_string = PyUnicode_AsUTF8AndSize(o, &length);
%#endif
		if (temp_string != NULL)
		{
			$2[0] = new char[length + 1];
			strcpy($2[0], temp_string);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError,"Not a UTF8 compatible string");
			delete[] $2;
			$2 = 0;
			return NULL;
		}
	}
	else if (PyList_Check($input))
	{
		$1 = PyList_Size($input);
		$2 = new char*[$1];
		for (int i = 0; i < $1; i++)
		{
			PyObject *o = PyList_GetItem($input,i);
			if (PyString_Check(o))
			{
				char *temp_string = PyString_AsString(o);
				int length = strlen(temp_string);
				$2[i] = new char[length + 1];
				strcpy($2[i], temp_string);
			}
			else if (PyUnicode_Check(o))
			{
%#if PY_VERSION_HEX < 0x03030000
				PyObject *utf8_obj = PyUnicode_AsUTF8String(o);
				char *temp_string = PyString_AsString(utf8_obj);
				Py_ssize_t length = PyString_Size(utf8_obj);
%#else
				Py_ssize_t length;
				const char* temp_string = PyUnicode_AsUTF8AndSize(o, &length);
%#endif
				if (temp_string != NULL)
				{
					$2[i] = new char[length + 1];
					strcpy($2[i], temp_string);
				}
				else
				{
					PyErr_SetString(PyExc_ValueError,"Not a UTF8 compatible string");
					delete[] $2;
					$2 = 0;
					return NULL;
				}
			}
			else
			{
				PyErr_SetString(PyExc_TypeError,"List may only contain string");
				delete[] $2;
				$2 = 0;
				return NULL;
			}
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,"Not a list, nor a single string value");
		$2 = 0;
		return NULL;
	}
};

%typemap(freearg) (int valuesCount, const char**valuesIn)
{
	if ($2)
	{
		for (int i = 0; i < $1; i++)
		{
			delete[] $2[i];
		}
		delete[] $2;
	}
};

%typemap(in) (const char *name)
{
	if (PyString_Check($input))
	{
		PyObject *o = $input;
		
		char *temp_string = PyString_AsString(o);
		int length = strlen(temp_string);
		$1 = new char[length + 1];
		strcpy($1, temp_string);
	}
	else if (PyUnicode_Check($input))
	{
		PyObject *o = $input;
%#if PY_VERSION_HEX < 0x03030000
		PyObject *utf8_obj = PyUnicode_AsUTF8String(o);
		char *temp_string = PyString_AsString(utf8_obj);
		Py_ssize_t length = PyString_Size(utf8_obj);
%#else
		Py_ssize_t length;
		const char* temp_string = PyUnicode_AsUTF8AndSize(o, &length);
%#endif
		if (temp_string != NULL)
		{
			$1 = new char[length + 1];
			strcpy($1, temp_string);
		}
		else
		{
			PyErr_SetString(PyExc_ValueError,"Not a UTF8 compatible string");
			$1 = 0;
			return NULL;
		}
	}
	else
	{
		PyErr_SetString(PyExc_TypeError,"Not a single string value");
		$1 = 0;
		return NULL;
	}
};

%typemap(freearg) (const char *name)
{
	if ($1)
	{
		delete[] $1;
	}
};

%typemap(in) (const char *description) = (const char *name);
%typemap(in) (int numberOfNames, const char **fieldNames) = (int valuesCount, const char**valuesIn);
%typemap(freearg) (int numberOfNames, const char **fieldNames) = (int valuesCount, const char**valuesIn);

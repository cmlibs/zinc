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

/* the following line handle binary data */
%apply (char *STRING, size_t LENGTH) { (const void *buffer, unsigned int buffer_length) }

%typemap(in, numinputs=0) (void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	char *memoryBuffer = 0;
	$1 = (void **)&memoryBuffer;
	unsigned int size = 0;
	$2 = &size;
};

%typemap(argout)(void **memory_buffer_references, unsigned int *memory_buffer_sizes)
{
	const char *mystring = (char *)(*$1);
	PyObject *o = PyString_FromString(mystring);
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

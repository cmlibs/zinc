Zinc API
========

Introduction
------------

The `CMLibs website <https://cmlibs.org>`_ provides some reference material for the Zinc API and its usage:

* `Zinc data model and API usage <https://cmlibs.org/documentation/zinc/index.html>`_;
* `Zinc C++ API class documentation <https://cmlibs.org/documentation/apidoc/zinc/latest/classes.html>`_;
* `Basic tutorials <https://cmlibs.org/documentation/tutorials/index.html>`_.

Python
------

Apart from syntax differences (no semicolons, imports vs. #include) using the API from Python is very similar to the above C++ interface, with these main differences:

1.  Python automatically cleans up strings returned by API methods e.g. getName().

2.  Python arrays/lists know their size so they are passed in on their own, whereas variable-sized arrays in the C++ API are preceded by the array size argument:

* C++::

   const double xi[3] = { 0.5, 0.5, 0.5 };
   fieldcache.setMeshLocation(element, 3, xi);

* Python::

   xi = [0.5, 0.5, 0.5]
   fieldcache.setMeshLocation(element, xi)

3. C++ methods which return arrays do so by filling 'out' array arguments, and variable sized arrays are preceded by the known size of the client array to fill. Python does not permit arguments to be modified, so these are appended to the return value, however the array size to return must be specified if it is present in the C++ API. Example evaluation of a 3-component field:

* C++::

   double outValues[3];
   int result = field.evaluateReal(fieldcache, 3, outValues);

* Python::

   result, outValues = field.evaluateReal(fieldcache, 3)

4. Python uses imports to load symbols, and enumerations are separated by ``.`` from the class name. This example also shows that fixed-size array arguments (in and out) do not have a size argument in either language:

* C++::

   #include <cmlibs/zinc/material.hpp>

   const double orange[3] = { 1.0, 0.5, 0.0 };
   material.setAttributeReal3(Material::ATTRIBUTE_DIFFUSE, orange);

* Python::

   from cmlibs.zinc.material import Material

   orange = [ 1.0, 0.5, 0.0 ]
   material.setAttributeReal3(Material.ATTRIBUTE_DIFFUSE, orange)

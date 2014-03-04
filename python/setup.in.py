""" Zinc Library: An advanced visualisation library for FE models

The Zinc library is an advanced visualisation library for FE models.  The
Zinc library understands a representation of mathematical fields, including
finite element, image-based and CAD.  It also understands fields derived by
mathematical operators.
"""

classifiers = """\
Development Status :: 3 - Alpha
Intended Audience :: Developers
Intended Audience :: Education
Programming Language :: Python
Operating System :: Microsoft :: Windows
Operating System :: Unix
"""

from distutils.core import setup

doclines = __doc__.split("\n")

setup(
	name='@PYPI_PACKAGE_NAME@',
	version='@PYPI_SOURCE_TARGZ_VERSION@',
	author='H. Sorby',
	author_email='h.sorby@auckland.ac.nz',
	packages=['opencmiss', 'opencmiss.zinc'],
	package_data={'opencmiss.zinc': ['@ZINC_SHARED_OBJECT_GLOB@', '@ZINC_LIBRARY_NAME@']},
	platforms=['any'],
	url='http://pypi.python.org/pypi/PyZinc/',
	license='LICENSE.txt',
	description=doclines[0],
	classifiers = filter(None, classifiers.split("\n")),
	long_description=open('README.txt').read(),
)


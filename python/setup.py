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
    name='PyZinc',
    version='3.0.0',
    author='H. Sorby',
    author_email='h.sorby@auckland.ac.nz',
    packages=['zinc'],
    package_dir={'zinc': 'zinc'},
    package_data={'zinc': ['*.so']},
    platforms=['any'],
    url='http://pypi.python.org/pypi/PyZinc/',
    license='LICENSE.txt',
    description=doclines[0],
    classifiers = filter(None, classifiers.split("\n")),
    long_description=open('README.txt').read(),
)


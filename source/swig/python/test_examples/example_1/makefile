# **************************************************************************
# FILE : makefile
#
# LAST MODIFIED : 8 December 2008
#
# DESCRIPTION :
#
# makefile for wrapping the entire example_functions for 
# python using swig 
# ==========================================================================

make: 
	swig -python -c++ example_functions.i
	g++ -c -Wall example_functions_wrap.cxx example_functions.cc -I/usr/include/python2.5
	g++ -shared example_functions_wrap.o example_functions.o -o _example_functions.so

clean:
	rm -f example_functions.o example_functions.py example_functions.pyc _example_functions.so example_functions_wrap.cxx example_functions_wrap.o


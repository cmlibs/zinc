# **************************************************************************
# FILE : makefile
#
# LAST MODIFIED : 8 December 2008
#
# DESCRIPTION :
#
# makefile for wrapping the arithmetics function in example_functions for 
# python using swig 
# ==========================================================================

make: 
	swig -python -c++ example_arithmetics.i
	g++ -c -Wall example_arithmetics_wrap.cxx example_functions.cc -I/usr/include/python2.5
	g++ -shared example_arithmetics_wrap.o example_functions.o -o _example_arithmetics.so

clean:
	rm -f example_functions.o example_arithmetics.py example_arithmetics.pyc _example_arithmetics.so example_arithmetics_wrap.cxx example_arithmetics_wrap.o

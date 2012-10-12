#**************************************************************************
# FILE : makefile
#
# LAST MODIFIED : 10 December 2008
#
# DESCRIPTION :
#
# makefile for wrapping the functions in the example file for 
# octave using swig.
# =========================================================================

make: 
	swig -octave -o factorial_wrap.cpp factorial_example.i
	mkoctfile factorial_example.c factorial_wrap.cpp 
	
clean:
	rm -f factorial_example.oct factorial_wrap.o factorial_wrap.cpp factorial_example.o


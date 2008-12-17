#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 10 December 2008

#DESCRIPTION :

#makefile for wrapping the execute_command file functions for 
#octave using swig.
#==========================================================================

make:
	swig -octave -o execute_command_wrap.cpp execute_command.i
	mkoctfile -I$(CMISS_ROOT)/cmgui/source -c execute_command_wrap.cpp 
	mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui execute_command_wrap.o -o execute_command.oct

clean:
	rm -f execute_command.oct execute_command_wrap.o execute_command_wrap.cpp


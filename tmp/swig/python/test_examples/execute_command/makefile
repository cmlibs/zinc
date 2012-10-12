#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 11 December 2008

#DESCRIPTION :

#makefile for wrapping the execute_command file functions for 
#python using swig.
#==========================================================================

make: 
	swig -python execute_command.i 
	gcc -c -Wall execute_command_wrap.c -I/usr/include/python2.5
	g++ -shared execute_command_wrap.o $(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/libcmgui.so -o _execute_command.so

clean:
	rm -f execute_command.py execute_command.pyc _execute_command.so execute_command_wrap.c execute_command_wrap.o
	

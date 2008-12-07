# example: wrap the cmiss_command_data_execute_command function
# 	   in cmiss_command_data

make: 
	swig -python execute_command.i
	g++ -c -Wall execute_command_wrap.c -I/usr/include/python2.5
	g++ -shared execute_command_wrap.o cmiss_command_data.o -o _execute_command.so


clean:
	rm -f execute_command.py execute_command.pyc _execute_command.so execute_command_wrap.c execute_command_wrap.o


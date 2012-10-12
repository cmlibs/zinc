#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 22 December 2008

#DESCRIPTION :

#makefile for cmgui python modules using swig
#==========================================================================

make: 
	swig -python cmiss_command_data.i
	swig -python cmiss_region.i
	swig -python cmiss_field.i
	#swig -python cmiss_node.i
	#swig -python cmiss_element.i 

	gcc -c -Wall cmiss_command_data_wrap.c -I/usr/include/python2.5 -I$(CMISS_ROOT)/cmgui/source
	gcc -c -Wall cmiss_region_wrap.c -I/usr/include/python2.5 
	gcc -c -Wall cmiss_field_wrap.c -I/usr/include/python2.5 
	#gcc -c -Wall cmiss_node_wrap.c -I/usr/include/python2.5 
	#gcc -c -Wall cmiss_element_wrap.c -I/usr/include/python2.5 

	g++ -shared cmiss_command_data_wrap.o -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui -o _cmiss_command_data.so
	g++ -shared cmiss_region_wrap.o -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui -o _cmiss_region.so
	g++ -shared cmiss_field_wrap.o -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui -o _cmiss_field.so
	#g++ -shared cmiss_node_wrap.o -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui -o _cmiss_node.so
	#g++ -shared cmiss_elemnt_wrap.o -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui -o _cmiss_element.so

clean:
	rm -f cmiss_command_data.py cmiss_command_data.pyc _cmiss_command_data.so cmiss_command_data_wrap.c cmiss_command_data_wrap.o
	rm -f cmiss_region.py cmiss_region.pyc _cmiss_region.so cmiss_region_wrap.c cmiss_region_wrap.o
	rm -f cmiss_field.py cmiss_field.pyc _cmiss_field.so cmiss_field_wrap.c cmiss_field_wrap.o
	rm -f cmiss_node.py cmiss_node.pyc _cmiss_node.so cmiss_node_wrap.c cmiss_node_wrap.o
	rm -f cmiss_element.py cmiss_element.pyc _cmiss_element.so cmiss_element_wrap.c cmiss_element_wrap.o
	

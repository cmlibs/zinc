#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 22 December 2008

#DESCRIPTION :

#makefile for cmgui python modules using swig
#==========================================================================


INCLUDES = -I/usr/include/python2.5 -I$(CMISS_ROOT)/cmgui/source

#Required for building shared libraries on x86_64
CFLAGS = -fPIC

LIBS = -L$(CMISS_ROOT)/cmgui/lib/x86_64-linux/cmgui-wx-debug/ -lcmgui

make: 
	swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_command_data.i
	swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_region.i
	swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_field.i
	#swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_node.i
	swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_element.i 
	swig -python -I$(CMISS_ROOT)/cmgui/source cmiss_texture.i 

	gcc -c -Wall cmiss_command_data_wrap.c $(CFLAGS) $(INCLUDES)
	gcc -c -Wall cmiss_region_wrap.c $(CFLAGS) $(INCLUDES)
	gcc -c -Wall cmiss_field_wrap.c $(CFLAGS) $(INCLUDES)
	#gcc -c -Wall cmiss_node_wrap.c $(CFLAGS) $(INCLUDES)
	gcc -c -Wall cmiss_element_wrap.c $(CFLAGS) $(INCLUDES)
	gcc -c -Wall cmiss_texture_wrap.c $(CFLAGS) $(INCLUDES)

	g++ -shared cmiss_command_data_wrap.o $(LIBS) -o _command_data.so
	g++ -shared cmiss_region_wrap.o $(LIBS) -o _region.so
	g++ -shared cmiss_field_wrap.o $(LIBS) -o _field.so
	#g++ -shared cmiss_node_wrap.o $(LIBS) -o _node.so
	g++ -shared cmiss_element_wrap.o $(LIBS) -o _element.so
	g++ -shared cmiss_texture_wrap.o $(LIBS) -o _texture.so

clean:
	rm -f command_data.py command_data.pyc _command_data.so cmiss_command_data_wrap.c cmiss_command_data_wrap.o
	rm -f region.py region.pyc _region.so cmiss_region_wrap.c cmiss_region_wrap.o
	rm -f field.py field.pyc _field.so cmiss_field_wrap.c cmiss_field_wrap.o
	rm -f node.py node.pyc _node.so cmiss_node_wrap.c cmiss_node_wrap.o
	rm -f element.py element.pyc _element.so cmiss_element_wrap.c cmiss_element_wrap.o
	rm -f texture.py texture.pyc _texture.so cmiss_texture_wrap.c cmiss_texture_wrap.o


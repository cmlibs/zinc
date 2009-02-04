#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 04 February 2009

#DESCRIPTION :

#makefile for cmgui API wrappers using swig
#==========================================================================

# Compilers
CC = gcc
CXX = g++

# Flags
CFLAGS = -c -Wall -fPIC #Required for building shared libraries on x86_64
CXXFLAGS = -shared 

# Includes
SWIG_INCLUDES = -I$(CMISS_ROOT)/cmgui/source

CMISS_LIBS = -L$(CMISS_ROOT)/cmgui/lib/x86_64-linux/cmgui-wx-debug/ -lcmgui
#CMISS_LIBS = -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui

PYTHON_INCLUDES = -I/usr/include/python2.5 -I$(CMISS_ROOT)/cmgui/source

OCTAVE_INCLUDES = -I$(CMISS_ROOT)/cmgui/source

# Dependencies
INTERFACES = cmiss_command_data.i cmiss_region.i cmiss_field.i cmiss_element.i cmiss_texture.i cmiss_scene_viewer.i #cmiss_node.i
PYTHON_WRAPS = $(INTERFACES:.i=_wrap.c)
PYTHON_OBJECTS = $(PYTHON_WRAPS:.c=.o)
PYTHON_MODULES = $(subst cmiss,,$(INTERFACES:.i=.so))

Cmisspython: $(PYTHON_MODULES)

$(PYTHON_MODULES): $(PYTHON_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CMISS_LIBS) cmiss$(@:.so=_wrap.o) -o $@

$(PYTHON_OBJECTS): $(PYTHON_WRAPS)
	$(CC) $(CFLAGS) $(PYTHON_INCLUDES) $(@:.o=.c)	
	
$(PYTHON_WRAPS): $(INTERFACES)
	swig -python $(SWIG_INCLUDES) $(@:_wrap.c=.i)

Cmissoctave: 
	swig -octave $(SWIG_INCLUDES) -o cmiss_command_data_wrapoct.cpp cmiss_command_data.i
	mkoctfile $(OCTAVE_INCLUDES) -c cmiss_command_data_wrapoct.cpp 
	mkoctfile $(CMISS_LIBS) cmiss_command_data_wrapoct.o -o cmiss_command_data.oct
	
	swig -octave $(SWIG_INCLUDES) -o cmiss_region_wrapoct.cpp cmiss_region.i
	mkoctfile $(OCTAVE_INCLUDES) -c cmiss_region_wrapoct.cpp 
	mkoctfile $(CMISS_LIBS) cmiss_region_wrapoct.o -o cmiss_region.oct

	swig -octave $(SWIG_INCLUDES) -o cmiss_field_wrapoct.cpp cmiss_field.i
	mkoctfile $(OCTAVE_INCLUDES) -c cmiss_field_wrapoct.cpp 
	mkoctfile $(CMISS_LIBS) cmiss_field_wrapoct.o -o cmiss_field.oct
	
	#swig -octave $(SWIG_INCLUDES) -o cmiss_node_wrapoct.cpp cmiss_node.i
	#mkoctfile $(OCTAVE_INCLUDES) -c cmiss_node_wrapoct.cpp 
	#mkoctfile $(CMISS_LIBS) cmiss_node_wrapoct.o -o cmiss_node.oct

	swig -octave $(SWIG_INCLUDES) -o cmiss_element_wrapoct.cpp cmiss_element.i
	mkoctfile $(OCTAVE_INCLUDES) -c cmiss_element_wrapoct.cpp 
	mkoctfile $(CMISS_LIBS) cmiss_element_wrapoct.o -o cmiss_element.oct

clean:
	rm -f $(PYTHON_WRAPS) $(PYTHON_OBJECTS) $(PYTHON_MODULES) $(subst cmiss_,,$(INTERFACES:.i=.py)) $(subst cmiss_,,$(INTERFACES:.i=.pyc))


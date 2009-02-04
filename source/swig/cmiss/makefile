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
CXX_LIBS = -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui
#CXX_LIBS = -L$(CMISS_ROOT)/cmgui/lib/x86_64-linux/cmgui-wx-debug/ -lcmgui
CC_INCLUDES = -I/usr/include/python2.5 -I$(CMISS_ROOT)/cmgui/source

# Dependencies
INTERFACES = cmiss_command_data.i cmiss_region.i cmiss_field.i cmiss_element.i cmiss_texture.i cmiss_scene_viewer.i #cmiss_node.i
PYTHON_WRAPS = $(INTERFACES:.i=_wrap.c)
PYTHON_OBJECTS = $(PYTHON_WRAPS:.c=.o)
PYTHON_MODULES = $(subst cmiss,,$(INTERFACES:.i=.so))

Cmisspython: $(PYTHON_MODULES)

$(PYTHON_MODULES): $(PYTHON_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CXX_LIBS) cmiss$(@:.so=_wrap.o) -o $@

$(PYTHON_OBJECTS): $(PYTHON_WRAPS)
	$(CC) $(CFLAGS) $(CC_INCLUDES) $(@:.o=.c)	
	
$(PYTHON_WRAPS): $(INTERFACES)
	swig -python $(SWIG_INCLUDES) $(@:_wrap.c=.i)


clean:
	rm -f $(PYTHON_WRAPS) $(PYTHON_OBJECTS) $(PYTHON_MODULES) $(subst cmiss_,,$(INTERFACES:.i=.py)) $(subst cmiss_,,$(INTERFACES:.i=.pyc))


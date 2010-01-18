#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 14 January 2010

#DESCRIPTION :

#makefile for cmgui API wrappers in Python using swig
#==========================================================================

# Compilers
CC = gcc
CXX = g++

# Flags
CFLAGS = -c -Wall -fPIC #Required for building shared libraries on x86_64
CXXFLAGS = -shared -Wall -fPIC

# Includes
SWIG_INCLUDES = -I$(CMISS_ROOT)/cmgui/source

#CMISS_LIBS = -L$(CMISS_ROOT)/cmgui/lib/x86_64-linux/cmgui-wx-debug/ -lcmgui
CMISS_LIBS = -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui

PYTHON_INCLUDES = -I/usr/include/python2.5 -I$(CMISS_ROOT)/cmgui/source

OCTAVE_INCLUDES = -I$(CMISS_ROOT)/cmgui/source

# Dependencies
INTERFACES = CmissCommandData.i CmissRegion.i CmissFieldModule.i
PYTHON_WRAPS = $(INTERFACES:.i=_wrap.cxx)
PYTHON_OBJECTS = $(PYTHON_WRAPS:.cxx=.o)
PYTHON_MODULES = $(subst Cmgui,_,$(INTERFACES:.i=.so))

CmguiPython: $(PYTHON_MODULES)

$(PYTHON_MODULES): $(PYTHON_OBJECTS)
	$(CXX) $(CXXFLAGS) $(CMISS_LIBS) $(subst .so,_wrap.o, $(subst _,Cmiss,$@)) -o $@

$(PYTHON_OBJECTS): $(PYTHON_WRAPS)
	$(CC) $(CFLAGS) $(PYTHON_INCLUDES) $(@:.o=.cxx)	
	
$(PYTHON_WRAPS): $(INTERFACES)
	swig -c++ -python $(SWIG_INCLUDES) $(@:_wrap.cxx=.i)

clean:
	rm -f $(PYTHON_WRAPS) $(PYTHON_OBJECTS) $(PYTHON_MODULES) $(subst Cmiss,,$(INTERFACES:.i=.py)) $(subst Cmiss,,$(INTERFACES:.i=.pyc))


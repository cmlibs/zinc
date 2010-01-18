#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 14 January 2010

#DESCRIPTION :

#makefile for cmgui API wrappers in Python using swig
#
# IMPORTANT NOTE!
# This makefile builds swig python wrappers in swig/python/Cmiss based on
# the new cmgui C++ api and interface files in swig/interface.
# At this time, the examples under the swig/python/test_examples work with
# the wrappers generated from sources in swig/cmiss
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

INTERFACE_PATH=$(CMISS_ROOT)/cmgui/source/swig/interface
OBJECT_PATH=$(CMISS_ROOT)/cmgui/source/swig/python/object
BIN_PATH=$(CMISS_ROOT)/cmgui/source/swig/python/Cmiss
VPATH=$(INTERFACE_PATH):$(OBJECT_PATH):$(BIN_PATH)

# Dependencies
INTERFACES = CmissCommandData.i CmissRegion.i CmissFieldModule.i
PYTHON_PACKAGE_INIT = __init__.py
PYTHON_WRAPS = $(INTERFACES:.i=_wrap.cxx)
PYTHON_OBJECTS = $(PYTHON_WRAPS:.cxx=.o)
PYTHON_MODULES = $(subst Cmiss,_,$(INTERFACES:.i=.so))

CmguiPython: $(PYTHON_MODULES) $(PYTHON_PACKAGE_INIT)

$(PYTHON_PACKAGE_INIT) : $(PYTHON_MODULES) makefile
	@if [ ! -d $(BIN_PATH) ]; then \
		mkdir -p $(BIN_PATH); \
	fi
	echo '# Cmiss Python package initialisation file' > $(BIN_PATH)/$(PYTHON_PACKAGE_INIT);

$(PYTHON_MODULES): $(PYTHON_OBJECTS)
	@if [ ! -d $(BIN_PATH) ]; then \
		mkdir -p $(BIN_PATH); \
	fi
	$(CXX) $(CXXFLAGS) $(CMISS_LIBS) $(OBJECT_PATH)/$(subst .so,_wrap.o,$(subst _,Cmiss,$@)) -o $(BIN_PATH)/$@

$(PYTHON_OBJECTS): $(PYTHON_WRAPS)
	@if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi
	$(CC) $(CFLAGS) $(PYTHON_INCLUDES) $(OBJECT_PATH)/$(@:.o=.cxx) -o $(OBJECT_PATH)/$@
	
$(PYTHON_WRAPS): $(INTERFACES)
	@if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi
	@if [ ! -d $(BIN_PATH) ]; then \
		mkdir -p $(BIN_PATH); \
	fi
	swig -c++ -python $(SWIG_INCLUDES) -o $(OBJECT_PATH)/$@ -outdir $(BIN_PATH) $(INTERFACE_PATH)/$(@:_wrap.cxx=.i)

clean:
	rm -r $(OBJECT_PATH)
	rm -r $(BIN_PATH)

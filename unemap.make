#Context of where we are
ifndef SYSNAME
  SYSNAME := $(shell uname)
  ifeq ($(SYSNAME),)
    $(error error with shell command uname)
  endif
endif

ifndef NODENAME
  NODENAME := $(shell uname -n)
  ifeq ($(NODENAME),)
    $(error error with shell command uname -n)
  endif
endif

ifndef MACHNAME
  MACHNAME := $(shell uname -m)
  ifeq ($(MACHNAME),)
    $(error error with shell command uname -m)
  endif
endif

#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=MOTIF_USER_INTERFACE
STATIC_LINK=false
DEBUG=true
ABI=
MEMORYCHECK=
USE_UNEMAP_NODES=
USE_UNEMAP_3D=

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

MAKEFILE = unemap.make
SUBMAKEFILE := unemap.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

#Developers default
unemap-debug :

#Separating these rules allow the command line options to propogate and
#variables that are not defined not to propogate.
unemap unemap-debug unemap-debug-memorycheck unemap-static unemap-static-debug unemap-nodes unemap-3d unemap-3d-debug unemap64 unemap64-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
unemap unemap-debug unemap-debug-memorycheck unemap-static unemap-static-debug unemap-nodes unemap-3d unemap-3d-debug unemap64 unemap64-debug : USER_INTERFACE=MOTIF_USER_INTERFACE
unemap-static unemap-static-debug : STATIC_LINK_OPTION=STATIC_LINK=$(STATIC_LINK)
unemap-static unemap-static-debug : STATIC_LINK=true
unemap unemap-nodes unemap-static unemap-3d unemap64 : DEBUG_OPTION=DEBUG=$(DEBUG)
unemap unemap-nodes unemap-static unemap-3d unemap64 : DEBUG=false
unemap-debug unemap-debug-memorycheck unemap-static-debug unemap-3d-debug unemap64-debug : DEBUG_OPTION=DEBUG=$(DEBUG)
unemap-debug unemap-debug-memorycheck unemap-static-debug unemap-3d-debug unemap64-debug : DEBUG=true
unemap64 unemap64-debug utilities64 : ABI_OPTION=ABI=$(ABI)
unemap64 unemap64-debug utilities64 : ABI=64
unemap-debug-memorycheck : MEMORYCHECK_OPTION=MEMORYCHECK=$(MEMORYCHECK)
unemap-debug-memorycheck : MEMORYCHECK=true
unemap-nodes : USE_UNEMAP_NODES_OPTION=USE_UNEMAP_NODES=$(USE_UNEMAP_NODES)
unemap-nodes : USE_UNEMAP_NODES=true
unemap-nodes unemap-3d : USE_UNEMAP_3D_OPTION=USE_UNEMAP_3D=$(USE_UNEMAP_3D)
unemap-nodes unemap-3d : USE_UNEMAP_3D=true

utilities utilities64 : TARGET_OPTION=utilities
utilities utilities64 : force

ifdef TARGET
   TARGET_OPTION = $(TARGET)
endif
ifdef USER_INTERFACE
   USER_INTERFACE_OPTION = USER_INTERFACE=$(USER_INTERFACE)
endif
ifdef STATIC_LINK
   STATIC_LINK_OPTION = STATIC_LINK=$(STATIC_LINK)
endif
ifdef DEBUG
   DEBUG_OPTION = DEBUG=$(DEBUG)
endif
ifdef ABI
   ABI_OPTION = ABI=$(ABI)
endif
ifdef MEMORYCHECK
   MEMORYCHECK_OPTION = MEMORYCHECK=$(MEMORYCHECK)
endif
ifdef USE_UNEMAP_NODES
   USE_UNEMAP_NODES_OPTION = USE_UNEMAP_NODES=$(USE_UNEMAP_NODES)
endif
ifdef USE_UNEMAP_3D
   USE_UNEMAP_3D_OPTION = USE_UNEMAP_3D=$(USE_UNEMAP_3D)
endif
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(STATIC_LINK_OPTION) $(DEBUG_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION) $(USE_UNEMAP_NODES_OPTION) $(USE_UNEMAP_3D_OPTION)

unemap unemap-debug unemap-debug-memorycheck unemap-static unemap-static-debug unemap-nodes unemap-3d unemap-3d-debug unemap64 unemap64-debug utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) ;

ifeq ($(SYSNAME:IRIX%=),)
all : unemap unemap-debug unemap-debug-memorycheck unemap-static unemap-static-debug unemap64 unemap-nodes unemap-3d unemap-3d-debug utilities utilities64
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
all : unemap unemap-debug unemap-debug-memorycheck unemap-static unemap-static-debug unemap-3d unemap-3d-debug utilities
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
all : unemap unemap-debug unemap64 unemap64-debug unemap-3d
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
all :
endif # SYSNAME == win32
ifeq ($(SYSNAME),CYGWIN%=)
all :
endif # SYSNAME == CYGWIN%=

update :
	$(CMISS_ROOT)/bin/cmissmake unemap;

cronjob:
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/unemap_programmer.mail ; \
		if ! $(MAKE) -f $(MAKEFILE) update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/unemap_programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

force :
	@echo "\n" > /dev/null

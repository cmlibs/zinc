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
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=MOTIF_USER_INTERFACE
STATIC_LINK=false
DEBUG=true
ABI=
MEMORYCHECK=

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

MAKEFILE = cmgui.make
SUBMAKEFILE := cmgui.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

#Developers default
cmgui-debug :

#Separating these rules allow the command line options to propogate and
#variables that are not defined not to propogate.
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE=MOTIF_USER_INTERFACE
cmgui-static cmgui-static-debug : STATIC_LINK_OPTION=STATIC_LINK=$(STATIC_LINK)
cmgui-static cmgui-static-debug : STATIC_LINK=true
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk : DEBUG=false
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug cmgui-gtk-debug : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug cmgui-gtk-debug : DEBUG=true
cmgui64 cmgui64-debug utilities64 : ABI_OPTION=ABI=$(ABI)
cmgui64 cmgui64-debug utilities64 : ABI=64
cmgui-debug-memorycheck : MEMORYCHECK_OPTION=MEMORYCHECK=$(MEMORYCHECK)
cmgui-debug-memorycheck : MEMORYCHECK=true
cmgui-console : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-console : USER_INTERFACE=CONSOLE_USER_INTERFACE
cmgui-gtk cmgui-gtk-debug cmgui-gtk-static-lib cmgui-gtk-lib : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-gtk cmgui-gtk-debug cmgui-gtk-static-lib cmgui-gtk-lib : USER_INTERFACE=GTK_USER_INTERFACE

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
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(STATIC_LINK_OPTION) $(DEBUG_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION)

cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui64 cmgui64-debug cmgui-console cmgui-gtk cmgui-gtk-debug utilities:
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) ;

#Recurse on these targets so we don't have to redefine in this file what cmgui-gtk-debug is etc.
cmgui-gtk-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=static_lib ;
cmgui-gtk-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=static_lib ;
cmgui-gtk-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=so_lib ;
cmgui-gtk-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=so_lib ;

.NOTPARALLEL:

ifeq ($(SYSNAME:IRIX%=),)
all : cmgui cmgui-debug cmgui64 cmgui-console cmgui-debug-memorycheck
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
all : cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui-console
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
all : cmgui cmgui-debug cmgui64 cmgui64-debug
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
all : cmgui-gtk
endif # SYSNAME == win32
ifeq ($(SYSNAME),CYGWIN%=)
all :
endif # SYSNAME == CYGWIN%=

update :
	$(CMISS_ROOT)/bin/cmissmake cmgui;

cronjob:
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/programmer.mail ; \
		if ! $(MAKE) -f $(MAKEFILE) update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

force :
	@echo "\n" > /dev/null

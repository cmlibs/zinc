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
PROFILE=false
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
ifeq ($(SYSNAME),win32)
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE=WIN32_USER_INTERFACE
else # SYSNAME == win32
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE=MOTIF_USER_INTERFACE
endif # SYSNAME == win32
cmgui-static cmgui-static-debug : STATIC_LINK_OPTION=STATIC_LINK=$(STATIC_LINK)
cmgui-static cmgui-static-debug : STATIC_LINK=true
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk cmgui-gtk-gtkmain cmgui-no3dgraphics cmgui64-no3dgraphics : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk cmgui-gtk-gtkmain cmgui-no3dgraphics cmgui64-no3dgraphics : DEBUG=false
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug cmgui-gtk-debug cmgui-gtk-gtkmain-debug cmgui-no3dgraphics-debug cmgui-no3dgraphics-debug-memorycheck cmgui64-no3dgraphics-debug : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug cmgui-gtk-debug cmgui-gtk-gtkmain-debug cmgui-no3dgraphics-debug cmgui-no3dgraphics-debug-memorycheck cmgui64-no3dgraphics-debug : DEBUG=true
cmgui64 cmgui64-debug cmgui64-no3dgraphics cmgui64-no3dgraphics-debug utilities64 : ABI_OPTION=ABI=$(ABI)
cmgui64 cmgui64-debug cmgui64-no3dgraphics cmgui64-no3dgraphics-debug utilities64 : ABI=64
cmgui-debug-memorycheck cmgui-no3dgraphics-debug-memorycheck : MEMORYCHECK_OPTION=MEMORYCHECK=$(MEMORYCHECK)
cmgui-debug-memorycheck cmgui-no3dgraphics-debug-memorycheck : MEMORYCHECK=true
cmgui-console : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-console : USER_INTERFACE=CONSOLE_USER_INTERFACE
cmgui-gtk cmgui-gtk-debug cmgui-gtk-gtkmain cmgui-gtk-gtkmain-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-gtk cmgui-gtk-debug cmgui-gtk-gtkmain cmgui-gtk-gtkmain-debug : USER_INTERFACE=GTK_USER_INTERFACE
cmgui-gtk-gtkmain cmgui-gtk-gtkmain-debug : USE_GTKMAIN_OPTION=USE_GTKMAIN=$(USE_GTKMAIN)
cmgui-gtk-gtkmain cmgui-gtk-gtkmain-debug : USE_GTKMAIN=true
cmgui-no3dgraphics cmgui-no3dgraphics-debug cmgui-no3dgraphics-debug-memorycheck cmgui64-no3dgraphics cmgui64-no3dgraphics-debug : GRAPHICS_API_OPTION=GRAPHICS_API=$(GRAPHICS_API)
cmgui-no3dgraphics cmgui-no3dgraphics-debug cmgui-no3dgraphics-debug-memorycheck cmgui64-no3dgraphics cmgui64-no3dgraphics-debug : GRAPHICS_API=NO3D_GRAPHICS

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
ifdef PROFILE
   PROFILE_OPTION = PROFILE=$(PROFILE)
endif
ifdef ABI
   ABI_OPTION = ABI=$(ABI)
endif
ifdef MEMORYCHECK
   MEMORYCHECK_OPTION = MEMORYCHECK=$(MEMORYCHECK)
endif
ifdef GRAPHICS_API
   GRAPHICS_API_OPTION = GRAPHICS_API=$(GRAPHICS_API)
endif
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(STATIC_LINK_OPTION) $(DEBUG_OPTION) $(PROFILE_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION) $(USE_GTKMAIN_OPTION) $(GRAPHICS_API_OPTION)

#Force the use of the cross compiler for cmiss on Linux.
ifeq ($(SYSNAME),Linux)
   ifeq ($(MACHNAME),i686)
      ifeq ($(USER),cmiss)
         MAKE=i386-glibc21-linux-cross-make
      endif
   endif
endif

cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui64 cmgui64-debug cmgui-console cmgui-gtk cmgui-gtk-debug utilities cmgui-gtk-gtkmain cmgui-gtk-gtkmain-debug cmgui-no3dgraphics cmgui-no3dgraphics-debug cmgui-no3dgraphics-debug-memorycheck cmgui64-no3dgraphics cmgui64-no3dgraphics-debug :
		cd source ; \
		$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) ;

#Recurse on these targets so we don't have to redefine in this file what cmgui-gtk-debug is etc.
cmgui-lib :
	$(MAKE) -f $(MAKEFILE) cmgui TARGET=so_lib ;
cmgui-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-debug TARGET=so_lib ;
cmgui-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui TARGET=static_lib ;
cmgui-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-debug TARGET=static_lib ;
cmgui-gtk-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=so_lib ;
cmgui-gtk-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=so_lib ;
cmgui-gtk-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=static_lib ;
cmgui-gtk-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=static_lib ;
cmgui-gtk-gtkmain-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-gtkmain TARGET=so_lib ;
cmgui-gtk-gtkmain-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-gtkmain-debug TARGET=so_lib ;
cmgui-gtk-gtkmain-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-gtkmain TARGET=static_lib ;
cmgui-gtk-gtkmain-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-gtkmain-debug TARGET=static_lib ;
cmgui-no3dgraphics-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics TARGET=so_lib ;
cmgui-no3dgraphics-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-debug TARGET=so_lib ;
cmgui-no3dgraphics-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics TARGET=static_lib ;
cmgui-no3dgraphics-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-debug TARGET=static_lib ;
cmgui-no3dgraphics-debug-memorycheck-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-debug-memorycheck TARGET=static_lib ;
cmgui64-no3dgraphics-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui64-no3dgraphics TARGET=static_lib ;
cmgui64-no3dgraphics-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui64-no3dgraphics-debug TARGET=static_lib ;

.NOTPARALLEL:

ifeq ($(SYSNAME:IRIX%=),)
all : cmgui cmgui-debug cmgui64 cmgui-console cmgui-debug-memorycheck cmgui-static-lib cmgui-debug-static-lib cmgui-no3dgraphics-static-lib cmgui-no3dgraphics-debug-static-lib cmgui-no3dgraphics-debug-memorycheck-static-lib cmgui64-no3dgraphics-static-lib
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifeq ($(MACHNAME),x86_64)
all : cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static-lib cmgui-debug-static-lib cmgui-no3dgraphics-static-lib cmgui-no3dgraphics-debug-static-lib cmgui-no3dgraphics-debug-memorycheck-static-lib
   else # MACHNAME == x86_64
all : cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui-console cmgui-static-lib cmgui-debug-static-lib cmgui-gtk cmgui-gtk-debug cmgui-gtk-lib cmgui-gtk-debug-lib cmgui-gtk-static-lib cmgui-gtk-debug-static-lib cmgui-gtk-gtkmain-lib cmgui-gtk-gtkmain-debug-lib cmgui-gtk-gtkmain-static-lib cmgui-gtk-gtkmain-debug-static-lib cmgui-no3dgraphics-static-lib cmgui-no3dgraphics-debug-static-lib cmgui-no3dgraphics-debug-memorycheck-static-lib
   endif # MACHNAME == x86_64
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
all : cmgui cmgui-debug cmgui64 cmgui64-debug cmgui-static-lib cmgui-no3dgraphics-static-lib cmgui-no3dgraphics-debug-static-lib cmgui64-no3dgraphics-static-lib cmgui64-no3dgraphics-debug-static-lib
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
all : cmgui cmgui-debug cmgui-gtk
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

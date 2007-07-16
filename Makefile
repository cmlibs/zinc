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

ifeq ($(SYSNAME:CYGWIN%=),)# CYGWIN
  #Default to win32
  SYSNAME=win32
endif
ifeq ($(SYSNAME:MINGW32%=),)# MSYS
  #Default to win32
  SYSNAME=win32
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

MAKEFILE = Makefile
SUBMAKEFILE := cmgui.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

sinclude source/developer.Makefile

#Developers default
cmgui-motif-debug :

#Separating these rules allow the command line options to propogate and
#variables that are not defined not to propogate.
cmgui-motif cmgui-motif-debug cmgui-motif-static cmgui-motif-static-debug cmgui64-motif cmgui64-motif-debug cmgui-motif-unemap cmgui-motif-debug-unemap : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-motif cmgui-motif-debug cmgui-motif-static cmgui-motif-static-debug cmgui64-motif cmgui64-motif-debug cmgui-motif-unemap cmgui-motif-debug-unemap : USER_INTERFACE=MOTIF_USER_INTERFACE
cmgui-win32 cmgui-win32-debug cmgui-win32-static cmgui-win32-static-debug cmgui64-win32 cmgui64-win32-debug : USER_INTERFACE=WIN32_USER_INTERFACE
cmgui-win32 cmgui-win32-debug cmgui-win32-static cmgui-win32-static-debug cmgui64-win32 cmgui64-win32-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-motif-static cmgui-motif-static-debug : STATIC_LINK_OPTION=STATIC_LINK=$(STATIC_LINK)
cmgui-motif-static cmgui-motif-static-debug : STATIC_LINK=true
cmgui-motif cmgui-motif-static cmgui64-motif cmgui-console cmgui-wx cmgui64-wx cmgui-carbon cmgui-gtk cmgui-no3dgraphics-motif cmgui64-no3dgraphics-motif cmgui-motif-unemap cmgui-win32 : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui-motif cmgui-motif-static cmgui64-motif cmgui-console cmgui-wx cmgui64-wx cmgui-carbon cmgui-gtk cmgui-no3dgraphics-motif cmgui64-no3dgraphics-motif cmgui-motif-unemap cmgui-win32 : DEBUG=false
cmgui-motif-debug cmgui-motif-debug-memorycheck cmgui-wx-debug-memorycheck cmgui64-wx-debug cmgui64-wx-debug-memorycheck cmgui-motif-static-debug cmgui64-motif-debug cmgui-wx-debug  cmgui-carbon-debug cmgui-gtk-debug cmgui-no3dgraphics-motif-debug cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-no3dgraphics-motif-debug cmgui-motif-unemap-debug cmgui-win32-debug : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui-motif-debug cmgui-motif-debug-memorycheck cmgui-wx-debug-memorycheck cmgui64-wx-debug cmgui64-wx-debug-memorycheck cmgui-motif-static-debug cmgui64-motif-debug cmgui-wx-debug  cmgui-carbon-debug cmgui-gtk-debug cmgui-no3dgraphics-motif-debug cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-no3dgraphics-motif-debug cmgui-motif-unemap-debug cmgui-win32-debug : DEBUG=true
cmgui64-motif cmgui64-motif-debug cmgui64-no3dgraphics-motif cmgui64-no3dgraphics-motif-debug utilities64 : ABI_OPTION=ABI=$(ABI)
cmgui64-motif cmgui64-motif-debug cmgui64-no3dgraphics-motif cmgui64-no3dgraphics-motif-debug utilities64 : ABI=64
cmgui-motif-debug-memorycheck cmgui-wx-debug-memorycheck cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-wx-debug-memorycheck: MEMORYCHECK_OPTION=MEMORYCHECK=$(MEMORYCHECK)
cmgui-motif-debug-memorycheck cmgui-wx-debug-memorycheck cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-wx-debug-memorycheck: MEMORYCHECK=true
cmgui-console : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-console : USER_INTERFACE=CONSOLE_USER_INTERFACE
cmgui-gtk cmgui-gtk-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-wx cmgui-wx-debug cmgui-wx-debug-memorycheck cmgui64-wx cmgui64-wx-debug cmgui64-wx-debug-memorycheck: USER_INTERFACE=WX_USER_INTERFACE
cmgui-wx cmgui-wx-debug cmgui-wx-debug-memorycheck cmgui64-wx cmgui64-wx-debug cmgui64-wx-debug-memorycheck: USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-carbon cmgui-carbon-debug : USER_INTERFACE=CARBON_USER_INTERFACE
cmgui-carbon cmgui-carbon-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-gtk cmgui-gtk-debug : USER_INTERFACE=GTK_USER_INTERFACE
cmgui-no3dgraphics-motif cmgui-no3dgraphics-motif-debug cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-no3dgraphics-motif cmgui64-no3dgraphics-motif-debug : GRAPHICS_API_OPTION=GRAPHICS_API=$(GRAPHICS_API)
cmgui-no3dgraphics-motif cmgui-no3dgraphics-motif-debug cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-no3dgraphics-motif cmgui64-no3dgraphics-motif-debug : GRAPHICS_API=NO3D_GRAPHICS
cmgui-motif-unemap cmgui-motif-unemap-debug : UNEMAP_OPTION=UNEMAP=$(UNEMAP)
cmgui-motif-unemap cmgui-motif-unemap-debug : UNEMAP=true

utilities utilities64 : TARGET_OPTION=utilities
utilities utilities64 : force

clean : force
	@echo "To 'make clean' a single version of cmgui, say cmgui-wx-debug use 'make cmgui-wx-debug TARGET=clean'.  To clean everything use 'make clean-all'"

clean-all :
	-rm -r object
	-rm -r uidh
	-rm -r xrch

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
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(STATIC_LINK_OPTION) $(DEBUG_OPTION) $(PROFILE_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION) $(GRAPHICS_API_OPTION) $(UNEMAP_OPTION)

#Force the use of the cross compiler for cmiss on Linux.
ifeq ($(SYSNAME),Linux)
   ifeq ($(MACHNAME),i686)
      ifeq ($(USER),cmiss)
         MAKE=i386-glibc23-linux-cross-make
      endif
   endif
endif

cmgui-motif cmgui-motif-debug cmgui-motif-debug-memorycheck cmgui-motif-static cmgui-motif-static-debug cmgui64-motif cmgui64-motif-debug cmgui-console cmgui-wx cmgui-wx-debug cmgui-wx-debug-memorycheck cmgui64-wx cmgui64-wx-debug cmgui64-wx-debug-memorycheck cmgui-carbon cmgui-carbon-debug cmgui-gtk cmgui-gtk-debug utilities cmgui-no3dgraphics-motif cmgui-no3dgraphics-motif-debug cmgui-no3dgraphics-motif-debug-memorycheck cmgui64-no3dgraphics-motif cmgui64-no3dgraphics-motif-debug cmgui-motif-unemap cmgui-motif-unemap-debug cmgui-win32 cmgui-win32-debug :
		cd source ; \
		$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) $(DEVELOPER_OPTIONS) ;

#Recurse on these targets so we don't have to redefine in this file what cmgui-gtk-debug is etc.
cmgui-motif-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-motif TARGET=so_lib ;
cmgui-motif-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-motif-debug TARGET=so_lib ;
cmgui-motif-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-motif TARGET=static_lib ;
cmgui-motif-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-motif-debug TARGET=static_lib ;
cmgui-gtk-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=so_lib ;
cmgui-gtk-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=so_lib ;
cmgui-gtk-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=static_lib ;
cmgui-gtk-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-gtk-debug TARGET=static_lib ;
cmgui-carbon-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-carbon TARGET=static_lib ;
cmgui-carbon-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-carbon-debug TARGET=static_lib ;
cmgui-win32-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-win32 TARGET=so_lib ;
cmgui-win32-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-win32-debug TARGET=so_lib ;
cmgui-win32-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-win32 TARGET=static_lib ;
cmgui-win32-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-win32-debug TARGET=static_lib ;
cmgui-no3dgraphics-motif-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-motif TARGET=so_lib ;
cmgui-no3dgraphics-motif-debug-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-motif-debug TARGET=so_lib ;
cmgui-no3dgraphics-motif-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-motif TARGET=static_lib ;
cmgui-no3dgraphics-motif-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-motif-debug TARGET=static_lib ;
cmgui-no3dgraphics-motif-debug-memorycheck-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui-no3dgraphics-motif-debug-memorycheck TARGET=static_lib ;
cmgui64-no3dgraphics-motif-static-lib :
	$(MAKE) -f $(MAKEFILE) cmgui64-no3dgraphics-motif TARGET=static_lib ;
cmgui64-no3dgraphics-motif-debug-static-lib:
	$(MAKE) -f $(MAKEFILE) cmgui64-no3dgraphics-motif-debug TARGET=static_lib ;

.NOTPARALLEL:

ifeq ($(SYSNAME:IRIX%=),)
all : cmgui-motif cmgui-motif-debug cmgui64-motif cmgui-console cmgui-motif-debug-memorycheck cmgui-motif-static-lib cmgui-motif-debug-static-lib cmgui-no3dgraphics-motif-static-lib cmgui-no3dgraphics-motif-debug-static-lib cmgui-no3dgraphics-motif-debug-memorycheck-static-lib cmgui64-no3dgraphics-motif-static-lib
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifeq ($(MACHNAME),x86_64)
all : svn_update cmgui-motif cmgui-motif-debug cmgui-motif-debug-memorycheck cmgui-motif-static-lib cmgui-motif-debug-static-lib cmgui-no3dgraphics-motif-static-lib cmgui-no3dgraphics-motif-debug-static-lib cmgui-no3dgraphics-motif-debug-memorycheck-static-lib cmgui-gtk-debug-static-lib cmgui-gtk-static-lib cmgui64-wx cmgui64-wx-debug cmgui64-wx-debug-memorycheck
   else # MACHNAME == x86_64
all : svn_update cmgui-motif cmgui-motif-debug cmgui-motif-debug-memorycheck cmgui-motif-static cmgui-motif-static-debug cmgui-console cmgui-motif-static-lib cmgui-motif-debug-static-lib cmgui-gtk cmgui-gtk-debug cmgui-gtk-lib cmgui-gtk-debug-lib cmgui-gtk-static-lib cmgui-gtk-debug-static-lib cmgui-no3dgraphics-motif-static-lib cmgui-no3dgraphics-motif-debug-static-lib cmgui-no3dgraphics-motif-debug-memorycheck-static-lib cmgui-motif-unemap cmgui-wx cmgui-wx-debug cmgui-wx-debug-memorycheck
   endif # MACHNAME == x86_64
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
all : cmgui-motif cmgui-motif-debug cmgui64-motif cmgui64-motif-debug cmgui-motif-static-lib cmgui-no3dgraphics-motif-static-lib cmgui-no3dgraphics-motif-debug-static-lib cmgui64-no3dgraphics-motif-static-lib cmgui64-no3dgraphics-motif-debug-static-lib
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
all : svn_update cmgui-win32 cmgui-win32-debug cmgui-gtk cmgui-wx cmgui-wx-debug cmgui-win32-lib cmgui-win32-static-lib cmgui-win32-debug-lib cmgui-win32-debug-static-lib
endif # SYSNAME == win32
ifeq ($(SYSNAME),CYGWIN%=)
all :
endif # SYSNAME == CYGWIN%=
ifeq ($(SYSNAME),Darwin)
   ifeq ($(MACHNAME),i386)
      all : cmgui-carbon cmgui-carbon-debug cmgui-carbon-debug-static-lib cmgui-wx cmgui-wx-debug
   else # MACHNAME == i386
      all : cmgui-motif cmgui-motif-debug cmgui-motif-static-lib cmgui-no3dgraphics-motif-static-lib cmgui-no3dgraphics-motif-debug-static-lib cmgui-carbon cmgui-carbon-debug cmgui-carbon-debug-static-lib
   endif # MACHNAME == i386
endif # SYSNAME == Darwin

#Update whereever possible first (svn not available on our IRIX,
#	isn't installed on bioeng21 and doesn't support ssl on hpc!)
svn_update :
	svn update;

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

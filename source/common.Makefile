# **************************************************************************
# FILE : common.Makefile
#
# LAST MODIFIED : 12 June 2003
#
# DESCRIPTION :
#
# Makefile for common rules for cmgui, unemap and cell
# ==========================================================================


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

ifndef DEBUG
  ifndef OPT
    OPT = false
  endif
  ifeq ($(OPT),false)
    DEBUG = true
  else
    DEBUG = false
  endif
endif

ifndef MP
  MP = false
endif

# set architecture dependent directories and default options
ifeq ($(SYSNAME:IRIX%=),)# SGI
  # Specify what application binary interface (ABI) to use i.e. 32, n32 or 64
  ifndef ABI
    ifdef SGI_ABI
      ABI := $(patsubst -%,%,$(SGI_ABI))
    else
      ABI = n32
    endif
  endif
  # Specify which instruction set to use i.e. -mips#
  ifndef MIPS
    # Using mips3 for most basic version on esu* machines
    # as there are still some Indys around.
    # Although mp versions are unlikely to need mips3 they are made this way
    # because it makes finding library locations easier.
    MIPS = 4
    ifeq ($(filter-out esu%,$(NODENAME)),)
      ifeq ($(ABI),n32)
        ifneq ($(DEBUG),false)
          MIPS=3
        endif
      endif
    endif
  endif
  INSTRUCTION = mips
  OPERATING_SYSTEM = irix
endif
ifeq ($(SYSNAME),Linux)
  ifndef STATIC
    STATIC = true
  endif
  ifeq ($(filter-out i%86,$(MACHNAME)),)
    INSTRUCTION = i686
  else
    INSTRUCTION := $(MACHNAME)
  endif
  OPERATING_SYSTEM = linux
endif
ifeq ($(SYSNAME),win32)
  INSTRUCTION = i386
  OPERATING_SYSTEM = win32
  ifndef STATIC
    STATIC = true
  endif
endif
ifeq ($(SYSNAME),SunOS)
  INSTRUCTION = 
  OPERATING_SYSTEM = solaris
endif
ifeq ($(SYSNAME),AIX)
  ifndef ABI
    ifdef OBJECT_MODE
      ifneq ($(OBJECT_MODE),32_64)
        ABI = $(OBJECT_MODE)
      endif
    endif
  endif
  ifndef ABI
    ABI = 32
  endif
  INSTRUCTION = rs6000
  OPERATING_SYSTEM = aix
endif
ifeq ($(SYSNAME:CYGWIN%=),)# CYGWIN
  ifndef STATIC
    STATIC = true
  endif
  INSTRUCTION = i686
  OPERATING_SYSTEM = cygwin
endif

BIN_ARCH_DIR = $(INSTRUCTION)-$(OPERATING_SYSTEM)
ifdef ABI
   LIB_ARCH_DIR = $(INSTRUCTION)-$(ABI)-$(OPERATING_SYSTEM)
else # ABI
   LIB_ARCH_DIR = $(INSTRUCTION)-$(OPERATING_SYSTEM)
endif # ABI

ifeq ($(SYSNAME:IRIX%=),)
   ifneq ($(ABI),64)
      UIL = uil
   else # ABI != 64
      UIL = uil64
   endif # ABI != 64
   CC = cc -c
   CPP = CC -c
   CPP_FLAGS = -LANG:std
   FORTRAN = f77 -c
   MAKEDEPEND = makedepend -f- -Y --
   CPREPROCESS = cc -P
   # LINK = cc
   # Must use C++ linker for XML */
   LINK = CC
   ifneq ($(DEBUG),true)
      OPTIMISATION_FLAGS = -O
      COMPILE_FLAGS = -ansi -pedantic -woff 1521
      COMPILE_DEFINES = -DOPTIMISED
      STRICT_FLAGS = 
      CPP_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match */
   else # DEBUG != true
      OPTIMISATION_FLAGS = -g
      COMPILE_FLAGS = -ansi -fullwarn -pedantic -woff 1521
      COMPILE_DEFINES = -DREPORT_GL_ERRORS -DUSE_PARAMETER_ON 
      # STRICT_FLAGS = -diag_error 1042,1174,1185,1196,1409,1551,1552,3201
      STRICT_FLAGS = -diag_error 1000-9999
      # need to suppress for Boost mostly but also for STL
      CPP_STRICT_FLAGS = -diag_error 1000-9999 -diag_suppress 1110,1174,1209,1234,1375,1424,3201
      DIGITAL_MEDIA_NON_STRICT_FLAGS = -diag_warning 1429
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = three_d_drawing/dm_interface.c | three_d_drawing/movie_extensions.c
   endif # DEBUG != true
   ifeq ($(MIPS),3)
      OPTIMISATION_FLAGS += -mips3
   else
      OPTIMISATION_FLAGS += -mips4
   endif

   ifeq ($(ABI),64)
      TARGET_TYPE_FLAGS = -G0 -64
      TARGET_TYPE_DEFINES = -DO64
   else # ABI == 64
      TARGET_TYPE_FLAGS = -n32
      TARGET_TYPE_DEFINES =
   endif # ABI == 64
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   UIL = uil
#    ifeq ($(MACHNAME),ia64)
#       # Using Intel compilers
#       CC = ecc -c
#       CPP = $(CC)
#       # turn on warnings,
#       # suppress messages about non-standard fortran (including REAL*8,
#       # more than 19 continuation lines),
#       # suppress comment messages (including obsolescent alternate return),
#       # auto arrays (as well as scalars), no aliasing,
#       # no preprocessing, temporary arrays on stack if possible.
#       FORTRAN = efc -c -W1 -w95 -cm -auto -fpp0 -stack_temps
#    else
      # gcc
      CC = gcc -c
      CPP = g++ -c
      CPP_FLAGS =
      FORTRAN = g77 -c -fno-second-underscore
#    endif
   MAKEDEPEND = gcc -MM -MG
   CPREPROCESS = gcc -E -P
   ifneq ($(STATIC_LINK),true)
      LINK = gcc
      # LINK = egcs -shared -L/usr/X11R6/lib -v */
      # LINK = gcc -L/usr/X11R6/lib -v */
   else # STATIC_LINK) != true
      LINK = gcc -static
      # LINK = g++ --no-demangle -rdynamic -L/usr/X11R6/lib*/
   endif # STATIC_LINK) != true
   ifneq ($(DEBUG),true)
      OPTIMISATION_FLAGS = -O
      COMPILE_DEFINES = -DOPTIMISED
      COMPILE_FLAGS = -fPIC
      STRICT_FLAGS = -Werror
      CPP_STRICT_FLAGS = -Werror
      DIGITAL_MEDIA_NON_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match
   else  # DEBUG != true
      OPTIMISATION_FLAGS = -g
      COMPILE_DEFINES = -DREPORT_GL_ERRORS -DUSE_PARAMETER_ON
      COMPILE_FLAGS = -fPIC
      # A bug with gcc on esp56 stops -Wformat from working */
      STRICT_FLAGS = -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Werror
      CPP_STRICT_FLAGS = -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Werror
      DIGITAL_MEDIA_NON_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match */
      TARGET_TYPE_FLAGS =
      TARGET_TYPE_DEFINES =
   endif # DEBUG != true
   ifeq ($(MACHNAME),ia64)
      TARGET_TYPE_DEFINES = -DO64
      # To work around errors of the form:
      # /home/hunter/gui/cmgui/source/api/cmiss_core.c:60: relocation truncated to fit: PCREL21B display_message
      LINK += -Wl,-relax
   endif
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   UIL = uil
   CC = xlc -c
   CPP = xlc -qnolm -c
   CPP_FLAGS =
   FORTRAN = f77 -c
   MAKEDEPEND = makedepend -f-
   CPREPROCESS = 
   #Specify a runtime libpath so that the executable doesn't just add in the location
   #of every library (dynamic and static).  In particular I do not want Mesa to be loaded
   #by default even if that is what is linked against.
   LINK = xlc -blibpath:/usr/lib:/lib
   ifeq ($(ABI),32)
      #Increase the maximum memory for a 32 bit executable
      LINK += -bmaxdata:0x80000000
   endif # $(ABI) == 32
   ifneq ($(DEBUG),true)
      OPTIMISATION_FLAGS = -O2 -qmaxmem=-1
   else # DEBUG != true
      OPTIMISATION_FLAGS = -g
   endif # DEBUG != true
   COMPILE_FLAGS = 
   COMPILE_DEFINES = 
   STRICT_FLAGS = 
   CPP_STRICT_FLAGS = 
   DIGITAL_MEDIA_NON_STRICT_FLAGS = 
   DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match
   ifeq ($(ABI),64)
      TARGET_TYPE_FLAGS =  -q64
      TARGET_TYPE_DEFINES = -DO64
      AR_FLAGS = -X64
   else # ABI == 64
      TARGET_TYPE_FLAGS = -q32
      TARGET_TYPE_DEFINES =
   endif # ABI == 64
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   UIL = uil
   # CC = gcc -c -mno-cygwin -fnative-struct */
   CC = gcc -c -mno-cygwin -mms-bitfields
   CPP = gcc -c
   CPP_FLAGS =
   FORTRAN = f77 -c
   MAKEDEPEND = gcc -MM -MG
   CPREPROCESS = 
   LINK = gcc -mno-cygwin -fnative-struct -mms-bitfields
   ifeq ($(filter-out CONSOLE_USER_INTERFACE GTK_USER_INTERFACE,$(USER_INTERFACE)),)
      LINK += -mconsole 
   else # $(USER_INTERFACE) == CONSOLE_USER_INTERFACE || $(USER_INTERFACE) == GTK_USER_INTERFACE
      LINK += -mwindows
   endif # $(USER_INTERFACE) == CONSOLE_USER_INTERFACE || $(USER_INTERFACE) == GTK_USER_INTERFACE
   ifneq ($(DEBUG),true)
      OPTIMISATION_FLAGS = -O2
   else # DEBUG != true
      OPTIMISATION_FLAGS = -g
   endif # DEBUG != true
   COMPILE_FLAGS = 
   COMPILE_DEFINES = 
   STRICT_FLAGS = 
   CPP_STRICT_FLAGS = 
   DIGITAL_MEDIA_NON_STRICT_FLAGS = 
   DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match
   TARGET_TYPE_FLAGS =
   TARGET_TYPE_DEFINES =
endif # SYSNAME == win32
ifeq ($(SYSNAME:CYGWIN%=),)# CYGWIN
   UIL = uil
   CC = gcc -c
   CPP = g++ -c
   CPP_FLAGS =
   FORTRAN = g77 -c -fno-second-underscore
   MAKEDEPEND = gcc -MM -MG
   CPREPROCESS = gcc -E -P
   ifneq ($(STATIC_LINK),true)
      LINK = gcc
      # LINK = egcs -shared -L/usr/X11R6/lib -v */
      # LINK = gcc -L/usr/X11R6/lib -v */
   else # STATIC_LINK) != true
      LINK = gcc -static
      # LINK = g++ --no-demangle -rdynamic -L/usr/X11R6/lib*/
   endif # STATIC_LINK) != true
   ifneq ($(DEBUG),true)
      OPTIMISATION_FLAGS = -O
      COMPILE_DEFINES = -DOPTIMISED
      COMPILE_FLAGS = 
      STRICT_FLAGS = -Werror
      CPP_STRICT_FLAGS = -Werror
      DIGITAL_MEDIA_NON_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match
   else  # DEBUG != true
      OPTIMISATION_FLAGS = -g
      COMPILE_DEFINES = -DREPORT_GL_ERRORS -DUSE_PARAMETER_ON
      COMPILE_FLAGS = 
      # A bug with gcc on esp56 stops -Wformat from working */
      STRICT_FLAGS = -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Werror
      CPP_STRICT_FLAGS = -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Werror
      DIGITAL_MEDIA_NON_STRICT_FLAGS = 
      DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN = NONE # Must specify a pattern that doesn't match */
      TARGET_TYPE_FLAGS =
      TARGET_TYPE_DEFINES =
   endif # DEBUG != true
endif # SYSNAME == CYGWIN%=

#Always look in cmgui utilities so that gx finds it.
PRODUCT_UTILITIES_PATH=$(CMISS_ROOT)/cmgui/utilities/$(BIN_ARCH_DIR)
UTILITIES_PATH=$(CMGUI_DEV_ROOT)/utilities/$(BIN_ARCH_DIR)

# DSO Link command
DSO_LINK = $(LINK) $(ALL_FLAGS) -shared

.MAKEOPTS : -r

.SUFFIXES :
.SUFFIXES : .o .c .d .cpp .f .uil .uidh
ifeq ($(SYSNAME),win32)
.SUFFIXES : .res .rc
endif # SYSNAME == win32

%.o: %.c %.d 
	@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
		mkdir -p $(OBJECT_PATH)/$(*D); \
	fi
ifdef CMISS_ROOT_DEFINED
	@if [ -f $*.c ]; then \
	   case $*.c in  \
	      $(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
	         set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $*.c;; \
	      * ) \
	         set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $*.c;; \
	   esac ; \
	else \
	   case $*.c in  \
	      $(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
	         set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $(PRODUCT_SOURCE_PATH)/$*.c;; \
	      * ) \
		      set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $(PRODUCT_SOURCE_PATH)/$*.c;; \
	   esac ; \
	fi
else # CMISS_ROOT_DEFINED
	@case $*.c in  \
	   $(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
	      set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $*.c;; \
	   * ) \
	      set -x ; $(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(STRICT_FLAGS) $*.c;; \
	esac ;
endif # CMISS_ROOT_DEFINED

%.o: %.cpp %.d
	@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
		mkdir -p $(OBJECT_PATH)/$(*D); \
	fi
ifdef CMISS_ROOT_DEFINED
	@if [ -f $*.cpp ]; then \
	   case $*.cpp in  \
	      $(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
            set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $*.cpp;; \
	      * ) \
	   	   set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $*.cpp;; \
	   esac ; \
	else \
	   case $*.cpp in  \
	      $(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
	        set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $(PRODUCT_SOURCE_PATH)/$*.cpp;; \
	      * ) \
	        set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $(PRODUCT_SOURCE_PATH)/$*.cpp;; \
	   esac ; \
	fi
else # CMISS_ROOT_DEFINED
	case $*.cpp in  \
		$(DIGITAL_MEDIA_NON_STRICT_FLAGS_PATTERN) ) \
	  set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $(DIGITAL_MEDIA_NON_STRICT_FLAGS) $*.cpp;; \
	  * ) \
	  set -x ; $(CPP) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(CPP_FLAGS) $(CPP_STRICT_FLAGS) $*.cpp;; \
	esac ;
endif # CMISS_ROOT_DEFINED)

%.o: %.f %.d
	@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
		mkdir -p $(OBJECT_PATH)/$(*D); \
	fi
ifdef CMISS_ROOT_DEFINED
	@if [ -f $*.f ]; then \
	   case $*.f in  \
	      * ) \
	        set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o $(OBJECT_PATH)/$*.o $(SOURCE_DIRECTORY_INC) $*.f ;; \
	   esac ; \
	else \
	   case $*.f in  \
	      * ) \
	        set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o $(OBJECT_PATH)/$*.o $(SOURCE_DIRECTORY_INC) $(PRODUCT_SOURCE_PATH)/$*.f ;; \
	   esac ; \
	fi
else # CMISS_ROOT_DEFINED
	case $*.f in  \
		* ) \
	  set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o $(OBJECT_PATH)/$*.o $(SOURCE_DIRECTORY_INC) $*.f ;; \
	esac ;
endif # CMISS_ROOT_DEFINED

$(OBJECT_PATH)/%.c : $(SOURCE_PATH)/%.c

ifdef NO_MAKE_DEPEND
%.d :
	@:
else # NO_MAKE_DEPEND
%.d :
	if [ ! -d $(OBJECT_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$(*D))) ]; then \
		mkdir -p $(OBJECT_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$(*D))) ; \
	fi
	@stem_name=$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$*)); \
   source_name=$(firstword $(wildcard $(SOURCE_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$*)).c $(SOURCE_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$*)).cpp $(PRODUCT_SOURCE_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$*)).c $(PRODUCT_SOURCE_PATH)/$(subst $(PRODUCT_OBJECT_PATH)/,,$(subst $(OBJECT_PATH)/,,$*)).cpp)) ; \
	set -x ; $(MAKEDEPEND) $(ALL_FLAGS) $(STRICT_FLAGS) $${source_name} 2> /dev/null | \
	sed -e "s%^.*\.o%$${stem_name}.o $(OBJECT_PATH)/$${stem_name}.d%;s%$(SOURCE_PATH)/%%g;s%$(PRODUCT_SOURCE_PATH)/%%g;s%/usr/local/.*\.h%%g" > $(OBJECT_PATH)/$${stem_name}.d ;
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   # Fix up the uidh references
	@stem_name=$(subst $(PRODUCT_OBJECT_PATH),,$(subst $(OBJECT_PATH)/,,$*)); \
	sed -e 's%$(UIDH_PATH)/%%g;s%$(PRODUCT_UIDH_PATH)/%%g' $(OBJECT_PATH)/$${stem_name}.d > $(OBJECT_PATH)/$${stem_name}.d2 ; \
	mv $(OBJECT_PATH)/$${stem_name}.d2 $(OBJECT_PATH)/$${stem_name}.d
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
endif # NO_MAKE_DEPEND

%.d: %.f
	@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
		mkdir -p $(OBJECT_PATH)/$(*D); \
	fi
	touch $(OBJECT_PATH)/$@

ifeq ($(USER_INTERFACE),WIN32_USER_INTERFACE)
.rc.res:
	@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
		mkdir -p $(OBJECT_PATH)/$(*D); \
	fi
ifdef CMISS_ROOT_DEFINED
	if [ -f $*.rc ]; then \
      SOURCE_RC=$*.rc; \
	else \
      SOURCE_RC=$(PRODUCT_SOURCE_PATH)/$*.rc; \
	fi ; \
	set -x ; \
	windres -o $(OBJECT_PATH)/$*.res -O coff $${SOURCE_RC}
else # CMISS_ROOT_DEFINED
	set -x ; \
   windres -o $(OBJECT_PATH)/$*.res -O coff $*.rc
endif # CMISS_ROOT_DEFINED)
endif # $(USER_INTERFACE) == WIN32_USER_INTERFACE

ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)

UID2UIDH = uid2uidh
UID2UIDH_BIN := $(wildcard $(UTILITIES_PATH)/$(UID2UIDH))
ifeq ($(UID2UID_BIN),)
   ifdef CMISS_ROOT_DEFINED
	   UID2UIDH_BIN := $(wildcard $(PRODUCT_UTILITIES_PATH)/$(UID2UIDH))
   endif # CMISS_ROOT_DEFINED
endif # $(UID2UIDH_BIN) ==
ifeq ($(UID2UIDH_BIN),)
   #If it still isn't found then assume we will build a local one.
   UID2UIDH_BIN := $(UTILITIES_PATH)/$(UID2UIDH)
else
   UID2UIDH_FOUND = true
endif

%.uidh : %.uil $(UID2UIDH_BIN)
	@if [ ! -d $(UIDH_PATH)/$(*D) ]; then \
		mkdir -p $(UIDH_PATH)/$(*D); \
	fi
	set -x ; \
	$(UIL) -o $(UIDH_PATH)/$*.uid $*.uil && \
	$(UID2UIDH_BIN) $(UIDH_PATH)/$*.uid $(UIDH_PATH)/$*.uidh
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

ifeq ($(SYSNAME:IRIX%=),)
   BUILD_OBJECT_LIST = cd $(OBJECT_PATH) ; (ls $(3) 2>&1 | sed "s%Cannot access %product_object/%;s%: No such file or directory%%;s%UX:ls: ERROR: %%" > $(1).list) ;
else # SYSNAME != IRIX%
   BUILD_OBJECT_LIST = cd $(OBJECT_PATH) ; (LC_ALL=C ls $(3) 2>&1 | sed "s%ls: %product_object/%;s%: No such file or directory%%" > $(1).list) ;
endif # SYSNAME ==/!= IRIX%

define BuildNormalTarget
	echo 'Building $(2)/$(1)'
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi
	if [ ! -d $(2) ]; then \
		mkdir -p $(2) ; \
	fi
	$(BUILD_OBJECT_LIST)
	cd $(OBJECT_PATH) ; \
	rm -f product_object ; \
	ln -s $(PRODUCT_OBJECT_PATH) product_object ; \
	$(LINK) -o $(1).tmp $(ALL_FLAGS) `cat $(1).list` $(4) && \
	mv $(1).tmp $(2)/$(1) ;
endef

define BuildStaticLibraryTarget
	echo 'Building static library $(2)/$(1)'
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi
	if [ ! -d $(dir $(2)/$(1)) ]; then \
		mkdir -p $(dir $(2)/$(1)) ; \
	fi
	$(BUILD_OBJECT_LIST)
	cd $(OBJECT_PATH) ; \
	rm -f product_object ; \
	ln -s $(PRODUCT_OBJECT_PATH) product_object ; \
	ar $(AR_FLAGS) cr $(1).tmp `cat $(1).list` && \
	mv $(1).tmp $(2)/$(1) ;
endef

define BuildSharedLibraryTarget
	echo 'Building shared library $(2)/$(1)'
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi
	if [ ! -d $(dir $(2)/$(1)) ]; then \
		mkdir -p $(dir $(2)/$(1)) ; \
	fi
	$(BUILD_OBJECT_LIST)
	cd $(OBJECT_PATH) ; \
	rm -f product_object ; \
	ln -s $(PRODUCT_OBJECT_PATH) product_object ; \
	$(LINK) -shared -o $(1).tmp $(ALL_FLAGS) `cat $(1).list` $(4) -Wl,-soname,$(5) && \
	mv $(1).tmp $(2)/$(1) ;
endef

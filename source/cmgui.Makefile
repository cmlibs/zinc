# **************************************************************************
# FILE : cmgui.Makefile
#
# LAST MODIFIED : 9 September 2003
#
# DESCRIPTION :
#
# Makefile for cmgui (the front end to CMISS)
# ==========================================================================

ifneq ($(WIN32_NATIVE_COMPILE),true)
   SHELL=/bin/sh
else # $(WIN32_NATIVE_COMPILE) != true
   SHELL=bash.exe
endif # $(WIN32_NATIVE_COMPILE) != true

ifndef CMGUI_DEV_ROOT
   CMGUI_DEV_ROOT = $(PWD:/source=)
endif # ! CMGUI_DEV_ROOT

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

SOURCE_PATH=$(CMGUI_DEV_ROOT)/source
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source

ifndef USER_INTERFACE
   USER_INTERFACE=MOTIF_USER_INTERFACE
endif

ifndef STATIC_LINK
   STATIC_LINK=false
endif

COMMONMAKEFILE := common.Makefile
COMMONMAKEFILE_FOUND = $(wildcard $(COMMONMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(COMMONMAKEFILE_FOUND),)
      COMMONMAKEFILE := $(PRODUCT_SOURCE_PATH)/$(COMMONMAKEFILE)
   endif # $(COMMONMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED
include $(COMMONMAKEFILE)

ifeq ($(filter CONSOLE_USER_INTERFACE GTK_USER_INTERFACE,$(USER_INTERFACE)),)
   UNEMAP = true
   LINK_CMISS = true
endif # $(USER_INTERFACE) != CONSOLE_USER_INTERFACE && $(USER_INTERFACE) != GTK_USER_INTERFACE
PERL_INTERPRETER = true
ifneq ($(SYSNAME),win32)
   IMAGEMAGICK = true
   USE_XML2 = true
endif # SYSNAME != win32
ifeq ($(SYSNAME),win32)
   ifeq ($(filter CONSOLE_USER_INTERFACE GTK_USER_INTERFACE,$(USER_INTERFACE)),)
      WIN32_USER_INTERFACE = true 
   endif # $(USER_INTERFACE) != CONSOLE_USER_INTERFACE && $(USER_INTERFACE) != GTK_USER_INTERFACE
endif # SYSNAME == win32

TARGET_EXECUTABLE_BASENAME = cmgui

ifneq ($(ABI),64)
   TARGET_ABI_SUFFIX =
else # $(ABI) != 64
   TARGET_ABI_SUFFIX = 64
endif # $(ABI) != 64

TARGET_USER_INTERFACE_SUFFIX :=
ifeq ($(USER_INTERFACE), CONSOLE_USER_INTERFACE)
   TARGET_USER_INTERFACE_SUFFIX := -console
endif # $(USER_INTERFACE) == CONSOLE_USER_INTERFACE
ifeq ($(USER_INTERFACE), GTK_USER_INTERFACE)
   TARGET_USER_INTERFACE_SUFFIX := -gtk
endif # $(USER_INTERFACE) == GTK_USER_INTERFACE

ifneq ($(STATIC_LINK),true)
   TARGET_STATIC_LINK_SUFFIX =
else # $(STATIC_LINK) != true
   TARGET_STATIC_LINK_SUFFIX = -static
endif # $(STATIC_LINK) != true 

ifneq ($(DEBUG),true)
   TARGET_DEBUG_SUFFIX =
else # $(DEBUG) != true
   TARGET_DEBUG_SUFFIX = -debug
endif # $(DEBUG) != true 

ifneq ($(MEMORYCHECK),true)
   TARGET_MEMORYCHECK_SUFFIX =
else # $(MEMORYCHECK) != true
   TARGET_MEMORYCHECK_SUFFIX = -memorycheck
endif # $(MEMORYCHECK) != true 

ifeq ($(SYSNAME:IRIX%=),)
   TARGET_FILETYPE_SUFFIX =
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   TARGET_FILETYPE_SUFFIX =
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   TARGET_FILETYPE_SUFFIX =
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   TARGET_FILETYPE_SUFFIX = .exe
endif # SYSNAME == win32
ifeq ($(SYSNAME),CYGWIN%=)
   TARGET_FILETYPE_SUFFIX = .exe
endif # SYSNAME == CYGWIN%=

BIN_PATH=$(CMGUI_DEV_ROOT)/bin/$(BIN_ARCH_DIR)

TARGET_SUFFIX = $(TARGET_ABI_SUFFIX)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_STATIC_LINK_SUFFIX)$(TARGET_DEBUG_SUFFIX)$(TARGET_MEMORYCHECK_SUFFIX)
BIN_TARGET = $(TARGET_EXECUTABLE_BASENAME)$(TARGET_SUFFIX)$(TARGET_FILETYPE_SUFFIX)
OBJECT_PATH=$(CMGUI_DEV_ROOT)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
PRODUCT_OBJECT_PATH=$(PRODUCT_PATH)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   UIDH_PATH=$(CMGUI_DEV_ROOT)/uidh/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)
   PRODUCT_UIDH_PATH=$(PRODUCT_PATH)/uidh/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

$(BIN_TARGET) :

ifdef MAKECMDGOALS
   define BUILDING_MESSAGE
Making $(MAKECMDGOALS) for version $(BIN_TARGET)

   endef
else
   define BUILDING_MESSAGE
Building $(BIN_TARGET) for $(LIB_ARCH_DIR)

   endef
endif
$(warning $(BUILDING_MESSAGE))

ifdef CMISS_ROOT_DEFINED
   VPATH=$(BIN_PATH):$(UTILITIES_PATH):$(OBJECT_PATH):$(UIDH_PATH):$(PRODUCT_SOURCE_PATH):$(PRODUCT_OBJECT_PATH):$(PRODUCT_UIDH_PATH)
else # CMISS_ROOT_DEFINED
   VPATH=$(BIN_PATH):$(UTILITIES_PATH):$(OBJECT_PATH):$(UIDH_PATH)
endif # CMISS_ROOT_DEFINED

SOURCE_DIRECTORY_INC = -I$(SOURCE_PATH) -I$(PRODUCT_SOURCE_PATH)

ifeq ($(SYSNAME:IRIX%=),)
   PLATFORM_DEFINES = -DSGI -Dmips -DCMGUI 
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   PLATFORM_DEFINES = -DGENERIC_PC -DCMGUI 
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   PLATFORM_DEFINES = -DAIX -DCMGUI 
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   PLATFORM_DEFINES = -DGENERIC_PC -DCMGUI 
   OPERATING_SYSTEM_DEFINES = -DWIN32_SYSTEM
endif # SYSNAME == win32
ifeq ($(SYSNAME:CYGWIN%=),)
   PLATFORM_DEFINES = -DGENERIC_PC -DCMGUI 
   OPERATING_SYSTEM_DEFINES = -DUNIX -DCYGWIN
endif # SYSNAME == CYGWIN%=

ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   USER_INTERFACE_DEFINES = -DMOTIF
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
ifeq ($(USER_INTERFACE), WIN32_USER_INTERFACE)
   USER_INTERFACE_DEFINES = -DWIN32_USER_INTERFACE
endif # $(USER_INTERFACE) == WIN32_USER_INTERFACE
ifeq ($(USER_INTERFACE), GTK_USER_INTERFACE)
   ifeq ($(SYSNAME),win32)
      # Our generic main loop is not implemented in Win32 so we use the GTK main step
      USER_INTERFACE_DEFINES = -DGTK_USER_INTERFACE -DUSE_GTK_MAIN_STEP
   else # $(SYSNAME) == win32
      USER_INTERFACE_DEFINES = -DGTK_USER_INTERFACE
   endif # $(SYSNAME) == win32
endif # $(USER_INTERFACE) == GTK_USER_INTERFACE
ifeq ($(USER_INTERFACE), CONSOLE_USER_INTERFACE)
  USER_INTERFACE = -DCONSOLE_USER_INTERFACE
endif # $(USER_INTERFACE) == CONSOLE_USER_INTERFACE

GRAPHICS_LIBRARY_DEFINES = -DOPENGL_API
GRAPHICS_LIB =
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   ifeq ($(SYSNAME:IRIX%=),)
      GRAPHICS_LIBRARY_DEFINES += -DDM_BUFFERS
      ifneq ($(ABI),64)
         GRAPHICS_LIBRARY_DEFINES += -DSGI_MOVIE_FILE -DSGI_DIGITAL_MEDIA
         GRAPHICS_LIB += -delay_load -lmoviefile -delay_load -ldmedia
      endif # ABI != 64
   endif # SYSNAME == IRIX%=
   ifeq ($(SYSNAME),Linux)
      GRAPHICS_LIBRARY_DEFINES += -DDM_BUFFERS
   endif # SYSNAME == Linux
   ifeq ($(SYSNAME),AIX)
      GRAPHICS_LIBRARY_DEFINES += -DDM_BUFFERS
   endif # SYSNAME == AIX
   ifeq ($(SYSNAME:CYGWIN%=),)
      GRAPHICS_LIBRARY_DEFINES += -DDM_BUFFERS
   endif # SYSNAME == CYGWIN%=
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

ifneq ($(USER_INTERFACE), GTK_USER_INTERFACE)
#For GTK_USER_INTERFACE the OpenGL comes from the GTK_LIBRARIES automatically
   ifeq ($(SYSNAME),Linux)
      GRAPHICS_LIB += -L$(firstword $(wildcard /usr/local/Mesa-5.0/lib /usr/X11R6/lib))
   endif # $(SYSNAME) == Linux
   ifeq ($(SYSNAME),win32)
      GRAPHICS_LIB += -L/usr/X11R6/lib
   endif # $(SYSNAME) == win32
   ifeq ($(SYSNAME),win32)
      GRAPHICS_LIB += -lopengl32 -lglu32
   else # $(SYSNAME) == win32
      GRAPHICS_LIB += -lGL -lGLU
   endif # $(SYSNAME) == win32 
endif # $(USER_INTERFACE) != GTK_USER_INTERFACE

ifndef LINK_CMISS
   CONNECTIVITY_DEFINES =
   WORMHOLE_LIB =
else # ! LINK_CMISS
   WORMHOLE_PATH=$(CMISS_ROOT)/wormhole
   WORMHOLE_INC = -I${WORMHOLE_PATH}/source
   CONNECTIVITY_DEFINES = -DLINK_CMISS
   # WORMHOLE_LIB = -L/usr/people/bullivan/wormhole/lib -lwormhole_n32
   # WORMHOLE_INC = -I/usr/people/bullivan/wormhole/source
   # WORMHOLE_LIB = -L/usr/people/bullivan/wormhole/lib -lwormhole
   # WORMHOLE_INC = -I/usr/people/bullivan/wormhole/source
   WORMHOLE_LIB = -L${WORMHOLE_PATH}/lib/$(LIB_ARCH_DIR) -lwormhole
endif # ! LINK_CMISS

ifndef IMAGEMAGICK
   IMAGEMAGICK_DEFINES =
   IMAGEMAGICK_LIB = 
else # ! IMAGEMAGICK
   IMAGEMAGICK_DEFINES += -DIMAGEMAGICK
   IMAGEMAGICK_PATH = $(CMISS_ROOT)/image_libraries
   IMAGEMAGICK_INC = -I$(IMAGEMAGICK_PATH)/include/$(LIB_ARCH_DIR)
   IMAGEMAGICK_LIB = $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libMagick.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libtiff.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libpng.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libjpeg.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libz.a
ifndef USE_XML2
   IMAGEMAGICK_LIB += $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libxml2.a
endif # USE_XML2

   ifeq ($(SYSNAME),AIX)
      IMAGEMAGICK_LIB += /usr/lib/libiconv.a
   endif # SYSNAME == AIX
endif # ! IMAGEMAGICK

MIRAGE_SRCS = \
	mirage/em_cmgui.c \
	slider/emoter_dialog.c

ifndef PERL_INTERPRETER
   INTERPRETER_INC =
   INTERPRETER_DEFINES = 
   INTERPRETER_SRCS =
   INTERPRETER_LIB =
else # ! PERL_INTERPRETER
   INTERPRETER_PATH = $(CMISS_ROOT)/perl_interpreter
   INTERPRETER_INC = -I$(INTERPRETER_PATH)/source/
   INTERPRETER_DEFINES = -DPERL_INTERPRETER
   INTERPRETER_SRCS =
   ifeq ($(SYSNAME),win32)
      INTERPRETER_LIB = $(INTERPRETER_PATH)/lib/$(LIB_ARCH_DIR)/libperlinterpreter-opt.a $(INTERPRETER_PATH)/lib/$(LIB_ARCH_DIR)/libPerl_cmiss.a $(INTERPRETER_PATH)/lib/$(LIB_ARCH_DIR)/libperl56.a
   else # $(SYSNAME) == win32
      INTERPRETER_LIB = $(INTERPRETER_PATH)/lib/$(LIB_ARCH_DIR)/libperlinterpreter.a
   endif # $(SYSNAME) == win32 
endif # ! PERL_INTERPRETER

ifndef UNEMAP
   UNEMAP_DEFINES =
   UNEMAP_SRCS =
else # ! UNEMAP
   # for all nodal stuff UNEMAP_DEFINES = -DUNEMAP -DSPECTRAL_TOOLS -DUNEMAP_USE_NODES 
   UNEMAP_DEFINES = -DUNEMAP -DSPECTRAL_TOOLS -DUNEMAP_USE_3D -DNOT_ACQUISITION_ONLY
   UNEMAP_SRCS = \
	   unemap/acquisition.c \
	   unemap/acquisition_window.c \
	   unemap/acquisition_work_area.c \
	   unemap/analysis.c \
	   unemap/analysis_calculate.c \
	   unemap/analysis_drawing.c \
	   unemap/analysis_window.c \
	   unemap/analysis_work_area.c \
	   unemap/bard.c \
	   unemap/beekeeper.c \
	   unemap/cardiomapp.c \
	   unemap/delaunay.c \
	   unemap/edf.c \
	   unemap/eimaging_time_dialog.c \
	   unemap/drawing_2d.c \
	   unemap/interpolate.c \
	   unemap/map_dialog.c \
	   unemap/mapping.c \
	   unemap/mapping_window.c \
	   unemap/neurosoft.c \
	   unemap/pacing_window.c \
	   unemap/page_window.c \
	   unemap/rig.c \
	   unemap/rig_node.c \
	   unemap/setup_dialog.c \
	   unemap/spectral_methods.c \
	   unemap/system_window.c \
	   unemap/trace_window.c \
	   unemap/unemap_command.c \
	   unemap/unemap_hardware_client.c \
	   unemap/unemap_package.c 
endif # ! UNEMAP

ifndef CELL
   CELL_DEFINES =
   CELL_SRCS =
else # ! CELL 
   CELL_DEFINES = -DCELL -DCELL_DISTRIBUTED
   CELL_SRCS = \
	   cell/cell_calculate.c \
	   cell/cell_calculate_dialog.c \
	   cell/cell_cmgui_interface.c \
	   cell/cell_cmiss_interface.c \
	   cell/cell_component.c \
	   cell/cell_export_dialog.c \
	   cell/cell_graphic.c \
	   cell/cell_input.c \
	   cell/cell_interface.c \
	   cell/cell_output.c \
	   cell/cell_plot.c \
	   cell/cell_unemap_interface.c \
	   cell/cell_variable.c \
	   cell/cell_variable_editing_dialog.c \
	   cell/cell_variable_unemap_interface.c \
      cell/cell_window.c \
      cell/distributed_editing_dialog.c \
      cell/distributed_editing_interface.c \
      cell/cell_model_routines/andre.f \
      cell/integrator_routines/euler.f \
      cell/integrator_routines/improved_euler.f \
      cell/integrator_routines/runge_kutta.f
endif # ! CELL 

ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   UIDH_INC = -I$(UIDH_PATH) -I$(PRODUCT_UIDH_PATH)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

EXTERNAL_INPUT_DEFINES =
EXTERNAL_INPUT_LIB = 
ifeq ($(SYSNAME:IRIX%=),)
   EXTERNAL_INPUT_DEFINES += -DDIALS -DSPACEBALL -DPOLHEMUS -DFARO -DEXT_INPUT -DSELECT_DESCRIPTORS
   EXTERNAL_INPUT_LIB += -lXext -lXi
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   EXTERNAL_INPUT_DEFINES = -DSELECT_DESCRIPTORS
endif # SYSNAME == Linux
ifeq ($(SYSNAME:CYGWIN%=),)
   EXTERNAL_INPUT_DEFINES = -DSELECT_DESCRIPTORS
endif # SYSNAME == CYGWIN%=

HAPTIC_LIB =
HAPTIC_INC =
# HAPTIC_LIB = -L/usr/local/phantom/GHOST/lib -lghost_n32Mips3
# HAPTIC_INC = -I/usr/local/phantom/GHOST/lib
# HAPTIC_LIB = -L/usr/local/phantom/GHOST/lib -lghost_o32
# HAPTIC_INC = -I/usr/local/phantom/GHOST/lib

# XML Stuff goes here!!

ifndef CELL
   XML_PATH =
   XML_DEFINES = 
   XML_LIB =
   XML_INC = 
else # ! CELL
   XML_PATH = $(CMISS_ROOT)/xml
   XML_DEFINES =
   ifeq ($(SYSNAME:IRIX%=),)
      XML_DEFINES += -DIRIX -D_REENTRANT
   endif # SYSNAME == IRIX%=
   ifeq ($(SYSNAME),Linux)
      XML_DEFINES = -DLINUX -D_REENTRANT
   endif # SYSNAME == Linux
   XML_LIB = $(XML_PATH)/library/$(LIB_ARCH_DIR)/libXML-optimised-1.0.0.a
   XML_INC = -I$(XML_PATH)/include
endif # ! CELL

ifndef USE_XML2
   XML2_PATH =
   XML2_DEFINES =
   XML2_INC =
   XML2_LIB =
else
   XML2_PATH = $(CMISS_ROOT)/image_libraries/
   XML2_DEFINES = -DHAVE_XML2
   XML2_INC = -I$(XML2_PATH)/include/$(LIB_ARCH_DIR)/libxml2/
   XML2_LIB = $(XML2_PATH)/lib/$(LIB_ARCH_DIR)/libxml2.a
endif

STEREO_DISPLAY_DEFINES = -DSTEREO
# STEREO_DISPLAY_DEFINES =

# POSTSCRIPT_DEFINES = -DFEEDBACK_POSTSCRIPT
POSTSCRIPT_DEFINES =

#  SHORT_NAMES were created to support OS's where external names <= 32 characters
#  NAME_DEFINES = -DSHORT_NAMES
NAME_DEFINES =

#  Temporary flags that are used during development
# TEMPORARY_DEVELOPMENT_FLAGS = -DCMGUI_REGIONS
# TEMPORARY_DEVELOPMENT_FLAGS = -DINTERNAL_UIDS -DPHANTOM_FARO
# TEMPORARY_DEVELOPMENT_FLAGS = -DINTERPOLATE_TEXELS -DOTHER_FIBRE_DIR -DREPORT_GL_ERRORS
# TEMPORARY_DEVELOPMENT_FLAGS = -DDO_NOT_ADD_DETAIL
# TEMPORARY_DEVELOPMENT_FLAGS = -DSELECT_ELEMENTS -DMERGE_TIMES -DPETURB_LINES -DDEPTH_OF_FIELD -DDO_NOT_ADD_DETAIL -DSGI_MOVIE_FILE
# TEMPORARY_DEVELOPMENT_FLAGS = -DSELECT_ELEMENTS -DMERGE_TIMES -DPETURB_LINES -DDEPTH_OF_FIELD -DBLEND_NODAL_VALUES -DNEW_SPECTRUM
# TEMPORARY_DEVELOPMENT_FLAGS = -DSELECT_ELEMENTS -DMERGE_TIMES -DPETURB_LINES -DDEPTH_OF_FIELD -DDO_NOT_ADD_DETAIL -DBLEND_NODAL_VALUES -DALLOW_MORPH_UNEQUAL_POINTSETS
# TEMPORARY_DEVELOPMENT_FLAGS = -DWINDOWS_DEV_FLAG

VIDEO_DEFINES =
VIDEO_LIB =
VIDEO_SRCS =

HELP_DEFINES = -DNETSCAPE_HELP
HELP_INCLUDE =
HELP_LIB =
HELP_SRCS = \
	help/help_interface.c

MEMORYCHECK_LIB = 
ifeq ($(SYSNAME:IRIX%=),)
   ifdef MEMORYCHECK
      MEMORYCHECK_LIB += -lmalloc_ss -lfpe 
   endif # MEMORYCHECK
endif # SYSNAME == IRIX%=

USER_INTERFACE_INC = 
USER_INTERFACE_LIB =
ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)
   ifeq ($(SYSNAME),Linux)
      ifneq ($(wildcard /usr/local/Mesa-5.0/include),)
         USER_INTERFACE_INC += -I/usr/local/Mesa-5.0/include
      endif
      USER_INTERFACE_INC += -I/usr/X11R6/include
      X_LIB = /usr/X11R6/lib
      USER_INTERFACE_LIB += -L$(X_LIB)
      #On Linux I am statically linking many of the libraries always to
      #reduce the version dependencies.

      #Mandrake 8.2 static libs are incompatible, this works around it by
      #comparing the size of the symbols and forcing Xmu to preload its
      #version if they differ in size.  Older greps don't have -o option.
      Xm_XeditRes = $(shell /usr/bin/objdump -t $(X_LIB)/libXm.a | /bin/grep '00000[0-f][0-f][1-f] _XEditResCheckMessages')
      Xmu_XeditRes = $(shell /usr/bin/objdump -t $(X_LIB)/libXmu.a | /bin/grep '00000[0-f][0-f][1-f] _XEditResCheckMessages')
      ifneq ($(Xm_XeditRes),)
         ifneq ($(Xmu_XeditRes),)
            ifneq ($(STATIC_LINK),true)
               USER_INTERFACE_LIB += -u _XEditResCheckMessages $(X_LIB)/libXmu.a
            else # STATIC_LINK != true
               USER_INTERFACE_LIB += -u _XEditResCheckMessages -lXmu 
            endif # STATIC_LINK != true
         endif
      endif
      ifneq ($(STATIC_LINK),true)
         USER_INTERFACE_LIB += $(X_LIB)/libMrm.a $(X_LIB)/libXm.a $(X_LIB)/libXt.a $(X_LIB)/libX11.a $(X_LIB)/libXmu.a $(X_LIB)/libXext.a $(X_LIB)/libXp.a $(X_LIB)/libSM.a $(X_LIB)/libICE.a
      else # STATIC_LINK != true
         USER_INTERFACE_LIB += -lMrm -lXm -lXp -lXt -lX11 -lXmu -lXext -lSM -lICE
      endif # STATIC_LINK != true
   else # SYSNAME == Linux
      ifeq ($(SYSNAME:CYGWIN%=),)
         USER_INTERFACE_INC += -I/usr/X11R6/include
         X_LIB = /usr/X11R6/lib
         USER_INTERFACE_LIB += -L$(X_LIB)
         USER_INTERFACE_LIB += -lMrm -lXm -lXp -lXt -lX11 -lXmu -lXext -lSM -lICE
      else # SYSNAME == CYGWIN%=
         USER_INTERFACE_LIB += -lMrm -lXm -lXt -lX11 -lXmu -lXext
         ifeq ($(SYSNAME:IRIX%=),)
            USER_INTERFACE_LIB += -lSgm
         endif # SYSNAME == IRIX%=
      endif # SYSNAME == CYGWIN%=
   endif # SYSNAME == Linux
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
ifeq ($(USER_INTERFACE),GTK_USER_INTERFACE)
   ifeq ($(SYSNAME),Linux)
      X_LIB = /usr/X11R6/lib
      USER_INTERFACE_LIB += -L$(X_LIB)
   endif
   ifeq ($(SYSNAME:CYGWIN%=),)
      X_LIB = /usr/X11R6/lib
      USER_INTERFACE_LIB += -L$(X_LIB)
   endif
   ifneq ($(SYSNAME),win32)
      USE_GTK2 = true
      ifeq ($(USE_GTK2),true)
         USER_INTERFACE_INC += $(shell pkg-config gtkglext-1.0 gtk+-2.0 --cflags)
         ifneq ($(STATIC_LINK),true)
            USER_INTERFACE_LIB += $(shell pkg-config gtkglext-1.0 gtk+-2.0 --libs)
         else # $(STATIC_LINK) != true
            USER_INTERFACE_LIB += -L/home/blackett/lib -lgtkglext-x11-1.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 -L/usr/local/Mesa-5.0/lib -lGLU -lGL
         endif # $(STATIC_LINK) != true
      else # $(USE_GTK2) == true
         USER_INTERFACE_INC +=  -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include/
         USER_INTERFACE_LIB +=  -lgtkgl -L/usr/local/Mesa-5.0/lib -lGLU -lGL -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11
      endif # $(USE_GTK2) == true
   else # $(SYSNAME) != win32
      # SAB It seems that ld currently requires (version 2.13.90 20021005) the 
      # perl_library to have a full filename alphabetically ahead of these gtk libs,
      # therefore be careful if you relocate these libraries perl_interpreter versus
      # win32_lib
      USER_INTERFACE_PATH = $(CMISS_ROOT)/win32lib/gtk2
      USER_INTERFACE_INC += -I$(USER_INTERFACE_PATH)/include/gtk-2.0 -I$(USER_INTERFACE_PATH)/include/pango-1.0 -I$(USER_INTERFACE_PATH)/include/glib-2.0/ -I$(USER_INTERFACE_PATH)/include/atk-1.0 -I$(USER_INTERFACE_PATH)/include/gtkgl-2.0/ -I$(USER_INTERFACE_PATH)/lib/glib-2.0/include -I$(USER_INTERFACE_PATH)/lib/gtk-2.0/include
      USER_INTERFACE_LIB += -L$(USER_INTERFACE_PATH)/lib -lgtkgl-2.0 -lgtk-win32-2.0.dll -lgdk-win32-2.0.dll -latk-1.0.dll -lgdk_pixbuf-2.0.dll -lpangowin32-1.0.dll -lpango-1.0.dll -lgobject-2.0.dll -lgmodule-2.0.dll -lglib-2.0.dll -lopengl32 -lglu32
      # USER_INTERFACE_INC = -L"c:\perl\5.6.1\lib\MSWin32-x86\CORE" -L"c:\dev\gcc\lib" -Ic:/dev/gtk2/include/gtk-2.0 -Ic:/dev/gtk2/include/pango-1.0 -Ic:/dev/gtk2/include/glib-2.0/ -Ic:/dev/gtk2/include/atk-1.0 -Ic:/dev/gtk2/include/gtkgl-2.0/ -Ic:/dev/gtk2/lib/glib-2.0/include -Ic:/dev/gtk2/lib/gtk-2.0/include
      # USER_INTERFACE_LIB += -L"c:\dev\gtk2\lib" -lgtkgl-2.0 -lgtk-win32-2.0 -lgdk-win32-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangowin32-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -lglib-2.0 */
   endif # $(SYSNAME) != win32
endif # $(USER_INTERFACE) == GTK_USER_INTERFACE

ifeq ($(SYSNAME:IRIX%=),)
   MATRIX_LIB = -lscs
else # ($(SYSNAME:IRIX%=),)
   MATRIX_LIB = -L$(CMISS_ROOT)/linear_solvers/lib/$(LIB_ARCH_DIR) -llapack-debug -lblas-debug
endif # ($(SYSNAME:IRIX%=),)
ifeq ($(SYSNAME),AIX)
   MATRIX_LIB += -lxlf90
endif # SYSNAME == AIX

ifeq ($(SYSNAME:IRIX%=),)
   LIB = -lPW -lftn -lm -lC -lCio -lpthread 
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifneq ($(STATIC_LINK),true)
      #For the dynamic link we really need to statically link the c++ as this
      #seems to be particularly variable between distributions.
      LIB = -lg2c -lm -ldl -lc -lpthread /usr/lib/libcrypt.a -lstdc++
   else # $(STATIC_LINK) != true
      LIB = -lg2c -lm -ldl -lpthread -lcrypt -lstdc++
   endif # $(STATIC_LINK) != true
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   LIB = -lm -ldl -lSM -lICE -lpthread -lcrypt -lbsd -lld -lC128
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   LIB = -lg2c -lgdi32  -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lwsock32 -lmpr -lwinmm -lversion -lodbc32 -lstdc++
endif # SYSNAME == win32

ALL_DEFINES = $(COMPILE_DEFINES) $(TARGET_TYPE_DEFINES) \
	$(PLATFORM_DEFINES) $(OPERATING_SYSTEM_DEFINES) $(USER_INTERFACE_DEFINES) \
	$(STEREO_DISPLAY_DEFINES) $(CONNECTIVITY_DEFINES) \
	$(EXTERNAL_INPUT_DEFINES) \
	$(GRAPHICS_LIBRARY_DEFINES) $(VIDEO_DEFINES) $(HELP_DEFINES) \
	$(POSTSCRIPT_DEFINES) $(NAME_DEFINES) $(TEMPORARY_DEVELOPMENT_FLAGS) \
	$(UNEMAP_DEFINES) \
	$(CELL_DEFINES) $(MOVIE_FILE_DEFINES) $(INTERPRETER_DEFINES)\
	$(IMAGEMAGICK_DEFINES) $(XML2_DEFINES)

ALL_INCLUDES = $(SOURCE_DIRECTORY_INC) $(HAPTIC_INC) $(WORMHOLE_INC) $(XML_INC) \
	$(UIDH_INC) $(USER_INTERFACE_INC) $(INTERPRETER_INC) $(IMAGEMAGICK_INC) \
	$(XML2_INC)

ALL_FLAGS = $(OPTIMISATION_FLAGS) $(COMPILE_FLAGS) $(TARGET_TYPE_FLAGS) \
	$(ALL_DEFINES) $(ALL_INCLUDES)

ALL_LIB = $(GRAPHICS_LIB) $(USER_INTERFACE_LIB) $(HAPTIC_LIB) \
	$(WORMHOLE_LIB) $(INTERPRETER_LIB) $(IMAGEMAGICK_LIB) \
	$(VIDEO_LIB) $(EXTERNAL_INPUT_LIB) $(HELP_LIB) \
	$(MOVIE_FILE_LIB) $(XML_LIB) $(XML2_LIB) $(MEMORYCHECK_LIB) $(MATRIX_LIB) \
	$(LIB)

API_SRCS = \
	api/cmiss_command.c \
	api/cmiss_core.c \
	api/cmiss_region.c \
	api/cmiss_scene_viewer.c \
	api/cmiss_value_derivative_matrix.c \
	api/cmiss_value_element_xi.c \
	api/cmiss_value_fe_value.c \
	api/cmiss_value_matrix.c \
	api/cmiss_variable.c \
	api/cmiss_variable_composite.c \
	api/cmiss_variable_composition.c \
	api/cmiss_variable_coordinates.c \
	api/cmiss_variable_derivative.c \
	api/cmiss_variable_finite_element.c \
	api/cmiss_variable_identity.c \
	api/cmiss_variable_new.cpp \
	api/cmiss_variable_new_basic.cpp
API_INTERFACE_SRCS = \
	api/cmiss_graphics_window.c
CHOOSE_INTERFACE_SRCS = \
	choose/choose_computed_field.c \
	choose/choose_control_curve.c \
	choose/choose_enumerator.c \
	choose/choose_fe_field.c \
	choose/choose_field_component.c \
	choose/choose_graphical_material.c \
	choose/choose_gt_object.c \
	choose/choose_scene.c \
	choose/choose_spectrum.c \
	choose/choose_texture.c \
	choose/choose_volume_texture.c \
	choose/chooser.c \
	choose/text_choose_fe_element.c \
	choose/text_choose_fe_node.c
COLOUR_INTERFACE_SRCS = \
	colour/colour_editor.c \
	colour/edit_var.c 
COMFILE_SRCS = \
	comfile/comfile.c 
COMFILE_INTERFACE_SRCS = \
	comfile/comfile_window.c 
COMMAND_SRCS = \
	command/cmiss.c \
	command/command.c \
	command/console.c \
	command/example_path.c \
	command/parser.c
COMMAND_INTERFACE_SRCS = \
	command/command_window.c
COMPUTED_FIELD_SRCS = \
	computed_field/computed_field.c \
	computed_field/computed_field_component_operations.c \
	computed_field/computed_field_compose.c \
	computed_field/computed_field_composite.c \
	computed_field/computed_field_control_curve.c \
	computed_field/computed_field_coordinate.c \
	computed_field/computed_field_deformation.c \
	computed_field/computed_field_derivatives.c \
	computed_field/computed_field_external.c \
	computed_field/computed_field_fibres.c \
	computed_field/computed_field_find_xi.c \
	computed_field/computed_field_finite_element.c \
	computed_field/computed_field_integration.c \
	computed_field/computed_field_matrix_operations.c \
	computed_field/computed_field_sample_texture.c \
	computed_field/computed_field_set.c \
	computed_field/computed_field_time.c \
	computed_field/computed_field_update.c \
	computed_field/computed_field_value_index_ranges.c \
	computed_field/computed_field_vector_operations.c \
	computed_field/computed_field_wrappers.c
COMPUTED_FIELD_INTERFACE_SRCS = \
	computed_field/computed_field_window_projection.c
COMPUTED_VARIABLE_SRCS = \
	computed_variable/computed_value.c \
	computed_variable/computed_value_derivative_matrix.c \
	computed_variable/computed_value_fe_value.c \
	computed_variable/computed_value_finite_element.c \
	computed_variable/computed_value_matrix.c \
	computed_variable/computed_variable.c \
	computed_variable/computed_variable_composite.c \
	computed_variable/computed_variable_composition.c \
	computed_variable/computed_variable_coordinates.c \
	computed_variable/computed_variable_derivative.c \
	computed_variable/computed_variable_finite_element.c \
	computed_variable/computed_variable_identity.c \
	computed_variable/computed_variable_standard_operations.c \
	computed_variable/variable.cpp \
	computed_variable/variable_basic.cpp
CURVE_SRCS = \
	curve/control_curve.c
CURVE_INTERFACE_SRCS = \
	curve/control_curve_editor.c \
	curve/control_curve_editor_dialog.c
DOF3_INTERFACE_SRCS = \
	dof3/dof3.c \
	dof3/dof3_control.c \
	dof3/dof3_input.c
ELEMENT_SRCS = \
   element/element_operations.c
ELEMENT_INTERFACE_SRCS = \
	element/element_creator.c \
	element/element_point_field_viewer_widget.c \
	element/element_point_tool.c \
	element/element_point_viewer.c \
	element/element_point_viewer_widget.c \
	element/element_tool.c
FINITE_ELEMENT_SRCS = \
	finite_element/export_finite_element.c \
	finite_element/finite_element.c \
	finite_element/finite_element_adjacent_elements.c \
	finite_element/finite_element_discretization.c \
	finite_element/finite_element_region.c \
	finite_element/finite_element_time.c \
	finite_element/finite_element_to_graphics_object.c \
	finite_element/finite_element_to_iges.c \
	finite_element/finite_element_to_iso_lines.c \
	finite_element/finite_element_to_streamlines.c \
	finite_element/import_finite_element.c \
	finite_element/read_fieldml.c \
	finite_element/snake.c \
	finite_element/write_fieldml.c
FINITE_ELEMENT_INTERFACE_SRCS = \
	finite_element/grid_field_calculator.c
GENERAL_SRCS = \
	general/any_object.c \
	general/callback.c \
	general/child_process.c \
	general/compare.c \
	general/debug.c \
	general/error_handler.c \
	general/geometry.c \
	general/heapsort.c \
	general/image_utilities.c \
	general/indexed_multi_range.c \
	general/integration.c \
	general/machine.c \
	general/matrix_vector.c \
	general/multi_range.c \
	general/myio.c \
	general/mystring.c \
	general/photogrammetry.c \
	general/statistics.c \
	general/time.c \
	general/value.c 
GENERAL_INTERFACE_SRCS = \
	general/postscript.c
GRAPHICS_SRCS = \
	graphics/auxiliary_graphics_types.c \
	graphics/colour.c \
	graphics/complex.c \
	graphics/defined_graphics_objects.c \
	graphics/element_group_settings.c \
	graphics/element_point_ranges.c \
	graphics/environment_map.c \
	graphics/glyph.c \
	graphics/graphical_element.c \
	graphics/graphics_library.c \
	graphics/graphics_object.c \
	graphics/import_graphics_object.c \
	graphics/iso_field_calculation.c \
	graphics/laguer.c \
	graphics/light.c \
	graphics/light_model.c \
	graphics/material.c \
	graphics/makegtobj.c \
	graphics/mcubes.c \
	graphics/order_independent_transparency.c \
	graphics/renderbinarywavefront.c \
	graphics/rendergl.c \
	graphics/rendervrml.c \
	graphics/renderwavefront.c \
	graphics/scene.c \
	graphics/scene_viewer.c \
	graphics/selected_graphic.c \
	graphics/spectrum.c \
	graphics/spectrum_settings.c \
	graphics/texture.c \
	graphics/texture_line.c \
	graphics/transform_tool.c \
	graphics/userdef_objects.c \
	graphics/volume_texture.c
GRAPHICS_INTERFACE_SRCS = \
	graphics/graphical_element_editor.c \
	graphics/graphics_window.c \
	graphics/movie_graphics.c \
	graphics/scene_editor.c \
	graphics/settings_editor.c \
	graphics/spectrum_editor.c \
	graphics/spectrum_editor_dialog.c \
	graphics/spectrum_editor_settings.c \
	graphics/texture_graphics.c \
	graphics/texturemap.c \
	graphics/volume_texture_editor.c \
	graphics/volume_texture_editor_dialog.c
INTERACTION_SRCS = \
	interaction/interaction_graphics.c \
	interaction/interaction_volume.c \
	interaction/interactive_event.c \
	interaction/interactive_tool.c
INTERACTION_INTERFACE_SRCS = \
	interaction/interactive_toolbar_widget.c \
	interaction/select_tool.c
IO_DEVICES_SRCS = \
	io_devices/conversion.c \
	io_devices/io_device.c \
	io_devices/matrix.c 
IO_DEVICES_INTERFACE_SRCS = \
	io_devices/haptic_input_module.cpp \
	io_devices/input_module.c \
	io_devices/input_module_dialog.c \
	io_devices/input_module_widget.c
ifdef LINK_CMISS
   LINK_INTERFACE_SRCS =  \
	   link/cmiss.c
else # LINK_CMISS
   LINK_INTERFACE_SRCS =
endif # LINK_CMISS
MATERIAL_INTERFACE_SRCS =  \
	material/material_editor.c \
	material/material_editor_dialog.c
MATRIX_SRCS =  \
	matrix/factor.c \
	matrix/matrix.c \
	matrix/matrix_blas.c
MENU_INTERFACE_SRCS =  \
	menu/menu_window.c
MOTIF_INTERFACE_SRCS =  \
	motif/image_utilities.c
NODE_SRCS = \
	node/node_operations.c \
	node/node_tool.c
NODE_INTERFACE_SRCS = \
	node/node_field_viewer_widget.c \
	node/node_viewer.c \
	node/node_viewer_widget.c
PROMPT_INTERFACE_SRCS =\
	prompt/prompt_window.c
REGION_SRCS = \
   region/cmiss_region.c \
   region/cmiss_region_write_info.c
REGION_INTERFACE_SRCS = \
   region/cmiss_region_chooser.c
SELECT_INTERFACE_SRCS = \
	select/select_control_curve.c \
	select/select_environment_map.c \
	select/select_graphical_material.c \
	select/select_private.c \
	select/select_spectrum.c
SELECTION_SRCS = \
	selection/any_object_selection.c \
	selection/element_point_ranges_selection.c \
	selection/element_selection.c \
	selection/node_selection.c
THREE_D_DRAWING_SRCS = \
	three_d_drawing/graphics_buffer.c
THREE_D_DRAWING_INTERFACE_SRCS = \
	three_d_drawing/dm_interface.c \
	three_d_drawing/movie_extensions.c \
	three_d_drawing/ThreeDDraw.c
TIME_SRCS = \
	time/time.c \
	time/time_keeper.c
TIME_INTERFACE_SRCS = \
	time/time_editor.c \
	time/time_editor_dialog.c
TRANSFORMATION_INTERFACE_SRCS = \
	transformation/transformation_editor.c
USER_INTERFACE_SRCS = \
	user_interface/confirmation.c \
	user_interface/event_dispatcher.c \
	user_interface/filedir.c \
	user_interface/message.c \
	user_interface/user_interface.c
USER_INTERFACE_INTERFACE_SRCS = \
	user_interface/call_work_procedures.c \
	user_interface/printer.c
VIEW_INTERFACE_SRCS = \
	view/camera.c \
	view/coord.c \
	view/coord_trans.c \
	view/poi.c \
	view/vector.c \
	view/view.c \
	view/view_control.c 

# DB.  makedepend has problems if too many files
SRCS_1 = \
	$(API_SRCS) \
	$(COMFILE_SRCS) \
	$(COMMAND_SRCS) \
	$(COMPUTED_VARIABLE_SRCS) \
	$(COMPUTED_FIELD_SRCS) \
	$(CURVE_SRCS) \
	$(ELEMENT_SRCS) \
	$(FINITE_ELEMENT_SRCS) \
	$(GENERAL_SRCS) \
	$(GRAPHICS_SRCS) \
	$(HELP_SRCS) \
	$(INTERACTION_SRCS) \
	$(IO_DEVICES_SRCS) \
	$(MATRIX_SRCS) \
	$(NODE_SRCS) \
	$(REGION_SRCS) \
	$(SELECTION_SRCS) \
	$(INTERPRETER_SRCS) \
	$(THREE_D_DRAWING_SRCS) \
	$(TIME_SRCS) \
	$(USER_INTERFACE_SRCS)

ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)
   SRCS_2 = \
	   $(API_INTERFACE_SRCS) \
	   $(CELL_SRCS) \
	   $(CHOOSE_INTERFACE_SRCS) \
	   $(COLOUR_INTERFACE_SRCS) \
	   $(COMFILE_INTERFACE_SRCS) \
	   $(COMMAND_INTERFACE_SRCS) \
	   $(COMPUTED_FIELD_INTERFACE_SRCS) \
	   $(CURVE_INTERFACE_SRCS) \
	   $(DOF3_INTERFACE_SRCS) \
	   $(ELEMENT_INTERFACE_SRCS) \
	   $(FINITE_ELEMENT_INTERFACE_SRCS) \
	   $(GENERAL_INTERFACE_SRCS) \
	   $(GRAPHICS_INTERFACE_SRCS) \
	   $(INTERACTION_INTERFACE_SRCS) \
	   $(IO_DEVICES_INTERFACE_SRCS) \
	   $(LINK_INTERFACE_SRCS) \
	   $(MATERIAL_INTERFACE_SRCS) \
	   $(MENU_INTERFACE_SRCS) \
		$(MIRAGE_SRCS) \
	   $(MOTIF_INTERFACE_SRCS) \
	   $(NODE_INTERFACE_SRCS) \
	   $(PROMPT_INTERFACE_SRCS) \
	   $(REGION_INTERFACE_SRCS) \
	   $(SELECT_INTERFACE_SRCS) \
	   $(SLIDER_INTERFACE_SRCS) \
	   $(THREE_D_DRAWING_INTERFACE_SRCS) \
	   $(TIME_INTERFACE_SRCS) \
	   $(TRANSFORMATION_INTERFACE_SRCS) \
	   $(UNEMAP_SRCS) \
	   $(USER_INTERFACE_INTERFACE_SRCS) \
	   $(VIDEO_SRCS) \
	   $(VIEW_INTERFACE_SRCS)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
ifeq ($(USER_INTERFACE),GTK_USER_INTERFACE)
      SRCS_2 = \
	      $(API_INTERFACE_SRCS) \
	      $(COMMAND_INTERFACE_SRCS) \
	      $(COMPUTED_FIELD_INTERFACE_SRCS) \
	      graphics/graphics_window.c \
	      gtk/gtk_cmiss_scene_viewer.c
endif # $(USER_INTERFACE) == GTK_USER_INTERFACE
ifeq ($(USER_INTERFACE),WIN32_USER_INTERFACE)
   SRCS_2 = \
	   $(COMMAND_INTERFACE_SRCS)
endif # $(USER_INTERFACE) == WIN32_USER_INTERFACE

SRCS = $(SRCS_1) $(SRCS_2)

ifneq ($(MEMORYCHECK),true)
   OBJSA = $(SRCS:.c=.o)
   OBJSB = $(OBJSA:.cpp=.o)
   OBJS = $(OBJSB:.f=.o)
else # $(MEMORYCHECK) != true
   OBJSA = $(SRCS:.c=.o)
   OBJSB = $(OBJSA:.cpp=.o)
   # Override the one changed file in the source list 
   OBJSC = $(OBJSB:general/debug.o=general/debug_memory_check.o)
   OBJS = $(OBJSC:.f=.o)
endif # $(MEMORYCHECK) != true

MAIN_SRC = cmgui.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

clean :
	-rm -r $(OBJECT_PATH)
ifdef UIDH_PATH
		-rm -r $(UIDH_PATH)
endif # UIDH_PATH

clobber : clean
	-rm $(BIN_PATH)/Target

transfer :
	tar -cvf - \
	../*.make readme.* *.make *.imake *.c version.o.h Cmgui \
	*.mms *.sed *.com *.opt *.ico \
	application/*.c application/*.h application/*.uil \
	choose/*.c choose/*.h choose/*.uil \
	colour/*.c colour/*.h colour/*.uil \
	comfile/*.c comfile/*.h comfile/*.rc comfile/*.uil \
	computed_field/*.c computed_field/*.h computed_field/*.rc computed_field/*.uil \
	computed_variable/*.c computed_variable/*.h computed_variable/*.rc computed_variable/*.uil \
	curve/*.c curve/*.h curve/*.uil \
	data/*.c data/*.h data/*.uil \
	database/*.c database/*.h database/*.uil \
	dof3/*.c dof3/*.h dof3/*.uil \
	element/*.c element/*.h element/*.uil \
	general/*.c general/*.h general/*.uil \
	help/*.c help/*.h help/*.uil \
	interaction/*.c interaction/*.cpp interaction/*.h interaction/*.uil \
	io_devices/*.c io_devices/*.cpp io_devices/*.h io_devices/*.uil \
	link/*.c link/*.h link/*.uil \
	material/*.c material/*.h material/*.uil \
	menu/*.c menu/*.h menu/*.uil \
	mirage/*.c mirage/*.h mirage/*.uil \
	node/*.c node/*.h node/*.uil \
	projection/*.c projection/*.h projection/*.uil \
	prompt/*.c prompt/*.h prompt/*.uil \
	select/*.c select/*.h select/*.uil \
	selection/*.c selection/*.h selection/*.uil \
	slider/*.c slider/*.h slider/*.uil \
	three_d_drawing/Makefile three_d_drawing/*.c three_d_drawing/*.h \
	time/*.c time/*.h time/*.uil \
	transformation/*.c transformation/*.h transformation/*.uil \
	unemap/drawing_2d.c unemap/drawing_2d.h \
	user_interface/*.c user_interface/*.h user_interface/*.uil \
	utilities/*.c utilities/*.h utilities/*.uil \
	view/*.c view/*.h view/*.uil \
	| gzip > cmgui_tar1.gz
	tar -cvf - \
	cell/*.c cell/*.h cell/*.uil \
	cell/cell_model_routines/*.f cell/cell_model_routines/*.inc \
	cell/integrator_routines/*.f \
	hypertext_help/*.c hypertext_help/*.h hypertext_help/Makefile \
	hypertext_help/*.mms \
	unemap/*.c unemap/*.h unemap/*.uil unemap/*.rc \
	unemap/utilities/Makefile unemap/utilities/*.c unemap/utilities/*.h  \
	unemap/utilities/*.uil unemap/utilities/*.rc \
	unemap/default_torso/*.ex* \
	unemap_hardware_service/*.c unemap_hardware_service/*.h \
	unemap_hardware_service/*.uil unemap_hardware_service/*.rc \
	xvg/README xvg/custom/*.c xvg/custom/*.f xvg/include/*.h xvg/cmgui/*.uil \
	xvg/cmgui/*.c xvg/cmgui/*.px xvg/cmgui/*.h \
	| gzip > cmgui_tar2.gz
	tar -cvf - \
	command/*.c command/*.h command/*.rc command/*.uil \
	finite_element/*.c finite_element/*.h finite_element/*.uil \
	graphics/Makefile graphics/*.c graphics/*.h graphics/*.rc graphics/*.uil \
	graphics/*.cpp graphics/marchg.dat graphics/mcubes.dat \
	html_widget/*.xbm html_widget/*.c html_widget/*.h html_widget/Makefile \
	html_widget/*.mms \
	socket/*.c socket/*.h \
	| gzip > cmgui_tar3.gz
	tar -cvf - \
	$(XML_PATH)/expat_lib/lib/* $(XML_PATH)/expat_lib/include/*.h \
	$(UTILITIES_PATH)/uid2uidh $(UTILITIES_PATH)/uid2uidh_linux \
	$(WORMHOLE_PATH)/lib/* $(WORMHOLE_PATH)/source/wormhole.h \
	$(INTERPRETER_PATH)/source/*.c $(INTERPRETER_PATH)/source/*.h \
	$(INTERPRETER_PATH)/source/*.f90 $(INTERPRETER_PATH)/source/*.pm \
	| gzip > cmgui_tar4.gz
	tar -cvf - \
	$(INTERPRETER_PATH)/generated/mips3-n32-debug/*.a \
	| gzip > cmgui_tar5.gz

compare :
	mv cmgui_tar tmp
	(cd tmp; tar -xvf cmgui_tar > tar.list 2>&1; \
sed "s/./diff/;s/,.*/ >> out.diff/;h;G;G;s/\n//2;s%>> out\.diffdiff %tmp/%;s/diff/echo FILE:/" \
<tar.list >compare.script; rm tar.list)
	mv tmp/cmgui_tar .
	sh tmp/compare.script

descrip.mms : Makefile descrip_mms.sed
	sed -f descrip_mms.sed < Makefile > descrip.mms

$(OBJECT_PATH)/version.o.h : $(OBJS) cmgui.Makefile
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi	
	echo '/* This is a generated file.  Do not edit.  Edit cmgui.c or cmgui.imake instead */' > $(OBJECT_PATH)/version.o.h;	  
	date > date.h
	sed 's/"//;s/./#define VERSION "CMISS(cmgui) version 001.001.018  &/;s/.$$/&\\nCopyright 1996-2002, Auckland UniServices Ltd."/' < date.h >> $(OBJECT_PATH)/version.o.h
#	sed 's/"//;s/./"CMISS(cmgui) version 001.001.018  &/;s/.$$/&\\nCopyright 1996-1998, Auckland UniServices Ltd."/' < date.h >> $(OBJECT_PATH)/version.o.h
# 	sed 's/./#define VERSION "CMISS(cmgui) version 001.001.018  &/;s/.$$/&\\nCopyright 1996-1998, Auckland UniServices Ltd."/' < date.h >> $(OBJECT_PATH)/version.o.h

$(MAIN_OBJ) : $(MAIN_SRC) $(OBJECT_PATH)/version.o.h $(INTERPRETER_LIB)
	@if [ -f $*.c ]; then \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $*.c > $(OBJECT_PATH)/$*.o.c ; \
	else \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $(PRODUCT_SOURCE_PATH)/$*.c > $(OBJECT_PATH)/$*.o.c ; \
	fi
	$(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(OBJECT_PATH)/$*.o.c;

# We want to limit the dynamic symbols in the final executable so that
# dlopen routines can't access symbols we don't want them to */
ifeq ($(SYSNAME:IRIX%=),)
   ifneq ($(EXPORT_EVERYTHING),true)
      # The .link extension prevents the makefile predecessor cycle 
      EXPORTS_FILE = cmgui.exports.link
      SRC_EXPORTS_FILE = cmgui.exports
      EXPORTS_LINK_FLAGS = -Wl,-exports_file,$(EXPORTS_FILE)
      $(OBJECT_PATH)/$(EXPORTS_FILE) :	$(SRC_EXPORTS_FILE)
			@if [ ! -d $(OBJECT_PATH) ]; then \
				mkdir -p $(OBJECT_PATH); \
			fi
			@if [ -f $(OBJECT_PATH)/$(EXPORTS_FILE).c ]; then \
				rm $(OBJECT_PATH)/$(EXPORTS_FILE).c; \
			fi
      ifdef CMISS_ROOT_DEFINED
			if [ -f $(SRC_EXPORTS_FILE) ]; then \
				cp $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(EXPORTS_FILE).c; \
			else \
				cp $(PRODUCT_SOURCE_PATH)/$(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(EXPORTS_FILE).c; \
			fi
      else # CMISS_ROOT_DEFINED
			cp $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(EXPORTS_FILE).c
      endif # CMISS_ROOT_DEFINED
			cd $(OBJECT_PATH) ; $(CPREPROCESS) $(ALL_DEFINES) $(EXPORTS_FILE).c
			cp $(OBJECT_PATH)/$(EXPORTS_FILE).i $(OBJECT_PATH)/$(EXPORTS_FILE)
   else # $(EXPORT_EVERYTHING) != true
      EXPORTS_FILE = no.exports
      EXPORTS_LINK_FLAGS = 
      $(OBJECT_PATH)/$(EXPORTS_FILE) :	
			touch $(OBJECT_PATH)/$(EXPORTS_FILE)
   endif # $(EXPORT_EVERYTHING) != true
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifneq ($(STATIC_LINK),true)
      ifneq ($(EXPORT_EVERYTHING),true)
#         NEW_BINUTILS=true
         ifdef NEW_BINUTILS
            # In Linux this requires an ld from binutils 2.12 or later 
            #   but is much better code so hopefully what I do here will
            #   replace the alternative when this version is ubiquitous */
            # Use a version script to reduce all variables not listed to
            #   local scope, this version script would allow regexp but
            #   the irix file does not yet */
            EXPORTS_FILE = cmgui_ld_version.script
            SRC_EXPORTS_FILE = cmgui.exports
            EXPORTS_LINK_FLAGS = -Wl,-export-dynamic,--version-script,$(EXPORTS_FILE)
            $(OBJECT_PATH)/$(EXPORTS_FILE) :	$(SRC_EXPORTS_FILE)
					@if [ ! -d $(OBJECT_PATH) ]; then \
						mkdir -p $(OBJECT_PATH); \
					fi
					@if [ -f $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c ]; then \
						rm $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					fi
            ifdef CMISS_ROOT_DEFINED
					if [ -f $(SRC_EXPORTS_FILE) ]; then \
						cp -f $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					else \
						cp -f $(PRODUCT_SOURCE_PATH)/$(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					fi
            else # CMISS_ROOT_DEFINED
					cp -f $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c
            endif # CMISS_ROOT_DEFINED
				$(CPREPROCESS) $(ALL_DEFINES) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c -o $(OBJECT_PATH)/$(EXPORTS_FILE).E
				echo > $(OBJECT_PATH)/$(EXPORTS_FILE)	"CMISS_EXP_1.0 {"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE)	"global:"
				sed -n -e 's/\([^ ]\+\)/  \1;/p' < $(OBJECT_PATH)/$(EXPORTS_FILE).E >> $(OBJECT_PATH)/$(EXPORTS_FILE)
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE)	"local:"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE)	"  *;"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE)	"};"
         else # NEW_BINUTILS
            # So this is the nastier fix which works until ld is fixed,
            # basically I create a shared object which requires the 
            # symbols we want to export and link against that, then
            # to remove the dependency on the shared object we just
            # linked against, we declare that the executable has the
            # soname of the library, pretty sneaky */
            EXPORTS_FILE = cmgui_symbols.so
            EXPORTS_SONAME = libcmgui.so # If we are going to put this in the executable lets give it a generic name 
            SRC_EXPORTS_FILE = cmgui.exports
            EXPORTS_LINK_FLAGS = $(EXPORTS_FILE) -Wl,-soname,$(EXPORTS_SONAME)
            $(OBJECT_PATH)/$(EXPORTS_FILE) :	$(SRC_EXPORTS_FILE)
					@if [ ! -d $(OBJECT_PATH) ]; then \
						mkdir -p $(OBJECT_PATH); \
					fi
					@if [ -f $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c ]; then \
						rm $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					fi
            ifdef CMISS_ROOT_DEFINED
					if [ -f $(SRC_EXPORTS_FILE) ]; then \
						cp -f $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					else \
						cp -f $(PRODUCT_SOURCE_PATH)/$(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c; \
					fi
            else # CMISS_ROOT_DEFINED
					cp -f $(SRC_EXPORTS_FILE) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c
            endif # CMISS_ROOT_DEFINED
				$(CPREPROCESS) $(ALL_DEFINES) $(OBJECT_PATH)/$(SRC_EXPORTS_FILE).list.c -o $(OBJECT_PATH)/$(EXPORTS_FILE).E
				echo > $(OBJECT_PATH)/$(EXPORTS_FILE).c	"#define lt_preloaded_symbols some_other_symbol"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE).c	"#define dynamic_ptr void *"
				sed -n -e 's/\([^ ]\+\)/extern char \1;/p' < $(OBJECT_PATH)/$(EXPORTS_FILE).E >> $(OBJECT_PATH)/$(EXPORTS_FILE).c
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE).c	"#undef lt_preloaded_symbols"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE).c	"const struct { const char *name; dynamic_ptr address; }"
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE).c	"lt_preloaded_symbols[] = {"
				sed -n -e 's/\([^ ]\+\)/  {\"\1\", (dynamic_ptr) \&\1},/p' < $(OBJECT_PATH)/$(EXPORTS_FILE).E >> $(OBJECT_PATH)/$(EXPORTS_FILE).c
				echo >> $(OBJECT_PATH)/$(EXPORTS_FILE).c	"{0, (dynamic_ptr) 0} };"
				$(LINK) -shared -o $(OBJECT_PATH)/$(EXPORTS_FILE) -Wl,-soname,$(EXPORTS_SONAME) $(OBJECT_PATH)/$(EXPORTS_FILE).c
         endif # NEW_BINUTILS
      else # $(EXPORT_EVERYTHING) != true
         EXPORTS_FILE = cmgui.dummy
         SRC_EXPORTS_FILE = cmgui.exports
         EXPORTS_LINK_FLAGS = -Wl,-E
         $(OBJECT_PATH)/$(EXPORTS_FILE) :	$(SRC_EXPORTS_FILE)
		   	touch $(OBJECT_PATH)/$(EXPORTS_FILE)
      endif # $(EXPORT_EVERYTHING) != true
   else # STATIC_LINK) != true 
      EXPORTS_FILE = no.exports
      EXPORTS_LINK_FLAGS = 
      $(OBJECT_PATH)/$(EXPORTS_FILE) :	
			touch $(OBJECT_PATH)/$(EXPORTS_FILE)
   endif # STATIC_LINK) != true
endif # SYSNAME == Linux

RESOURCE_FILES = 
COMPILED_RESOURCE_FILES = 
ifeq ($(SYSNAME),win32)
   ifneq ($(USER_INTERFACE),GTK_USER_INTERFACE)
      RESOURCE_FILES += command/command_window.rc
      COMPILED_RESOURCE_FILES += $(RESOURCE_FILES:.rc=.res)
   endif # $(USER_INTERFACE) != GTK_USER_INTERFACE
endif # $(SYSNAME) == win32

$(BIN_TARGET) : $(OBJS) $(COMPILED_RESOURCE_FILES) $(MAIN_OBJ) $(OBJECT_PATH)/$(EXPORTS_FILE)
	$(call BuildNormalTarget,$(BIN_TARGET),$(BIN_PATH),$(OBJS) $(MAIN_OBJ),$(ALL_LIB) $(EXPORTS_LINK_FLAGS) $(COMPILED_RESOURCE_FILES))

SO_LIB_SUFFIX = .so
ifeq ($(SYSNAME:IRIX%=),)
   ALL_SO_LINK_FLAGS = -no_unresolved
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ALL_SO_LINK_FLAGS = -lgcc_s -Wl,--no-undefined 
#   ALL_SO_LINK_FLAGS =
endif # SYSNAME == Linux

SO_LIB_GENERAL = cmgui_general
SO_LIB_GENERAL_TARGET = lib$(SO_LIB_GENERAL)$(TARGET_SUFFIX)$(SO_LIB_SUFFIX)
SO_LIB_GENERAL_SONAME = lib$(SO_LIB_GENERAL)$(SO_LIB_SUFFIX)
LIB_GENERAL_SRCS = \
	api/cmiss_core.c \
	command/parser.c \
	general/any_object.c \
	general/compare.c \
	general/debug.c \
	general/geometry.c \
	general/heapsort.c \
	general/matrix_vector.c \
	general/multi_range.c \
	general/myio.c \
	general/mystring.c \
	general/value.c \
	user_interface/message.c
LIB_GENERAL_OBJS = $(addsuffix .o,$(basename $(LIB_GENERAL_SRCS)))
$(SO_LIB_GENERAL_TARGET) : $(LIB_GENERAL_OBJS) cmgui.Makefile
	$(call BuildSharedLibraryTarget,$(SO_LIB_GENERAL_TARGET),$(BIN_PATH),$(LIB_GENERAL_OBJS),$(ALL_SO_LINK_FLAGS) $(LIB),$(SO_LIB_GENERAL_SONAME))

SO_LIB_FINITE_ELEMENT = cmgui_finite_element
SO_LIB_FINITE_ELEMENT_TARGET = lib$(SO_LIB_FINITE_ELEMENT)$(TARGET_SUFFIX)$(SO_LIB_SUFFIX)
SO_LIB_FINITE_ELEMENT_SONAME = lib$(SO_LIB_FINITE_ELEMENT)$(SO_LIB_SUFFIX)
LIB_FINITE_ELEMENT_SRCS = \
	finite_element/export_finite_element.c \
	finite_element/finite_element.c \
	finite_element/finite_element_region.c \
	finite_element/finite_element_time.c \
	finite_element/import_finite_element.c \
	finite_element/read_fieldml.c \
	finite_element/write_fieldml.c \
	$(REGION_SRCS)
LIB_FINITE_ELEMENT_OBJS = $(addsuffix .o,$(basename $(LIB_FINITE_ELEMENT_SRCS)))
$(SO_LIB_FINITE_ELEMENT_TARGET) : $(LIB_FINITE_ELEMENT_OBJS) $(SO_LIB_GENERAL_TARGET) cmgui.Makefile
	$(call BuildSharedLibraryTarget,$(SO_LIB_FINITE_ELEMENT_TARGET),$(BIN_PATH),$(LIB_FINITE_ELEMENT_OBJS),$(ALL_SO_LINK_FLAGS) $(BIN_PATH)/$(SO_LIB_GENERAL_TARGET) $(XML2_LIB) $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libz.a $(LIB),$(SO_LIB_FINITE_ELEMENT_SONAME))
#	$(call BuildSharedLibraryTarget,$(SO_LIB_FINITE_ELEMENT_TARGET),$(BIN_PATH),$(LIB_FINITE_ELEMENT_OBJS),$(ALL_SO_LINK_FLAGS) $(BIN_PATH)/$(SO_LIB_GENERAL_TARGET) $(XML2_LIB) $(LIB),$(SO_LIB_FINITE_ELEMENT_SONAME))

SO_LIB_COMPUTED_VARIABLE = cmgui_computed_variable
SO_LIB_COMPUTED_VARIABLE_TARGET = lib$(SO_LIB_COMPUTED_VARIABLE)$(TARGET_SUFFIX)$(SO_LIB_SUFFIX)
SO_LIB_COMPUTED_VARIABLE_SONAME = lib$(SO_LIB_COMPUTED_VARIABLE)$(SO_LIB_SUFFIX)
LIB_COMPUTED_VARIABLE_SRCS = \
	api/cmiss_variable_new.cpp \
	api/cmiss_variable_new_basic.cpp \
	$(filter-out %finite_element.c,$(COMPUTED_VARIABLE_SRCS)) \
	$(MATRIX_SRCS) \
	api/cmiss_value_derivative_matrix.c \
	api/cmiss_value_fe_value.c \
	api/cmiss_value_matrix.c \
	api/cmiss_variable.c \
	api/cmiss_variable_composite.c \
	api/cmiss_variable_composition.c \
	api/cmiss_variable_coordinates.c \
	api/cmiss_variable_derivative.c \
	api/cmiss_variable_identity.c	
LIB_COMPUTED_VARIABLE_OBJS = $(addsuffix .o,$(basename $(LIB_COMPUTED_VARIABLE_SRCS)))
$(SO_LIB_COMPUTED_VARIABLE_TARGET) : $(LIB_COMPUTED_VARIABLE_OBJS) $(SO_LIB_GENERAL_TARGET) cmgui.Makefile
	$(call BuildSharedLibraryTarget,$(SO_LIB_COMPUTED_VARIABLE_TARGET),$(BIN_PATH),$(LIB_COMPUTED_VARIABLE_OBJS),$(ALL_SO_LINK_FLAGS) $(BIN_PATH)/$(SO_LIB_GENERAL_TARGET) $(MATRIX_LIB) $(LIB),$(SO_LIB_COMPUTED_VARIABLE_SONAME))

SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT = cmgui_computed_variable_finite_element
SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET = lib$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT)$(TARGET_SUFFIX)$(SO_LIB_SUFFIX)
SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_SONAME = lib$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT)$(SO_LIB_SUFFIX)
LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_SRCS = \
	$(filter %finite_element.c,$(COMPUTED_VARIABLE_SRCS)) \
	api/cmiss_value_element_xi.c \
	api/cmiss_variable_finite_element.c
LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_OBJS = $(addsuffix .o,$(basename $(LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_SRCS)))
$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET) : $(LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_OBJS) $(SO_LIB_COMPUTED_VARIABLE_TARGET) $(SO_LIB_FINITE_ELEMENT_TARGET) $(SO_LIB_GENERAL_TARGET) cmgui.Makefile
	$(call BuildSharedLibraryTarget,$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET),$(BIN_PATH),$(LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_OBJS),$(ALL_SO_LINK_FLAGS) $(BIN_PATH)/$(SO_LIB_COMPUTED_VARIABLE_TARGET) $(BIN_PATH)/$(SO_LIB_FINITE_ELEMENT_TARGET) $(BIN_PATH)/$(SO_LIB_GENERAL_TARGET),$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_SONAME))

SO_ALL_LIB = $(GRAPHICS_LIB) $(USER_INTERFACE_LIB) $(HAPTIC_LIB) \
	$(WORMHOLE_LIB) $(IMAGEMAGICK_LIB) \
	$(VIDEO_LIB) $(EXTERNAL_INPUT_LIB) $(HELP_LIB) \
	$(MOVIE_FILE_LIB) $(XML_LIB) $(MEMORYCHECK_LIB) \
	$(LIB)

SO_LIB_TARGET = lib$(TARGET_EXECUTABLE_BASENAME)$(TARGET_SUFFIX)$(SO_LIB_SUFFIX)
SO_LIB_SONAME = lib$(TARGET_EXECUTABLE_BASENAME)$(SO_LIB_SUFFIX)
REMAINING_LIB_SRCS = \
	$(filter-out $(LIB_GENERAL_SRCS) $(LIB_FINITE_ELEMENT_SRCS) $(LIB_COMPUTED_VARIABLE_SRCS) $(LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_SRCS), $(SRCS))
REMAINING_LIB_OBJS = $(addsuffix .o,$(basename $(REMAINING_LIB_SRCS)))
# SAB We are not resolving everything here (i.e. $(ALL_SO_LINK_FLAGS)) as we want to bind
# to the appropriate interpreter at runtime.  We could settle on a standard interpreter
# so_name but that would have to be used by the actual external Cmiss::Perl_cmiss
# perl and python modules that are supplying us the connections to their interpreters.
$(SO_LIB_TARGET) : $(REMAINING_LIB_OBJS) $(COMPILED_RESOURCE_FILES) $(OBJECT_PATH)/$(EXPORTS_FILE) $(SO_LIB_COMPUTED_VARIABLE_TARGET) $(SO_LIB_FINITE_ELEMENT_TARGET) $(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET) $(SO_LIB_GENERAL_TARGET) cmgui.Makefile
	$(call BuildSharedLibraryTarget,$(SO_LIB_TARGET),$(BIN_PATH),$(REMAINING_LIB_OBJS),$(SO_ALL_LIB) $(COMPILED_RESOURCE_FILES) $(BIN_PATH)/$(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET) $(BIN_PATH)/$(SO_LIB_COMPUTED_VARIABLE_TARGET) $(BIN_PATH)/$(SO_LIB_FINITE_ELEMENT_TARGET) $(BIN_PATH)/$(SO_LIB_GENERAL_TARGET),$(SO_LIB_SONAME))

#Make so_lib to be a shorthand for making all the so_libs
so_lib : $(SO_LIB_GENERAL_TARGET) $(SO_LIB_FINITE_ELEMENT_TARGET) $(SO_LIB_COMPUTED_VARIABLE_TARGET) $(SO_LIB_COMPUTED_VARIABLE_FINITE_ELEMENT_TARGET) $(SO_LIB_TARGET)

STATIC_LIB_SUFFIX = .a
STATIC_LIB_TARGET = lib$(TARGET_EXECUTABLE_BASENAME)$(TARGET_SUFFIX)$(STATIC_LIB_SUFFIX)

$(STATIC_LIB_TARGET) : $(OBJS)
	$(call BuildStaticLibraryTarget,$(STATIC_LIB_TARGET),$(BIN_PATH),$(OBJS))

#Make static_lib to be a shorthand for the fully qualified file
static_lib : $(STATIC_LIB_TARGET)

# Force is a dummy rule used to ensure some objects are made every time as the
# target 'force' doesnt exist */
force :
	@echo "\n" > /dev/null

ifeq ($(MEMORYCHECK),true)
   # Specify rule for making a memory checking object with the normal c file 
   # This allows us to make a memory checking version without creating a new extension
   general/debug_memory_check.o: general/debug.c general/debug.h
		if [ ! -d $(OBJECT_PATH)/general ]; then \
			mkdir -p $(OBJECT_PATH)/general; \
		fi
		if [ -f general/debug.c ]; then \
			$(CC) -o $(OBJECT_PATH)/general/debug_memory_check.o $(ALL_FLAGS) -DMEMORY_CHECKING general/debug.c; \
		else \
			$(CC) -o $(OBJECT_PATH)/general/debug_memory_check.o $(ALL_FLAGS) -DMEMORY_CHECKING $(PRODUCT_PATH)/general/debug.c; \
		fi

   general/debug_memory_check.d : general/debug.c general/debug.h
		@if [ ! -d $(OBJECT_PATH)/$(*D) ]; then \
			mkdir -p $(OBJECT_PATH)/$(*D); \
		fi
		@set -x ; $(MAKEDEPEND) $(ALL_FLAGS) $(STRICT_FLAGS) $<  | \
		sed -e 's%^.*\.o%$*.o $(OBJECT_PATH)/$*.d%;s%$(SOURCE_PATH)/%%g;s%$(PRODUCT_SOURCE_PATH)/%%g' > $(OBJECT_PATH)/$*.d ;
   ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
      # Fix up the uidh references
		sed -e 's%$(UIDH_PATH)/%%g;s%$(PRODUCT_UIDH_PATH)/%%g' $(OBJECT_PATH)/$*.d > $(OBJECT_PATH)/$*.d2
		mv $(OBJECT_PATH)/$*.d2 $(OBJECT_PATH)/$*.d
   endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
endif # $(MEMORYCHECK) == true

ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)

   utilities : $(UID2UIDH_BIN)

   #Only have the rules to build it if it doesn't exist, so that if it is
   #found but the objects are in another version that won't force other
   #version to rebuild.
   ifneq ($(UID2UIDH_FOUND),true)
      UID2UIDH_SRCS = \
	      utilities/uid2uidh.c

      UID2UIDH_LIB =
      ifeq ($(SYSNAME:IRIX%=),)
         UID2UIDH_LIB += -lgen
      endif # SYSNAME == IRIX%=

      UID2UIDH_OBJSA = $(UID2UIDH_SRCS:.c=.o)
      UID2UIDH_OBJSB = $(UID2UIDH_OBJSA:.cpp=.o)
      UID2UIDH_OBJS = $(UID2UIDH_OBJSB:.f=.o)
      BUILD_UID2UIDH = $(call BuildNormalTarget,$(UID2UIDH),$(UTILITIES_PATH),$(UID2UIDH_OBJS),$(UID2UIDH_LIB))

      $(UID2UIDH_BIN): $(UID2UIDH_OBJS)
			$(BUILD_UID2UIDH)
   endif # $(UID2UIDH_FOUND != true
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

utilities : SpacesToTabs TabsToSpaces

SPACES_TO_TABS_SRCS = \
	utilities/spaces_to_tabs.c

SPACES_TO_TABS_OBJSA = $(SPACES_TO_TABS_SRCS:.c=.o)
SPACES_TO_TABS_OBJSB = $(SPACES_TO_TABS_OBJSA:.cpp=.o)
SPACES_TO_TABS_OBJS = $(SPACES_TO_TABS_OBJSB:.f=.o)
BUILD_SPACES_TO_TABS = $(call BuildNormalTarget,SpacesToTabs,$(UTILITIES_PATH),$(SPACES_TO_TABS_OBJS),-lm) 

SpacesToTabs: $(SPACES_TO_TABS_OBJS)
	$(BUILD_SPACES_TO_TABS)

TABS_TO_SPACES_SRCS = \
	utilities/tabs_to_spaces.c

TABS_TO_SPACES_OBJSA = $(TABS_TO_SPACES_SRCS:.c=.o)
TABS_TO_SPACES_OBJSB = $(TABS_TO_SPACES_OBJSA:.cpp=.o)
TABS_TO_SPACES_OBJS = $(TABS_TO_SPACES_OBJSB:.f=.o)
BUILD_TABS_TO_SPACES = $(call BuildNormalTarget,TabsToSpaces,$(UTILITIES_PATH),$(TABS_TO_SPACES_OBJS),-lm) 

TabsToSpaces: $(TABS_TO_SPACES_OBJS)
	$(BUILD_TABS_TO_SPACES)

DEPENDFILE = $(OBJECT_PATH)/$(BIN_TARGET).depend

DEPEND_FILES = $(OBJS:%.o=%.d) $(MAIN_OBJ:%.o=%.d) $(UID2UIDH_OBJS:%.o=%.d) $(SPACES_TO_TABS_OBJS:%.o=%.d) $(TABS_TO_SPACES_OBJS:%.o=%.d)
#Look in the OBJECT_PATH
DEPEND_FILES_OBJECT_PATH = $(DEPEND_FILES:%.d=$(OBJECT_PATH)/%.d)
DEPEND_FILES_OBJECT_FOUND = $(wildcard $(DEPEND_FILES_OBJECT_PATH))
DEPEND_FILES_OBJECT_NOTFOUND = $(filter-out $(DEPEND_FILES_OBJECT_FOUND),$(DEPEND_FILES_OBJECT_PATH))
DEPEND_FILES_MISSING_PART1 = $(DEPEND_FILES_OBJECT_NOTFOUND:$(OBJECT_PATH)/%.d=%.d)
#Look for missing files in the PRODUCT_OBJECT_PATH
ifdef CMISS_ROOT_DEFINED
   DEPEND_FILES_PRODUCT_PATH = $(DEPEND_FILES_MISSING_PART1:%.d=$(PRODUCT_OBJECT_PATH)/%.d)
   DEPEND_FILES_PRODUCT_FOUND = $(wildcard $(DEPEND_FILES_PRODUCT_PATH))
   DEPEND_FILES_PRODUCT_NOTFOUND = $(filter-out $(DEPEND_FILES_PRODUCT_FOUND),$(DEPEND_FILES_PRODUCT_PATH))

   DEPEND_FILES_MISSING = $(DEPEND_FILES_PRODUCT_NOTFOUND:$(PRODUCT_OBJECT_PATH)/%.d=%.d)
   DEPEND_FILES_INCLUDE = $(DEPEND_FILES_OBJECT_FOUND) $(DEPEND_FILES_PRODUCT_FOUND) $(DEPEND_FILES_MISSING)
else
   DEPEND_FILES_MISSING = $(DEPEND_FILES_MISSING_PART1)
   DEPEND_FILES_INCLUDE = $(DEPEND_FILES_OBJECT_FOUND) $(DEPEND_FILES_MISSING)
endif

#Touch a dummy include so that this makefile is reloaded and therefore the new .ds
$(DEPENDFILE) : $(DEPEND_FILES_INCLUDE)
	touch $(DEPENDFILE)

include $(DEPENDFILE)
include $(DEPEND_FILES_INCLUDE)

# **************************************************************************
# FILE : unemap.Makefile
#
# LAST MODIFIED : 14 May 2004
#
# DESCRIPTION :
#
# Makefile for unemap
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
   MIRAGE = true
   UNEMAP = true
   ifneq ($(SYSNAME),AIX)
      CELL = true
   endif # SYSNAME == AIX
   LINK_CMISS = true
endif # $(USER_INTERFACE) != CONSOLE_USER_INTERFACE && $(USER_INTERFACE) != GTK_USER_INTERFACE
PERL_INTERPRETER = true
ifneq ($(SYSNAME),win32)
   IMAGEMAGICK = true
endif # SYSNAME != win32
ifeq ($(SYSNAME),win32)
   ifeq ($(filter CONSOLE_USER_INTERFACE GTK_USER_INTERFACE,$(USER_INTERFACE)),)
      WIN32_USER_INTERFACE = true 
   endif # $(USER_INTERFACE) != CONSOLE_USER_INTERFACE && $(USER_INTERFACE) != GTK_USER_INTERFACE
endif # SYSNAME == win32

ifdef USE_UNEMAP_NODES
   ifndef USE_UNEMAP_3D
      USE_UNEMAP_3D = true
   endif # ! USE_UNEMAP_3D
endif # USE_UNEMAP_NODES

TARGET_EXECUTABLE_BASENAME = unemap

ifneq ($(ABI),64)
   TARGET_ABI_SUFFIX =
else # $(ABI) != 64
   TARGET_ABI_SUFFIX = 64
endif # $(ABI) != 64

TARGET_UNEMAP_TYPE_SUFFIX :=
ifdef USE_UNEMAP_3D
   ifdef USE_UNEMAP_NODES
      TARGET_UNEMAP_TYPE_SUFFIX := -nodes
   else # USE_UNEMAP_NODES
      TARGET_UNEMAP_TYPE_SUFFIX := -3d
   endif # USE_UNEMAP_NODES
endif # USE_UNEMAP_3D

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
ifeq ($(SYSNAME:CYGWIN%=),)
   TARGET_FILETYPE_SUFFIX = .exe
endif # SYSNAME == CYGWIN%=

BIN_PATH=$(CMGUI_DEV_ROOT)/bin/$(BIN_ARCH_DIR)

BIN_TARGET = $(TARGET_EXECUTABLE_BASENAME)$(TARGET_ABI_SUFFIX)$(TARGET_UNEMAP_TYPE_SUFFIX)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_STATIC_LINK_SUFFIX)$(TARGET_DEBUG_SUFFIX)$(TARGET_MEMORYCHECK_SUFFIX)$(TARGET_FILETYPE_SUFFIX)
OBJECT_PATH=$(CMGUI_DEV_ROOT)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_UNEMAP_TYPE_SUFFIX)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
PRODUCT_OBJECT_PATH=$(PRODUCT_PATH)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_UNEMAP_TYPE_SUFFIX)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
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
   PLATFORM_DEFINES = -DSGI -Dmips
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   PLATFORM_DEFINES = -DGENERIC_PC
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   PLATFORM_DEFINES = -DAIX
   OPERATING_SYSTEM_DEFINES = -DUNIX
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   PLATFORM_DEFINES = -DGENERIC_PC
   OPERATING_SYSTEM_DEFINES = -DWIN32_SYSTEM
endif # SYSNAME == win32
ifeq ($(SYSNAME:CYGWIN%=),)
   PLATFORM_DEFINES = -DGENERIC_PC
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
GRAPHICS_INC =
ifdef USE_UNEMAP_3D
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
      ifneq ($(wildcard $(CMISS_ROOT)/mesa/include/$(LIB_ARCH_DIR)),)
         GRAPHICS_INC += -I$(CMISS_ROOT)/mesa/include/$(LIB_ARCH_DIR)
      endif
      GRAPHICS_LIB += $(patsubst %,-L%,$(firstword $(wildcard $(CMISS_ROOT)/mesa/lib/$(LIB_ARCH_DIR) /usr/X11R6/lib)))
      ifeq ($(SYSNAME),win32)
         GRAPHICS_LIB += -lopengl32 -lglu32
      else # $(SYSNAME) == win32
         GRAPHICS_LIB += -lGL -lGLU
      endif # $(SYSNAME) == win32 
   endif # $(USER_INTERFACE) != GTK_USER_INTERFACE
endif # USE_UNEMAP_3D

USE_NODES_SWITCH =
ifdef USE_UNEMAP_NODES
USE_NODES_SWITCH += -DUNEMAP_USE_NODES
endif # USE_UNEMAP_NODES
ifdef USE_UNEMAP_3D
USE_NODES_SWITCH += -DUNEMAP_USE_3D
endif # USE_UNEMAP_NODES

ifndef IMAGEMAGICK
   IMAGEMAGICK_DEFINES =
   IMAGEMAGICK_LIB = 
else # ! IMAGEMAGICK
   IMAGEMAGICK_DEFINES += -DIMAGEMAGICK
   IMAGEMAGICK_PATH = $(CMISS_ROOT)/image_libraries
   IMAGEMAGICK_INC = -I$(IMAGEMAGICK_PATH)/include/$(LIB_ARCH_DIR)
   IMAGEMAGICK_LIB = $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libMagick.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libtiff.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libpng.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libjpeg.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libxml2.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libbz2.a $(IMAGEMAGICK_PATH)/lib/$(LIB_ARCH_DIR)/libz.a
   ifeq ($(SYSNAME),AIX)
      IMAGEMAGICK_LIB += /usr/lib/libiconv.a
   endif # SYSNAME == AIX
endif # ! IMAGEMAGICK

ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   UIDH_INC = -I$(UIDH_PATH) -I$(PRODUCT_UIDH_PATH)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

EXTERNAL_INPUT_DEFINES =
EXTERNAL_INPUT_LIB = 
ifdef USE_UNEMAP_3D
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
endif # USE_UNEMAP_3D

STEREO_DISPLAY_DEFINES = -DSTEREO
# STEREO_DISPLAY_DEFINES =

# POSTSCRIPT_DEFINES = -DFEEDBACK_POSTSCRIPT
POSTSCRIPT_DEFINES =

#  By default some names are "mangled" to get external names <= 32 characters
#  NAME_DEFINES =
NAME_DEFINES = -DFULL_NAMES

#  Temporary flags that are used during development
TEMPORARY_DEVELOPMENT_FLAGS =
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

USER_INTERFACE_INC = 
USER_INTERFACE_LIB =
USER_INTERFACE_INC = 
USER_INTERFACE_LIB =
ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)
   ifeq ($(SYSNAME),Linux)
      ifneq ($(wildcard /usr/local/Mesa/include),)
         USER_INTERFACE_INC += -I/usr/local/Mesa/include
      endif
      USER_INTERFACE_INC += -I/usr/X11R6/include
      X_LIB = /usr/X11R6/lib
      USER_INTERFACE_LIB += -L$(X_LIB)

      ifneq ($(STATIC_LINK),true)
         #I am statically linking Motif so that it does not have to be installed at runtime.
         USER_INTERFACE_LIB += $(X_LIB)/libMrm.a $(X_LIB)/libXm.a -lXp -lXt -lX11 -lXmu -lXext
      else # STATIC_LINK != true
         #Mandrake 8.2 static libs are incompatible, this works around it by
         #comparing the size of the symbols and forcing Xmu to preload its
         #version if they differ in size.  Older greps don't have -o option.
         Xm_XeditRes = $(shell /usr/bin/objdump -t $(X_LIB)/libXm.a | /bin/grep '00000[0-f][0-f][1-f] _XEditResCheckMessages')
         Xmu_XeditRes = $(shell /usr/bin/objdump -t $(X_LIB)/libXmu.a | /bin/grep '00000[0-f][0-f][1-f] _XEditResCheckMessages')
         ifneq ($(Xm_XeditRes),)
            ifneq ($(Xmu_XeditRes),)
               USER_INTERFACE_LIB += -u _XEditResCheckMessages -lXmu 
            endif
         endif
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
   endif # SYSNAME == CYGWIN%=
   ifneq ($(SYSNAME),win32)
      #USE_GTK2 = true
      ifeq ($(USE_GTK2),true)
         USER_INTERFACE_INC += $(shell "/home/blackett/bin/pkg-config gtkgl-2.0 gtk+-2.0 --cflags")
         ifneq ($(STATIC_LINK),true)
            USER_INTERFACE_LIB += $(shell "/home/blackett/bin/pkg-config gtkgl-2.0 gtk+-2.0 --libs")
         else # $(STATIC_LINK) != true
            USER_INTERFACE_LIB += -L/home/blackett/lib -lgtkgl-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 -L/usr/local/Mesa/lib -lGLU -lGL
         endif # $(STATIC_LINK) != true
      else # $(USE_GTK2) == true
         USER_INTERFACE_INC +=  -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include/
         USER_INTERFACE_LIB +=  -lgtkgl -L/usr/local/Mesa/lib -lGLU -lGL -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11
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

ifdef USE_UNEMAP_3D
   ifeq ($(SYSNAME:IRIX%=),)
	   MATRIX_LIB = -lscs
   else # ($(SYSNAME:IRIX%=),)
      MATRIX_LIB = -L$(CMISS_ROOT)/linear_solvers/lib/$(LIB_ARCH_DIR) -llapack-debug -lblas-debug
   endif # ($(SYSNAME:IRIX%=),)
   ifeq ($(SYSNAME),AIX)
      MATRIX_LIB += -lxlf90
   endif # SYSNAME == AIX
endif # USE_UNEMAP_3D


ifeq ($(SYSNAME:IRIX%=),)
   LIB = -lPW -lftn -lm -lC -lCio -lpthread 
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifneq ($(STATIC_LINK),true)
      #For the dynamic link we really need to statically link the c++ as this
      #seems to be particularly variable between distributions.
      ifdef USE_UNEMAP_3D
         LIB = -lg2c -lm -ldl -lc -lpthread /usr/lib/libcrypt.a -lstdc++
      else # USE_UNEMAP_3D
         LIB = -lm -ldl -lc -lpthread /usr/lib/libcrypt.a 
      endif # USE_UNEMAP_3D
   else # $(STATIC_LINK) != true
      LIB = -lm -ldl -lpthread -lcrypt -lstdc++
   endif # $(STATIC_LINK) != true
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   LIB = -lm -ldl -lSM -lICE -lpthread -lcrypt -lbsd -lld -lC128
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   LIB = -lgdi32  -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lwsock32 -lmpr -lwinmm -lversion -lodbc32
endif # SYSNAME == win32
ifeq ($(SYSNAME:CYGWIN%=),)
   ifneq ($(STATIC_LINK),true)
      #For the dynamic link we really need to statically link the c++ as this
      #seems to be particularly variable between distributions.
      LIB = -lg2c -lm -ldl -lc -lpthread /usr/lib/libcrypt.a -lstdc++
   else # $(STATIC_LINK) != true
      LIB = -lg2c -lm -lpthread -lcrypt -lstdc++
   endif # $(STATIC_LINK) != true
endif # SYSNAME == CYGWIN%=

ifdef USE_UNEMAP_3D
ALL_DEFINES = $(COMPILE_DEFINES) $(TARGET_TYPE_DEFINES) \
	$(PLATFORM_DEFINES) $(OPERATING_SYSTEM_DEFINES) $(USER_INTERFACE_DEFINES) \
	$(USE_NODES_SWITCH) $(STEREO_DISPLAY_DEFINES) \
	$(EXTERNAL_INPUT_DEFINES) $(IMAGEMAGICK_DEFINES) \
	$(GRAPHICS_LIBRARY_DEFINES) $(NAME_DEFINES) $(TEMPORARY_DEVELOPMENT_FLAGS) \
	-DNOT_ACQUISITION_ONLY \
	-DSPECTRAL_TOOLS \
	-DNETSCAPE_HELP
else # USE_UNEMAP_3D
ALL_DEFINES = $(COMPILE_DEFINES) $(TARGET_TYPE_DEFINES) \
	$(PLATFORM_DEFINES) $(OPERATING_SYSTEM_DEFINES) $(USER_INTERFACE_DEFINES) \
	$(USE_NODES_SWITCH) $(STEREO_DISPLAY_DEFINES) \
	$(EXTERNAL_INPUT_DEFINES) $(IMAGEMAGICK_DEFINES) $(NAME_DEFINES) \
   $(TEMPORARY_DEVELOPMENT_FLAGS) -DNOT_ACQUISITION_ONLY \
	-DSPECTRAL_TOOLS
endif # USE_UNEMAP_3D

ALL_INCLUDES = $(SOURCE_DIRECTORY_INC) $(WORMHOLE_INC) $(IMAGEMAGICK_INC) \
	$(GRAPHICS_INC) $(UIDH_INC) $(USER_INTERFACE_INC)

ALL_FLAGS = $(OPTIMISATION_FLAGS) $(COMPILE_FLAGS) $(TARGET_TYPE_FLAGS) \
	$(ALL_DEFINES) $(ALL_INCLUDES)

ifdef USE_UNEMAP_3D
ALL_LIB = $(GRAPHICS_LIB) $(USER_INTERFACE_LIB) \
	$(IMAGEMAGICK_LIB) $(EXTERNAL_INPUT_LIB) $(HELP_LIB) $(MATRIX_LIB) \
	$(LIB)
else # USE_UNEMAP_3D
ALL_LIB = $(USER_INTERFACE_LIB) $(IMAGEMAGICK_LIB) $(EXTERNAL_INPUT_LIB) \
	$(HELP_LIB) $(MATRIX_LIB) \
	$(LIB)
endif # USE_UNEMAP_3D

COMMAND_SRCS = \
	command/command.c \
	command/parser.c

ifdef USE_UNEMAP_3D
CHOOSE_SRCS = \
	choose/choose_computed_field.c \
	choose/choose_fe_field.c \
	choose/chooser.c
COLOUR_SRCS = \
	colour/edit_var.c
COMPUTED_FIELD_SRCS = \
	computed_field/computed_field.c \
	computed_field/computed_field_component_operations.c \
	computed_field/computed_field_composite.c \
	computed_field/computed_field_coordinate.c \
	computed_field/computed_field_finite_element.c \
	computed_field/computed_field_find_xi.c \
	computed_field/computed_field_fibres.c \
	computed_field/computed_field_integration.c \
	computed_field/computed_field_set.c \
	computed_field/computed_field_update.c \
	computed_field/computed_field_value_index_ranges.c \
	computed_field/computed_field_vector_operations.c \
	computed_field/computed_field_wrappers.c
ELEMENT_SRCS = \
	element/element_operations.c
FINITE_ELEMENT_SRCS = \
	finite_element/export_finite_element.c \
	finite_element/finite_element.c \
	finite_element/finite_element_discretization.c \
	finite_element/finite_element_region.c \
	finite_element/finite_element_time.c \
	finite_element/finite_element_to_graphics_object.c \
	finite_element/finite_element_to_iso_lines.c \
	finite_element/finite_element_to_streamlines.c \
	finite_element/finite_element_adjacent_elements.c \
	finite_element/import_finite_element.c
GENERAL_SRCS = \
	general/any_object.c \
	general/callback.c \
	general/compare.c \
	general/debug.c \
	general/error_handler.c \
	general/geometry.c \
	general/heapsort.c \
	general/image_utilities.c \
	general/indexed_multi_range.c \
	general/machine.c \
	general/matrix_vector.c \
	general/multi_range.c \
	general/myio.c \
	general/mystring.c \
	general/child_process.c \
	general/value.c \
	general/postscript.c \
	general/statistics.c
GRAPHICS_SRCS = \
	graphics/complex.c \
	graphics/laguer.c \
	graphics/spectrum.c \
	graphics/environment_map.c \
	graphics/texture_line.c \
	graphics/mcubes.c \
	graphics/makegtobj.c \
	graphics/glyph.c \
	graphics/graphical_element.c \
	graphics/auxiliary_graphics_types.c \
	graphics/element_group_settings.c \
	graphics/element_point_ranges.c \
	graphics/volume_texture.c \
	graphics/material.c \
	graphics/rendergl.c \
	graphics/graphics_object.c \
	graphics/graphics_library.c \
	graphics/light.c \
	graphics/light_model.c \
	graphics/colour.c \
	graphics/scene_viewer.c \
	graphics/scene.c \
	graphics/order_independent_transparency.c \
	graphics/selected_graphic.c \
	graphics/spectrum_settings.c \
	graphics/texture.c \
	graphics/transform_tool.c
HELP_INTERFACE_SRCS = \
	help/help_interface.c
INTERACTION_SRCS = \
	interaction/interaction_graphics.c \
	interaction/interaction_volume.c \
	interaction/interactive_tool.c \
	interaction/interactive_toolbar_widget.c \
	interaction/interactive_event.c
MATRIX_SRCS =  \
	matrix/factor.c \
	matrix/matrix.c \
	matrix/matrix_blas.c
MOTIF_INTERFACE_SRCS =  \
	motif/image_utilities.c
NODE_SRCS = \
	node/node_operations.c \
	node/node_tool.c
POSITION_SRCS = \
	io_devices/input_module.c \
	io_devices/matrix.c
REGION_SRCS = \
   region/cmiss_region.c \
   region/cmiss_region_chooser.c \
   region/cmiss_region_write_info.c
SELECTION_SRCS = \
	selection/element_point_ranges_selection.c \
	selection/element_selection.c \
	selection/node_selection.c
THREE_D_DRAWING_SRCS = \
	three_d_drawing/dm_interface.c \
	three_d_drawing/graphics_buffer.c \
	three_d_drawing/movie_extensions.c \
	three_d_drawing/ThreeDDraw.c
TIME_SRCS = \
	time/time.c \
	time/time_keeper.c \
	time/time_editor.c \
	time/time_editor_dialog.c
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
	unemap/unemap_package.c \
	unemap/unemap_hardware_client.c
else # USE_UNEMAP_3D
CHOOSE_SRCS =
COLOUR_SRCS =
COMPUTED_FIELD_SRCS =
ELEMENT_SRCS =
FINITE_ELEMENT_SRCS =
GENERAL_SRCS = \
	general/callback.c \
	general/compare.c \
	general/debug.c \
	general/error_handler.c \
	general/geometry.c \
	general/heapsort.c \
	general/image_utilities.c \
	general/machine.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	general/postscript.c
GRAPHICS_SRCS = \
	graphics/colour.c \
	graphics/material.c \
	graphics/spectrum.c \
	graphics/spectrum_settings.c \
	graphics/texture.c
HELP_INTERFACE_SRCS =
INTERACTION_SRCS =
POSITION_SRCS =
MOTIF_INTERFACE_SRCS =
NODE_SRCS =
REGION_SRCS =
SELECTION_SRCS =
TIME_SRCS = \
	time/time.c \
	time/time_keeper.c \
	time/time_editor.c \
	time/time_editor_dialog.c
THREE_D_DRAWING_SRCS =
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
	unemap/unemap_package.c \
	unemap/unemap_hardware_client.c
endif # USE_UNEMAP_3D
USER_INTERFACE_SRCS = \
	user_interface/call_work_procedures.c \
	user_interface/confirmation.c \
	user_interface/event_dispatcher.c \
	user_interface/filedir.c \
	user_interface/message.c \
	user_interface/printer.c \
	user_interface/user_interface.c

SRCS = \
	$(CHOOSE_SRCS) \
	$(COLOUR_SRCS) \
	$(COMMAND_SRCS) \
	$(COMPUTED_FIELD_SRCS) \
	$(ELEMENT_SRCS) \
	$(FINITE_ELEMENT_SRCS) \
	$(GENERAL_SRCS) \
	$(GRAPHICS_SRCS) \
	$(HELP_INTERFACE_SRCS) \
	$(INTERACTION_SRCS) \
	$(MATRIX_SRCS) \
	$(MOTIF_INTERFACE_SRCS) \
	$(NODE_SRCS) \
	$(POSITION_SRCS) \
	$(REGION_SRCS) \
	$(SELECTION_SRCS) \
	$(THREE_D_DRAWING_SRCS) \
	$(TIME_SRCS) \
	$(UNEMAP_SRCS) \
	$(USER_INTERFACE_SRCS)

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

MAIN_SRC = unemap.c
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
	../unemap.make unemap.imake common.imake unemap.Makefile common.Makefile \
   unemap.c version.h $(UTILITIES_PATH)/uid2uidh \
	unemap/*.c unemap/*.h unemap/*.uil unemap/*.rc \
	unemap_hardware_service/*.c unemap_hardware_service/*.h \
	unemap_hardware_service/*.uil \
	unemap/utilities/*.c unemap/utilities/*.h unemap/utilities/*.uil \
	command/parser.c command/parser.h \
	computed_field/computed_field.h \
	computed_field/computed_field_value_index_ranges.h finite_element/finite_element.h \
	finite_element/finite_element_to_graphics_object.h \
	finite_element/finite_element_to_streamlines.h \
	general/*.c general/*.h general/*.uil \
	graphics/auxiliary_graphics_types.h graphics/colour.c graphics/colour.h \
	graphics/element_group_settings.h graphics/environment_map.h \
	graphics/graphical_element.h graphics/graphics_object.h graphics/material.c \
	graphics/material.h graphics/selected_graphic.h graphics/spectrum.c \
	graphics/spectrum.h graphics/spectrum_settings.c \
	graphics/spectrum_settings.h graphics/graphics_library.h graphics/light.h \
	graphics/scene.h graphics/element_point_ranges.h graphics/texture.c \
	graphics/texture.h graphics/volume_texture.h \
	interaction/interaction_volume.h \
	io_devices/*.c io_devices/*.h io_devices/*.uil \
	selection/*.h \
	time/*.c time/*.h time/*.uil \
	three_d_drawing/*.h \
	user_interface/*.c user_interface/*.h user_interface/*.uil \
	| gzip > unemap_tar.gz

$(OBJECT_PATH)/version.o.h : $(OBJS) unemap.Makefile
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi	
	echo '/* This is a generated file.  Do not edit.  Edit unemap.c or unemap.Makefile instead */' > $(OBJECT_PATH)/version.o.h;	  
	date > date.h ;
	sed 's/"//;s/./#define VERSION "unemap version 001.001.010  &/;s/.$$/&\\nCopyright 1996-2004, Auckland UniServices Ltd."/' < date.h >> $(OBJECT_PATH)/version.o.h

$(MAIN_OBJ) : $(MAIN_SRC) $(OBJECT_PATH)/version.o.h $(INTERPRETER_LIB)
	@if [ -f $*.c ]; then \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $*.c > $(OBJECT_PATH)/$*.o.c ; \
	else \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $(PRODUCT_SOURCE_PATH)/$*.c > $(OBJECT_PATH)/$*.o.c ; \
	fi
	$(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(OBJECT_PATH)/$*.o.c;

$(BIN_TARGET) : $(OBJS) $(MAIN_OBJ)
	$(call BuildNormalTarget,$(BIN_TARGET),$(BIN_PATH),$(OBJS) $(MAIN_OBJ),$(ALL_LIB))

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

UNEMAP_UTILITIES_PATH = $(UTILITIES_PATH)/unemap

utilities : $(UNEMAP_UTILITIES_PATH)/activation_summary$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/affine$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/analysis_calculate$(TARGET_ABI_SUFFIX).lib
utilities : $(UNEMAP_UTILITIES_PATH)/bmsi2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/biosense2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/biosense2vrml$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/calculate_events$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/change_calibration$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/change_channels$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/change_configuration$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/change_events$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/change_frequency$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/combine_signals$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/difference_signals$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/extract_signal$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/fix_ecg$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/img2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/make_plaque$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/optical_signals$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/plt2cnfg$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/posdat2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/ratio_signals$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/register$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/sig2text$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/test_delaunay$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/test_unemap_hardware$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/text2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/tuff2sig$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/unique_channels$(TARGET_ABI_SUFFIX)
utilities : $(UNEMAP_UTILITIES_PATH)/write_calibration$(TARGET_ABI_SUFFIX)

ACTIVATION_SUMMARY_SRCS = \
	unemap/utilities/activation_summary.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/analysis_calculate.c \
	unemap/analysis.c \
	unemap/rig.c \
	unemap/rig_node.c \
	user_interface/message.c

ACTIVATION_SUMMARY_OBJSA = $(ACTIVATION_SUMMARY_SRCS:.c=.o)
ACTIVATION_SUMMARY_OBJSB = $(ACTIVATION_SUMMARY_OBJSA:.cpp=.o)
ACTIVATION_SUMMARY_OBJS = $(ACTIVATION_SUMMARY_OBJSB:.f=.o)
BUILD_ACTIVATION_SUMMARY = $(call BuildNormalTarget,activation_summary$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(ACTIVATION_SUMMARY_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/activation_summary$(TARGET_ABI_SUFFIX): $(ACTIVATION_SUMMARY_OBJS)
	$(BUILD_ACTIVATION_SUMMARY)

AFFINE_SRCS = \
	unemap/utilities/affine.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

AFFINE_OBJSA = $(AFFINE_SRCS:.c=.o)
AFFINE_OBJSB = $(AFFINE_OBJSA:.cpp=.o)
AFFINE_OBJS = $(AFFINE_OBJSB:.f=.o)
BUILD_AFFINE = $(call BuildNormalTarget,affine$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(AFFINE_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/affine$(TARGET_ABI_SUFFIX): $(AFFINE_OBJS)
	$(BUILD_AFFINE)

ANALYSIS_CALCULATE_LIB_SRCS = \
	unemap/analysis_calculate.c

ANALYSIS_CALCULATE_LIB_OBJSA = $(ANALYSIS_CALCULATE_LIB_SRCS:.c=.o)
ANALYSIS_CALCULATE_LIB_OBJSB = $(ANALYSIS_CALCULATE_LIB_OBJSA:.cpp=.o)
ANALYSIS_CALCULATE_LIB_OBJS = $(ANALYSIS_CALCULATE_LIB_OBJSB:.f=.o)
BUILD_ANALYSIS_CALCULATE_LIB = $(call BuildLibraryTarget,analysis_calculate$(TARGET_ABI_SUFFIX).lib,$(UNEMAP_UTILITIES_PATH),$(ANALYSIS_CALCULATE_LIB_OBJS),) 

$(UNEMAP_UTILITIES_PATH)/analysis_calculate$(TARGET_ABI_SUFFIX).lib: $(ANALYSIS_CALCULATE_LIB_OBJS)
	$(BUILD_ANALYSIS_CALCULATE_LIB)

BMSI2SIG_SRCS = \
	unemap/utilities/bmsi2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

BMSI2SIG_OBJSA = $(BMSI2SIG_SRCS:.c=.o)
BMSI2SIG_OBJSB = $(BMSI2SIG_OBJSA:.cpp=.o)
BMSI2SIG_OBJS = $(BMSI2SIG_OBJSB:.f=.o)
BUILD_BMSI2SIG = $(call BuildNormalTarget,bmsi2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(BMSI2SIG_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/bmsi2sig$(TARGET_ABI_SUFFIX): $(BMSI2SIG_OBJS)
	$(BUILD_BMSI2SIG)

BIOSENSE2SIG_SRCS = \
	unemap/utilities/biosense2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

BIOSENSE2SIG_OBJSA = $(BIOSENSE2SIG_SRCS:.c=.o)
BIOSENSE2SIG_OBJSB = $(BIOSENSE2SIG_OBJSA:.cpp=.o)
BIOSENSE2SIG_OBJS = $(BIOSENSE2SIG_OBJSB:.f=.o)
BUILD_BIOSENSE2SIG = $(call BuildNormalTarget,biosense2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(BIOSENSE2SIG_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/biosense2sig$(TARGET_ABI_SUFFIX): $(BIOSENSE2SIG_OBJS)
	$(BUILD_BIOSENSE2SIG)

BIOSENSE2VRML_SRCS = \
	unemap/utilities/biosense2vrml.c \
	general/debug.c \
	general/myio.c \
	general/mystring.c \
	user_interface/message.c

BIOSENSE2VRML_OBJSA = $(BIOSENSE2VRML_SRCS:.c=.o)
BIOSENSE2VRML_OBJSB = $(BIOSENSE2VRML_OBJSA:.cpp=.o)
BIOSENSE2VRML_OBJS = $(BIOSENSE2VRML_OBJSB:.f=.o)
BUILD_BIOSENSE2VRML = $(call BuildNormalTarget,biosense2vrml$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(BIOSENSE2VRML_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/biosense2vrml$(TARGET_ABI_SUFFIX): $(BIOSENSE2VRML_OBJS)
	$(BUILD_BIOSENSE2VRML)

CALCULATE_EVENTS_SRCS = \
	unemap/utilities/calculate_events.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/analysis.c \
	unemap/analysis_calculate.c \
	unemap/interpolate.c \
	unemap/rig.c \
	unemap/rig_node.c \
	user_interface/message.c

CALCULATE_EVENTS_OBJSA = $(CALCULATE_EVENTS_SRCS:.c=.o)
CALCULATE_EVENTS_OBJSB = $(CALCULATE_EVENTS_OBJSA:.cpp=.o)
CALCULATE_EVENTS_OBJS = $(CALCULATE_EVENTS_OBJSB:.f=.o)
BUILD_CALCULATE_EVENTS = $(call BuildNormalTarget,calculate_events$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CALCULATE_EVENTS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/calculate_events$(TARGET_ABI_SUFFIX): $(CALCULATE_EVENTS_OBJS)
	$(BUILD_CALCULATE_EVENTS)

CHANGE_CALIBRATION_SRCS = \
	unemap/utilities/change_calibration.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

CHANGE_CALIBRATION_OBJSA = $(CHANGE_CALIBRATION_SRCS:.c=.o)
CHANGE_CALIBRATION_OBJSB = $(CHANGE_CALIBRATION_OBJSA:.cpp=.o)
CHANGE_CALIBRATION_OBJS = $(CHANGE_CALIBRATION_OBJSB:.f=.o)
BUILD_CHANGE_CALIBRATION = $(call BuildNormalTarget,change_calibration$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CHANGE_CALIBRATION_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/change_calibration$(TARGET_ABI_SUFFIX): $(CHANGE_CALIBRATION_OBJS)
	$(BUILD_CHANGE_CALIBRATION)

CHANGE_CHANNELS_SRCS = \
	unemap/utilities/change_channels.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

CHANGE_CHANNELS_OBJSA = $(CHANGE_CHANNELS_SRCS:.c=.o)
CHANGE_CHANNELS_OBJSB = $(CHANGE_CHANNELS_OBJSA:.cpp=.o)
CHANGE_CHANNELS_OBJS = $(CHANGE_CHANNELS_OBJSB:.f=.o)
BUILD_CHANGE_CHANNELS = $(call BuildNormalTarget,change_channels$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CHANGE_CHANNELS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/change_channels$(TARGET_ABI_SUFFIX): $(CHANGE_CHANNELS_OBJS)
	$(BUILD_CHANGE_CHANNELS)

CHANGE_CONFIGURATION_SRCS = \
	unemap/utilities/change_configuration.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

CHANGE_CONFIGURATION_OBJSA = $(CHANGE_CONFIGURATION_SRCS:.c=.o)
CHANGE_CONFIGURATION_OBJSB = $(CHANGE_CONFIGURATION_OBJSA:.cpp=.o)
CHANGE_CONFIGURATION_OBJS = $(CHANGE_CONFIGURATION_OBJSB:.f=.o)
BUILD_CHANGE_CONFIGURATION = $(call BuildNormalTarget,change_configuration$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CHANGE_CONFIGURATION_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/change_configuration$(TARGET_ABI_SUFFIX): $(CHANGE_CONFIGURATION_OBJS)
	$(BUILD_CHANGE_CONFIGURATION)

CHANGE_EVENTS_SRCS = \
	unemap/utilities/change_events.c

CHANGE_EVENTS_OBJSA = $(CHANGE_EVENTS_SRCS:.c=.o)
CHANGE_EVENTS_OBJSB = $(CHANGE_EVENTS_OBJSA:.cpp=.o)
CHANGE_EVENTS_OBJS = $(CHANGE_EVENTS_OBJSB:.f=.o)
BUILD_CHANGE_EVENTS = $(call BuildNormalTarget,change_events$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CHANGE_EVENTS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/change_events$(TARGET_ABI_SUFFIX): $(CHANGE_EVENTS_OBJS)
	$(BUILD_CHANGE_EVENTS)

CHANGE_FREQUENCY_SRCS = \
	unemap/utilities/change_frequency.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

CHANGE_FREQUENCY_OBJSA = $(CHANGE_FREQUENCY_SRCS:.c=.o)
CHANGE_FREQUENCY_OBJSB = $(CHANGE_FREQUENCY_OBJSA:.cpp=.o)
CHANGE_FREQUENCY_OBJS = $(CHANGE_FREQUENCY_OBJSB:.f=.o)
BUILD_CHANGE_FREQUENCY = $(call BuildNormalTarget,change_frequency$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(CHANGE_FREQUENCY_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/change_frequency$(TARGET_ABI_SUFFIX): $(CHANGE_FREQUENCY_OBJS)
	$(BUILD_CHANGE_FREQUENCY)

COMBINE_SIGNALS_SRCS = \
	unemap/utilities/combine_signals.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

COMBINE_SIGNALS_OBJSA = $(COMBINE_SIGNALS_SRCS:.c=.o)
COMBINE_SIGNALS_OBJSB = $(COMBINE_SIGNALS_OBJSA:.cpp=.o)
COMBINE_SIGNALS_OBJS = $(COMBINE_SIGNALS_OBJSB:.f=.o)
BUILD_COMBINE_SIGNALS = $(call BuildNormalTarget,combine_signals$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(COMBINE_SIGNALS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/combine_signals$(TARGET_ABI_SUFFIX): $(COMBINE_SIGNALS_OBJS)
	$(BUILD_COMBINE_SIGNALS)

DIFFERENCE_SIGNALS_SRCS = \
	unemap/utilities/difference_signals.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

DIFFERENCE_SIGNALS_OBJSA = $(DIFFERENCE_SIGNALS_SRCS:.c=.o)
DIFFERENCE_SIGNALS_OBJSB = $(DIFFERENCE_SIGNALS_OBJSA:.cpp=.o)
DIFFERENCE_SIGNALS_OBJS = $(DIFFERENCE_SIGNALS_OBJSB:.f=.o)
BUILD_DIFFERENCE_SIGNALS = $(call BuildNormalTarget,difference_signals$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(DIFFERENCE_SIGNALS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/difference_signals$(TARGET_ABI_SUFFIX): $(DIFFERENCE_SIGNALS_OBJS)
	$(BUILD_DIFFERENCE_SIGNALS)

EXTRACT_SIGNAL_SRCS = \
	unemap/utilities/extract_signal.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

EXTRACT_SIGNAL_OBJSA = $(EXTRACT_SIGNAL_SRCS:.c=.o)
EXTRACT_SIGNAL_OBJSB = $(EXTRACT_SIGNAL_OBJSA:.cpp=.o)
EXTRACT_SIGNAL_OBJS = $(EXTRACT_SIGNAL_OBJSB:.f=.o)
BUILD_EXTRACT_SIGNAL = $(call BuildNormalTarget,extract_signal$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(EXTRACT_SIGNAL_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/extract_signal$(TARGET_ABI_SUFFIX): $(EXTRACT_SIGNAL_OBJS)
	$(BUILD_EXTRACT_SIGNAL)

FIX_ECG_SRCS = \
	unemap/utilities/fix_ecg.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

FIX_ECG_OBJSA = $(FIX_ECG_SRCS:.c=.o)
FIX_ECG_OBJSB = $(FIX_ECG_OBJSA:.cpp=.o)
FIX_ECG_OBJS = $(FIX_ECG_OBJSB:.f=.o)
BUILD_FIX_ECG = $(call BuildNormalTarget,fix_ecg$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(FIX_ECG_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/fix_ecg$(TARGET_ABI_SUFFIX): $(FIX_ECG_OBJS)
	$(BUILD_FIX_ECG)

IMG2SIG_SRCS = \
	unemap/utilities/img2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

IMG2SIG_OBJSA = $(IMG2SIG_SRCS:.c=.o)
IMG2SIG_OBJSB = $(IMG2SIG_OBJSA:.cpp=.o)
IMG2SIG_OBJS = $(IMG2SIG_OBJSB:.f=.o)
BUILD_IMG2SIG = $(call BuildNormalTarget,img2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(IMG2SIG_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/img2sig$(TARGET_ABI_SUFFIX): $(IMG2SIG_OBJS)
	$(BUILD_IMG2SIG)

MAKE_PLAQUE_SRCS = \
	unemap/utilities/make_plaque.c

MAKE_PLAQUE_OBJSA = $(MAKE_PLAQUE_SRCS:.c=.o)
MAKE_PLAQUE_OBJSB = $(MAKE_PLAQUE_OBJSA:.cpp=.o)
MAKE_PLAQUE_OBJS = $(MAKE_PLAQUE_OBJSB:.f=.o)
BUILD_MAKE_PLAQUE = $(call BuildNormalTarget,make_plaque$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(MAKE_PLAQUE_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/make_plaque$(TARGET_ABI_SUFFIX): $(MAKE_PLAQUE_OBJS)
	$(BUILD_MAKE_PLAQUE)

OPTICAL_SIGNALS_SRCS = \
	unemap/utilities/optical_signals.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/rig.c \
	unemap/rig_node.c \
	unemap/spectral_methods.c \
	user_interface/message.c

OPTICAL_SIGNALS_OBJSA = $(OPTICAL_SIGNALS_SRCS:.c=.o)
OPTICAL_SIGNALS_OBJSB = $(OPTICAL_SIGNALS_OBJSA:.cpp=.o)
OPTICAL_SIGNALS_OBJS = $(OPTICAL_SIGNALS_OBJSB:.f=.o)
BUILD_OPTICAL_SIGNALS = $(call BuildNormalTarget,optical_signals$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(OPTICAL_SIGNALS_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/optical_signals$(TARGET_ABI_SUFFIX): $(OPTICAL_SIGNALS_OBJS)
	$(BUILD_OPTICAL_SIGNALS)

PLT2CNFG_SRCS = \
	unemap/utilities/plt2cnfg.c

PLT2CNFG_OBJSA = $(PLT2CNFG_SRCS:.c=.o)
PLT2CNFG_OBJSB = $(PLT2CNFG_OBJSA:.cpp=.o)
PLT2CNFG_OBJS = $(PLT2CNFG_OBJSB:.f=.o)
BUILD_PLT2CNFG = $(call BuildNormalTarget,plt2cnfg$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(PLT2CNFG_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/plt2cnfg$(TARGET_ABI_SUFFIX): $(PLT2CNFG_OBJS)
	$(BUILD_PLT2CNFG)

POSDAT2SIG_SRCS = \
	unemap/utilities/posdat2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

POSDAT2SIG_OBJSA = $(POSDAT2SIG_SRCS:.c=.o)
POSDAT2SIG_OBJSB = $(POSDAT2SIG_OBJSA:.cpp=.o)
POSDAT2SIG_OBJS = $(POSDAT2SIG_OBJSB:.f=.o)
BUILD_POSDAT2SIG = $(call BuildNormalTarget,posdat2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(POSDAT2SIG_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/posdat2sig$(TARGET_ABI_SUFFIX): $(POSDAT2SIG_OBJS)
	$(BUILD_POSDAT2SIG)

RATIO_SIGNALS_SRCS = \
	unemap/utilities/ratio_signals.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/analysis.c \
	unemap/analysis_calculate.c \
	unemap/rig.c \
	unemap/rig_node.c \
	user_interface/message.c

RATIO_SIGNALS_OBJSA = $(RATIO_SIGNALS_SRCS:.c=.o)
RATIO_SIGNALS_OBJSB = $(RATIO_SIGNALS_OBJSA:.cpp=.o)
RATIO_SIGNALS_OBJS = $(RATIO_SIGNALS_OBJSB:.f=.o)
BUILD_RATIO_SIGNALS = $(call BuildNormalTarget,ratio_signals$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(RATIO_SIGNALS_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/ratio_signals$(TARGET_ABI_SUFFIX): $(RATIO_SIGNALS_OBJS)
	$(BUILD_RATIO_SIGNALS)

REGISTER_SRCS = \
	unemap/utilities/register.c \
	command/parser.c \
	general/compare.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/unemap_hardware_client.c \
	unemap/rig.c \
	user_interface/event_dispatcher.c \
	user_interface/message.c

REGISTER_OBJSA = $(REGISTER_SRCS:.c=.o)
REGISTER_OBJSB = $(REGISTER_OBJSA:.cpp=.o)
REGISTER_OBJS = $(REGISTER_OBJSB:.f=.o)
BUILD_REGISTER = $(call BuildNormalTarget,register$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(REGISTER_OBJS),$(LIB))

$(UNEMAP_UTILITIES_PATH)/register$(TARGET_ABI_SUFFIX): $(REGISTER_OBJS)
	$(BUILD_REGISTER)

SIG2TEXT_SRCS = \
	unemap/utilities/sig2text.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/analysis.c \
	unemap/analysis_calculate.c \
	unemap/rig.c \
	unemap/rig_node.c \
	user_interface/message.c

SIG2TEXT_OBJSA = $(SIG2TEXT_SRCS:.c=.o)
SIG2TEXT_OBJSB = $(SIG2TEXT_OBJSA:.cpp=.o)
SIG2TEXT_OBJS = $(SIG2TEXT_OBJSB:.f=.o)
BUILD_SIG2TEXT = $(call BuildNormalTarget,sig2text$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(SIG2TEXT_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/sig2text$(TARGET_ABI_SUFFIX): $(SIG2TEXT_OBJS)
	$(BUILD_SIG2TEXT)

TEST_DELAUNAY_SRCS = \
	unemap/utilities/test_delaunay.c \
	general/debug.c \
	general/mystring.c \
	unemap/delaunay.c \
	user_interface/message.c

TEST_DELAUNAY_OBJSA = $(TEST_DELAUNAY_SRCS:.c=.o)
TEST_DELAUNAY_OBJSB = $(TEST_DELAUNAY_OBJSA:.cpp=.o)
TEST_DELAUNAY_OBJS = $(TEST_DELAUNAY_OBJSB:.f=.o)
BUILD_TEST_DELAUNAY = $(call BuildNormalTarget,test_delaunay$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(TEST_DELAUNAY_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/test_delaunay$(TARGET_ABI_SUFFIX): $(TEST_DELAUNAY_OBJS)
	$(BUILD_TEST_DELAUNAY)

TEST_UNEMAP_HARDWARE_SRCS = \
	unemap/utilities/test_unemap_hardware.c \
	general/compare.c \
	general/debug.c \
	general/mystring.c \
	unemap/unemap_hardware_client.c \
	user_interface/event_dispatcher.c \
	user_interface/message.c

TEST_UNEMAP_HARDWARE_OBJSA = $(TEST_UNEMAP_HARDWARE_SRCS:.c=.o)
TEST_UNEMAP_HARDWARE_OBJSB = $(TEST_UNEMAP_HARDWARE_OBJSA:.cpp=.o)
TEST_UNEMAP_HARDWARE_OBJS = $(TEST_UNEMAP_HARDWARE_OBJSB:.f=.o)
BUILD_TEST_UNEMAP_HARDWARE = $(call BuildNormalTarget,test_unemap_hardware$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(TEST_UNEMAP_HARDWARE_OBJS),$(LIB))

$(UNEMAP_UTILITIES_PATH)/test_unemap_hardware$(TARGET_ABI_SUFFIX): $(TEST_UNEMAP_HARDWARE_OBJS)
	$(BUILD_TEST_UNEMAP_HARDWARE)

TEXT2SIG_SRCS = \
	unemap/utilities/text2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/rig.c \
	unemap/rig_node.c \
	user_interface/message.c

TEXT2SIG_OBJSA = $(TEXT2SIG_SRCS:.c=.o)
TEXT2SIG_OBJSB = $(TEXT2SIG_OBJSA:.cpp=.o)
TEXT2SIG_OBJS = $(TEXT2SIG_OBJSB:.f=.o)
BUILD_TEXT2SIG = $(call BuildNormalTarget,text2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(TEXT2SIG_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/text2sig$(TARGET_ABI_SUFFIX): $(TEXT2SIG_OBJS)
	$(BUILD_TEXT2SIG)

TUFF2SIG_SRCS = \
	unemap/utilities/tuff2sig.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

TUFF2SIG_OBJSA = $(TUFF2SIG_SRCS:.c=.o)
TUFF2SIG_OBJSB = $(TUFF2SIG_OBJSA:.cpp=.o)
TUFF2SIG_OBJS = $(TUFF2SIG_OBJSB:.f=.o)
BUILD_TUFF2SIG = $(call BuildNormalTarget,tuff2sig$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(TUFF2SIG_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/tuff2sig$(TARGET_ABI_SUFFIX): $(TUFF2SIG_OBJS)
	$(BUILD_TUFF2SIG)

UNIQUE_CHANNELS_SRCS = \
	unemap/utilities/unique_channels.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

UNIQUE_CHANNELS_OBJSA = $(UNIQUE_CHANNELS_SRCS:.c=.o)
UNIQUE_CHANNELS_OBJSB = $(UNIQUE_CHANNELS_OBJSA:.cpp=.o)
UNIQUE_CHANNELS_OBJS = $(UNIQUE_CHANNELS_OBJSB:.f=.o)
BUILD_UNIQUE_CHANNELS = $(call BuildNormalTarget,unique_channels$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(UNIQUE_CHANNELS_OBJS),-lm)

$(UNEMAP_UTILITIES_PATH)/unique_channels$(TARGET_ABI_SUFFIX): $(UNIQUE_CHANNELS_OBJS)
	$(BUILD_UNIQUE_CHANNELS)

WRITE_CALIBRATION_SRCS = \
	unemap/utilities/write_calibration.c \
	command/parser.c \
	general/debug.c \
	general/geometry.c \
	general/matrix_vector.c \
	general/myio.c \
	general/mystring.c \
	unemap/interpolate.c \
	unemap/rig.c \
	user_interface/message.c

WRITE_CALIBRATION_OBJSA = $(WRITE_CALIBRATION_SRCS:.c=.o)
WRITE_CALIBRATION_OBJSB = $(WRITE_CALIBRATION_OBJSA:.cpp=.o)
WRITE_CALIBRATION_OBJS = $(WRITE_CALIBRATION_OBJSB:.f=.o)
BUILD_WRITE_CALIBRATION = $(call BuildNormalTarget,write_calibration$(TARGET_ABI_SUFFIX),$(UNEMAP_UTILITIES_PATH),$(WRITE_CALIBRATION_OBJS),-lm) 

$(UNEMAP_UTILITIES_PATH)/write_calibration$(TARGET_ABI_SUFFIX): $(WRITE_CALIBRATION_OBJS)
	$(BUILD_WRITE_CALIBRATION)

UTILITIES_SRCS = \
	$(WRITE_CALIBRATION_SRCS) \
	$(UNIQUE_CHANNELS_SRCS) \
	$(TUFF2SIG_SRCS) \
	$(TEXT2SIG_SRCS) \
	$(TEST_UNEMAP_HARDWARE_SRCS) \
	$(TEST_DELAUNAY_SRCS) \
	$(SIG2TEXT_SRCS) \
	$(REGISTER_SRCS) \
	$(RATIO_SIGNALS_SRCS) \
	$(POSDAT2SIG_SRCS) \
	$(PLT2CNFG_SRCS) \
	$(OPTICAL_SIGNALS_SRCS) \
	$(MAKE_PLAQUE_SRCS) \
	$(IMG2SIG_SRCS) \
	$(FIX_ECG_SRCS) \
	$(EXTRACT_SIGNAL_SRCS) \
	$(DIFFERENCE_SIGNALS_SRCS) \
	$(COMBINE_SIGNALS_SRCS) \
	$(CHANGE_FREQUENCY_SRCS) \
	$(CHANGE_EVENTS_SRCS) \
	$(CHANGE_CONFIGURATION_SRCS) \
	$(CHANGE_CHANNELS_SRCS) \
	$(CHANGE_CALIBRATION_SRCS) \
	$(CALCULATE_EVENTS_SRCS) \
	$(BIOSENSE2VRML_SRCS) \
	$(BIOSENSE2SIG_SRCS) \
	$(BMSI2SIG_SRCS) \
	$(AFFINE_SRCS) \
	$(ACTIVATION_SUMMARY_SRCS)

UTILITIES_OBJS = \
	$(WRITE_CALIBRATION_OBJS) \
	$(UNIQUE_CHANNELS_OBJS) \
	$(TUFF2SIG_OBJS) \
	$(TEXT2SIG_OBJS) \
	$(TEST_UNEMAP_HARDWARE_OBJS) \
	$(TEST_DELAUNAY_OBJS) \
	$(SIG2TEXT_OBJS) \
	$(REGISTER_OBJS) \
	$(RATIO_SIGNALS_OBJS) \
	$(POSDAT2SIG_OBJS) \
	$(PLT2CNFG_OBJS) \
	$(OPTICAL_SIGNALS_OBJS) \
	$(MAKE_PLAQUE_OBJS) \
	$(IMG2SIG_OBJS) \
	$(FIX_ECG_OBJS) \
	$(EXTRACT_SIGNAL_OBJS) \
	$(DIFFERENCE_SIGNALS_OBJS) \
	$(COMBINE_SIGNALS_OBJS) \
	$(CHANGE_FREQUENCY_OBJS) \
	$(CHANGE_EVENTS_OBJS) \
	$(CHANGE_CONFIGURATION_OBJS) \
	$(CHANGE_CHANNELS_OBJS) \
	$(CHANGE_CALIBRATION_OBJS) \
	$(CALCULATE_EVENTS_OBJS) \
	$(BIOSENSE2VRML_OBJS) \
	$(BIOSENSE2SIG_OBJS) \
	$(BMSI2SIG_OBJS) \
	$(AFFINE_OBJS) \
	$(ACTIVATION_SUMMARY_OBJS)

DEPENDFILE = $(OBJECT_PATH)/$(BIN_TARGET).depend

DEPEND_FILES = $(OBJS:%.o=%.d) $(MAIN_OBJ:%.o=%.d) $(UID2UIDH_OBJS:%.o=%.d) $(UTILITIES_OBJS:%.o=%.d)
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
$(DEPENDFILE) : $(DEPEND_FILES_MISSING)
	touch $(DEPENDFILE)

include $(DEPENDFILE)
include $(DEPEND_FILES_INCLUDE)

# **************************************************************************
# FILE : unemap.Makefile
#
# LAST MODIFIED : 27 November 2002
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

COMMONMAKEFILE := common.Makefile
COMMONMAKEFILE_FOUND = $(wildcard $(COMMONMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(COMMONMAKEFILE_FOUND),)
      COMMONMAKEFILE := $(PRODUCT_SOURCE_PATH)/$(COMMONMAKEFILE)
   endif # $(COMMONMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED
include $(COMMONMAKEFILE)

CELL = true

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

TARGET_EXECUTABLE_BASENAME = cell

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

ifneq ($(DYNAMIC_GL_LINUX),true)
   TARGET_DYNAMIC_GL_SUFFIX =
else # $(DYNAMIC_GL_LINUX) != true
   TARGET_DYNAMIC_GL_SUFFIX = -dynamicgl
endif # $(DYNAMIC_GL_LINUX) != true 

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

ifneq ($(SYSNAME),win32)
   TARGET_FILETYPE_SUFFIX =
else # $(SYSNAME) != win32)
   TARGET_FILETYPE_SUFFIX = .exe
endif # $(SYSNAME) != win32)

BIN_PATH=$(CMGUI_DEV_ROOT)/bin/$(BIN_ARCH_DIR)

BIN_TARGET = $(TARGET_EXECUTABLE_BASENAME)$(TARGET_ABI_SUFFIX)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DYNAMIC_GL_SUFFIX)$(TARGET_DEBUG_SUFFIX)$(TARGET_MEMORYCHECK_SUFFIX)$(TARGET_FILETYPE_SUFFIX)
OBJECT_PATH=$(CMGUI_DEV_ROOT)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
PRODUCT_OBJECT_PATH=$(PRODUCT_PATH)/object/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
DSO_PATH=$(CMGUI_DEV_ROOT)/dso/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
PRODUCT_DSO_PATH=$(PRODUCT_PATH)/dso/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)$(TARGET_USER_INTERFACE_SUFFIX)$(TARGET_DEBUG_SUFFIX)
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   UIDH_PATH=$(CMGUI_DEV_ROOT)/uidh/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)
   PRODUCT_UIDH_PATH=$(PRODUCT_PATH)/uidh/$(LIB_ARCH_DIR)/$(TARGET_EXECUTABLE_BASENAME)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

ifdef MAKECMDGOALS
   define BUILDING_MESSAGE
Making $(MAKECMDGOALS) for version $(BIN_TARGET)

   endef
else
   define BUILDING_MESSAGE
Building $(BIN_TARGET)

   endef
endif
$(warning $(BUILDING_MESSAGE))

$(BIN_TARGET) :

DEPENDFILE = $(OBJECT_PATH)/$(BIN_TARGET).depend
PRODUCT_DEPENDFILE = $(PRODUCT_OBJECT_PATH)/$(BIN_TARGET).depend

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
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

ifneq ($(USER_INTERFACE), GTK_USER_INTERFACE)
   #For GTK_USER_INTERFACE the OpenGL comes from the GTK_LIBRARIES automatically
   ifeq ($(SYSNAME),Linux)
      ifneq ($(DYNAMIC_GL_LINUX),true)
         GRAPHICS_LIB += -L/usr/local/lib
      else # $(DYNAMIC_GL_LINUX) != true
         GRAPHICS_LIB += -L/usr/X11R6/lib
      endif # $(DYNAMIC_GL_LINUX) != true 
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

ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   UIDH_INC = -I$(UIDH_PATH) -I$(PRODUCT_UIDH_PATH)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

STEREO_DISPLAY_DEFINES = -DSTEREO
# STEREO_DISPLAY_DEFINES =

#  By default some names are "mangled" to get external names <= 32 characters
#  NAME_DEFINES =
NAME_DEFINES = -DFULL_NAMES

USER_INTERFACE_INC = 
USER_INTERFACE_LIB =
ifeq ($(USER_INTERFACE),MOTIF_USER_INTERFACE)
   ifneq ($(DYNAMIC_GL_LINUX),true)
      ifneq ($(SYSNAME:IRIX%=),)
         USER_INTERFACE_INC += -I/usr/X11R6/include
      endif # SYSNAME != IRIX%=
      ifeq ($(SYSNAME),Linux)
         USER_INTERFACE_LIB += -lMrm -lXmu -lXm -lXp -lXt -lX11 -lXmu -lXext -lSM -lICE
      else # SYSNAME == Linux
         USER_INTERFACE_LIB += -lMrm -lXmu -lXm -lXt -lX11 -lXmu -lXext
         ifeq ($(SYSNAME:IRIX%=),)
            USER_INTERFACE_LIB += -lSgm
         endif # SYSNAME == IRIX%=
      endif # SYSNAME == Linux
  else # $(DYNAMIC_GL_LINUX) != true
      # Even though this is a dynamic executable I want to statically link as much
      # as possible so that the executable is as portable as possible
      X_LIB = /usr/X11R6/lib
      USER_INTERFACE_INC += -I/usr/X11R6/include
      USER_INTERFACE_LIB += $(X_LIB)/libMrm.a $(X_LIB)/libXm.a $(X_LIB)/libXt.a $(X_LIB)/libX11.a $(X_LIB)/libXmu.a $(X_LIB)/libXext.a $(X_LIB)/libXp.a $(X_LIB)/libSM.a $(X_LIB)/libICE.a
   endif # $(DYNAMIC_GL_LINUX) != true
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
ifeq ($(USER_INTERFACE),GTK_USER_INTERFACE)
   ifneq ($(SYSNAME),win32)
      #USE_GTK2 = true
      ifeq ($(USE_GTK2),true)
         USER_INTERFACE_INC += $(shell "/home/blackett/bin/pkg-config gtkgl-2.0 gtk+-2.0 --cflags")
         ifneq ($(DYNAMIC_GL_LINUX),true)
            USER_INTERFACE_LIB += $(shell "/home/blackett/bin/pkg-config gtkgl-2.0 gtk+-2.0 --libs")
         else # $(DYNAMIC_GL_LINUX) != true
            USER_INTERFACE_LIB += -L/home/blackett/lib -lgtkgl-2.0 -lgtk-x11-2.0 -lgdk-x11-2.0 -latk-1.0 -lgdk_pixbuf-2.0 -lm -lpangox-1.0 -lpango-1.0 -lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0 /home/blackett/lib/Mesa-4.0.4/libGLU.a /usr/lib/libGL.so
         endif # $(DYNAMIC_GL_LINUX) != true
      else # $(USE_GTK2) == true
         USER_INTERFACE_INC +=  -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include/
         USER_INTERFACE_LIB +=  -lgtkgl -lgtk -lgdk -lgmodule -lglib -ldl -lXi -lXext -lX11 -L/usr/X11R6/lib -lGLU -lGL
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
   LIB = -lPW -lftn -lm -lC -lCio -lpthread 
   DSO_LIB = -lftn -lm
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
   ifneq ($(DYNAMIC_GL_LINUX),true)
      LIB = -lg2c -lm -ldl -lpthread -lcrypt -lstdc++
      DSO_LIB = -lg2c -lm
   else # $(DYNAMIC_GL_LINUX) != true
      LIB = -lg2c -lm -ldl -lc /usr/lib/libpthread.a /usr/lib/libcrypt.a `ls -1 /usr/lib/libstdc++*.a | tail -1`
      DSO_LIB = -lg2c -lm
   endif # $(DYNAMIC_GL_LINUX) != true
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
   LIB = -lm -ldl -lSM -lICE -lpthread -lcrypt -lbsd -lld -lC128
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
   LIB = -lgdi32  -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -lnetapi32 -luuid -lwsock32 -lmpr -lwinmm -lversion -lodbc32
endif # SYSNAME == win32

UNEMAP_DEFINES = -DUNEMAP -DSPECTRAL_TOOLS
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
	unemap/drawing_2d.c \
	unemap/edf.c \
	unemap/eimaging_time_dialog.c \
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
	unemap/unemap_package.c \
	unemap/unemap_hardware_client.c

CELL_DEFINES = -DCELL
CELL_SRCS = \
  cell/header_test.c \
	cell/cell_calculate.c \
	cell/cell_calculate_dialog.c \
	cell/cell_cmgui_interface.c \
	cell/cell_cmiss_interface.c \
	cell/cell_component.c \
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
  cell/cell_model_routines/andre.f \
  cell/integrator_routines/euler.f \
  cell/integrator_routines/improved_euler.f \
  cell/integrator_routines/runge_kutta.f

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

ALL_DEFINES = $(COMPILE_DEFINES) $(TARGET_TYPE_DEFINES) \
	$(PLATFORM_DEFINES) $(OPERATING_SYSTEM_DEFINES) $(USER_INTERFACE_DEFINES) \
	$(STEREO_DISPLAY_DEFINES) $(UNEMAP_DEFINES) $(CELL_DEFINES) $(XML_DEFINES)\
	$(EXTERNAL_INPUT_DEFINES) \
	$(GRAPHICS_LIBRARY_DEFINES) $(NAME_DEFINES) $(TEMPORARY_DEVELOPMENT_FLAGS)

ALL_INCLUDES = $(SOURCE_DIRECTORY_INC) $(XML_INC) \
	$(UIDH_INC) $(USER_INTERFACE_INC)

ALL_FLAGS = $(OPTIMISATION_FLAGS) $(COMPILE_FLAGS) $(TARGET_TYPE_FLAGS) \
	$(ALL_DEFINES) $(ALL_INCLUDES)

ALL_LIB = $(XML_LIB) $(GRAPHICS_LIB) $(USER_INTERFACE_LIB) \
   $(IMAGEMAGICK_LIB) $(LIB)

APPLICATION_SRCS = 
CHOOSE_SRCS = \
	choose/choose_enumerator.c \
	choose/choose_texture.c \
	choose/chooser.c
COLOUR_SRCS = \
	colour/colour_editor.c \
	colour/edit_var.c 
COMFILE_SRCS = 
COMMAND_SRCS = \
	command/parser.c
COMPUTED_FIELD_SRCS =  \
	computed_field/computed_field.c \
	computed_field/computed_field_component_operations.c \
	computed_field/computed_field_coordinate.c \
	computed_field/computed_field_composite.c \
	computed_field/computed_field_fibres.c \
	computed_field/computed_field_find_xi.c \
	computed_field/computed_field_finite_element.c \
	computed_field/computed_field_set.c \
	computed_field/computed_field_value_index_ranges.c \
	computed_field/computed_field_wrappers.c
CURVE_SRCS = \
	curve/control_curve.c
DATA_SRCS = 
DOF3_SRCS =
ELEMENT_SRCS = 
FINITE_ELEMENT_SRCS = \
	finite_element/export_finite_element.c \
	finite_element/finite_element.c \
	finite_element/finite_element_adjacent_elements.c \
	finite_element/finite_element_discretization.c \
	finite_element/finite_element_time.c \
	finite_element/finite_element_to_graphics_object.c \
	finite_element/finite_element_to_iso_lines.c \
	finite_element/finite_element_to_streamlines.c \
	finite_element/import_finite_element.c
GENERAL_SRCS =  \
	general/any_object.c \
	general/callback.c \
	general/child_process.c \
	general/compare.c \
	general/debug.c \
	general/error_handler.c \
	general/geometry.c \
	general/heapsort.c \
	general/indexed_multi_range.c \
	general/image_utilities.c \
	general/machine.c \
	general/matrix_vector.c \
	general/multi_range.c \
	general/myio.c \
	general/mystring.c \
	general/postscript.c \
	general/statistics.c \
	general/value.c 
GRAPHICS_SRCS = \
	graphics/auxiliary_graphics_types.c \
	graphics/colour.c \
	graphics/complex.c \
	graphics/element_group_settings.c \
	graphics/element_point_ranges.c \
	graphics/environment_map.c \
	graphics/glyph.c \
	graphics/graphical_element.c \
	graphics/laguer.c \
	graphics/light.c \
	graphics/light_model.c \
	graphics/makegtobj.c \
	graphics/mcubes.c \
	graphics/material.c \
	graphics/graphics_library.c \
	graphics/graphics_object.c \
	graphics/graphics_window.c \
	graphics/import_graphics_object.c \
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
	graphics/texturemap.c \
	graphics/transform_tool.c \
	graphics/userdef_objects.c \
	graphics/volume_texture.c
INTERACTION_SRCS = \
	interaction/interaction_graphics.c \
	interaction/interaction_volume.c \
	interaction/interactive_tool.c \
	interaction/interactive_toolbar_widget.c \
	interaction/interactive_event.c \
	interaction/select_tool.c
IO_DEVICES_SRCS = \
	io_devices/conversion.c \
	io_devices/matrix.c 
LINK_SRCS = 
MATERIAL_SRCS =  \
	material/material_editor.c \
	material/material_editor_dialog.c
MENU_SRCS = 
MOTIF_INTERFACE_SRCS =  \
	motif/image_utilities.c
NODE_SRCS = 
PROJECTION_SRCS = 
PROMPT_SRCS = 
SELECT_SRCS = \
	select/select_environment_map.c \
	select/select_graphical_material.c \
	select/select_private.c \
	select/select_spectrum.c
SELECTION_SRCS = \
	selection/any_object_selection.c \
	selection/element_point_ranges_selection.c \
	selection/element_selection.c \
	selection/node_selection.c
SLIDER_SRCS = 
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
TRANSFORMATION_SRCS =
USER_INTERFACE_SRCS =  \
	user_interface/call_work_procedures.c \
	user_interface/confirmation.c \
	user_interface/event_dispatcher.c \
	user_interface/filedir.c \
	user_interface/message.c \
	user_interface/printer.c \
	user_interface/user_interface.c
VIEW_SRCS =

SRCS_1 = \
	$(APPLICATION_SRCS) \
	$(CELL_SRCS) \
	$(CHOOSE_SRCS) \
	$(COLOUR_SRCS) \
	$(COMFILE_SRCS) \
	$(COMMAND_SRCS) \
	$(COMPUTED_FIELD_SRCS) \
	$(CURVE_SRCS) \
	$(DATA_SRCS) \
	$(DOF3_SRCS) \
	$(ELEMENT_SRCS) \
	$(FINITE_ELEMENT_SRCS) \
	$(GENERAL_SRCS) \
	$(GRAPHICS_SRCS) \
	$(HELP_SRCS) \
	$(INTERACTION_SRCS) \
	$(IO_DEVICES_SRCS) \
	$(LINK_SRCS) \
	$(MATERIAL_SRCS) \
	$(MENU_SRCS) \
	$(MIRAGE_SRCS) \
	$(MOTIF_INTERFACE_SRCS) \
	$(NAG_SRCS) \
	$(NODE_SRCS) \
	$(PROJECTION_SRCS) \
	$(PROMPT_SRCS) \
	$(SELECT_SRCS) \
	$(SELECTION_SRCS) \
	$(SLIDER_SRCS) \
	$(THREE_D_DRAWING_SRCS)

SRCS_2 = \
	$(INTERPRETER_SRCS) \
	$(TIME_SRCS) \
	$(TRANSFORMATION_SRCS) \
	$(USER_INTERFACE_SRCS) \
	$(XVG_SRCS) \
	$(VIDEO_SRCS) \
	$(UNEMAP_SRCS) \
	$(VIEW_SRCS)

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

MAIN_SRC = cell.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)

clean :
	-rm -r $(OBJECT_PATH)
ifdef UIDH_PATH
		-rm -r $(UIDH_PATH)
endif # UIDH_PATH

clobber : clean
	-rm $(BIN_PATH)/Target

depend :
	if [ ! -d $(dir $(DEPENDFILE)) ]; then \
		mkdir -p $(dir $(DEPENDFILE)); \
	fi
	echo > $(DEPENDFILE)
	(makedepend -f $(DEPENDFILE) -o.o -Y -- $(ALL_FLAGS) -- $(MAIN_SRC) 2> $(DEPENDFILE).tmp)
	(makedepend -f $(DEPENDFILE) -o.o -Y -a -- $(ALL_FLAGS) -- $(SRCS_1) 2>> $(DEPENDFILE).tmp)
	(makedepend -f $(DEPENDFILE) -o.o -Y -a -- $(ALL_FLAGS) -- $(SRCS_2) 2>> $(DEPENDFILE).tmp)
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   # The uidh get their full path in the dependencies which stops the VPATH
   #.uil.uidh rule from picking up dependencies so I remove this PATH
	sed "s%$(UIDH_PATH)/%%g" $(DEPENDFILE) > $(DEPENDFILE).tmp2
	mv $(DEPENDFILE).tmp2 $(DEPENDFILE)
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
ifeq ($(SYSNAME:IRIX%=),)
	(ls $(SRCS) 2>&1 | sed "s%Cannot access %%;s%: No such file or directory%%;s%.*%&: &%;s%\.[^.]*:%.o:%;s%UX:ls: ERROR: %%" >> $(DEPENDFILE))
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
	(ls $(SRCS) 2>&1 | sed "s%ls: %%;s%: No such file or directory%%;s%.*%&: &%;s%\.[^.]*:%.o:%" >> $(DEPENDFILE))
endif # SYSNAME == Linux
ifeq ($(USER_INTERFACE), MOTIF_USER_INTERFACE)
   # Try and make a rule for the uidhs if they don't exist already,
   # It is bad, based on the format of the error output from makedepend and it only
   # gets the first inclusion.  If it fails then you can get the correct makedepend 
   # by ensuring all the uidh files already exist before you makedepend
	( grep uidh $(DEPENDFILE).tmp | grep makedepend | awk -v Obj=o -F "[ ,]" '{printf("%s.%s:",substr($$4, 1, length($$4) - 2),Obj); for(i = 1 ; i <= NF ; i++)  { if (match($$i,"uidh")) printf(" %s", substr($$i, 2, length($$i) -2)) } printf("\n");}' >> $(DEPENDFILE))
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE
	rm $(DEPENDFILE).tmp

$(OBJECT_PATH)/version.o.h : $(OBJS) cell.Makefile
	if [ ! -d $(OBJECT_PATH) ]; then \
		mkdir -p $(OBJECT_PATH); \
	fi	
	echo '/* This is a generated file.  Do not edit.  Edit cell.c or cell.Makefile instead */' > $(OBJECT_PATH)/version.o.h;	  
	date > date.h ;
	sed 's/"//;s/./#define VERSION "\\033\[0;32mCell version 001.002.001 &/;s/.$$/&\\nCopyright 1996-2002, Auckland UniServices Ltd.\\033\[0m"/' < date.h >> $(OBJECT_PATH)/version.o.h

$(MAIN_OBJ) : $(MAIN_SRC) $(OBJECT_PATH)/version.o.h
	@if [ -f $*.c ]; then \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $*.c > $(OBJECT_PATH)/$*.o.c ; \
	else \
		set -x; \
		cat $(OBJECT_PATH)/version.o.h $(PRODUCT_SOURCE_PATH)/$*.c > $(OBJECT_PATH)/$*.o.c ; \
	fi
	$(CC) -o $(OBJECT_PATH)/$*.o $(ALL_FLAGS) $(OBJECT_PATH)/$*.o.c;

$(BIN_TARGET) : $(OBJS) $(MAIN_OBJ)
	if [ ! -d $(BIN_PATH) ]; then \
		mkdir -p $(BIN_PATH); \
	fi
ifeq ($(SYSNAME:IRIX%=),)
	cd $(OBJECT_PATH) ; (ls $(OBJS) $(MAIN_OBJ) 2>&1 | sed "s%Cannot access %product_object/%;s%: No such file or directory%%;s%UX:ls: ERROR: %%" > object.list)
endif # SYSNAME == IRIX%=
ifeq ($(SYSNAME),Linux)
	cd $(OBJECT_PATH) ; (ls $(OBJS) $(MAIN_OBJ) 2>&1 | sed "s%ls: %product_object/%;s%: No such file or directory%%" > object.list)
endif # SYSNAME == Linux
ifeq ($(SYSNAME),AIX)
	cd $(OBJECT_PATH) ; (ls $(OBJS) $(MAIN_OBJ) 2>&1 | sed "s%ls: %product_object/%;s%: No such file or directory%%" > object.list)
endif # SYSNAME == AIX
ifeq ($(SYSNAME),win32)
	cd $(OBJECT_PATH) ; (ls $(OBJS) $(MAIN_OBJ) 2>&1 | sed "s%ls: %product_object/%;s%: No such file or directory%%" > object.list)
endif # SYSNAME == win32
   # Link in the OBJECT_PATH and copy to the BIN_PATH as often the OBJECT_PATH is kept locally.
	cd $(OBJECT_PATH) ; \
	rm -f product_object ; \
	ln -s $(PRODUCT_OBJECT_PATH) product_object ; \
	$(LINK) -o $(BIN_TARGET).exe $(ALL_FLAGS) `cat object.list` $(ALL_LIB) ; \
	cp $(BIN_TARGET).exe $(BIN_PATH)/$(BIN_TARGET)

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
      BUILD_UID2UIDH = $(call BuildNormalTarget,$(UID2UIDH),$(UTILITIES_PATH),UID2UIDH_OBJS,$(UID2UIDH_LIB))

      $(UID2UIDH_BIN): $(UID2UIDH_OBJS)
			$(BUILD_UID2UIDH)
   endif # $(UID2UIDH_FOUND != true
endif # $(USER_INTERFACE) == MOTIF_USER_INTERFACE

andre_dso : $(DSO_PATH)/andre.so

$(DSO_PATH)/andre.so :
	@if [ ! -d $(DSO_PATH) ]; then \
		mkdir -p $(DSO_PATH); \
	fi
	@set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o /tmp/andre.ObjSuffix $(SOURCE_DIRECTORY_INC) $(SOURCE_PATH)/cell/cell_model_routines/andre.f ;
	@set -x ; $(DSO_LINK) /tmp/andre.ObjSuffix $(DSO_LIB) -o $(DSO_PATH)/andre.so ;
	@rm -f /tmp/andre.ObjSuffix ;

hmt_dso : $(DSO_PATH)/hmt.so

$(DSO_PATH)/hmt.so : $(SOURCE_PATH)/cell/cell_model_routines/hmt.f
	@if [ ! -d $(DSO_PATH) ]; then \
		mkdir -p $(DSO_PATH); \
	fi
	@set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o /tmp/hmt.ObjSuffix $(SOURCE_DIRECTORY_INC) $(SOURCE_PATH)/cell/cell_model_routines/hmt.f ;
	@set -x ; $(DSO_LINK) /tmp/hmt.ObjSuffix $(DSO_LIB) -o $(DSO_PATH)/hmt.so ;
	@rm -f /tmp/hmt.ObjSuffix ;

euler_dso : $(DSO_PATH)/euler.so

$(DSO_PATH)/euler.so :
	@if [ ! -d $(DSO_PATH) ]; then \
		mkdir -p $(DSO_PATH); \
	fi
	@set -x ; $(FORTRAN) $(OPTIMISATION_FLAGS) $(TARGET_TYPE_FLAGS) -o /tmp/euler.ObjSuffix $(SOURCE_DIRECTORY_INC) $(SOURCE_PATH)/cell/integrator_routines/euler.f ;
	@set -x ; $(DSO_LINK) /tmp/euler.ObjSuffix $(DSO_LIB) -o $(DSO_PATH)/euler.so ;
	@rm -f /tmp/euler.ObjSuffix ;

# Still try and include both of them
sinclude $(PRODUCT_DEPENDFILE)
sinclude $(DEPENDFILE)

SUBDIRS = @Cmiss_Value @Cmiss_Value_Matrix @Cmiss_Value_Derivative_matrix @Cmiss_Variable

MEX = /product/local/matlab6.5/bin/mex -g
INCLUDE_DIRS = -I/dept/eng_sci/blackett/cmgui/source -I/dept/eng_sci/blackett/cmgui/source/matlab
DEFINES = 
#LIB_DIRS = -L/dept/eng_sci/blackett/lib64
LIB_DIRS = -L/dept/eng_sci/blackett/lib32

C_FILES = $(foreach dir,$(SUBDIRS),$(wildcard $(dir)/*.c $(dir)/private/*.c))
#OUTPUT_SUFFIX =  mexsg64
OUTPUT_SUFFIX =  mexsg
MEX_TARGETS = $(patsubst %.c,%.$(OUTPUT_SUFFIX),$(C_FILES))
LIBRARIES =

all: $(MEX_TARGETS)

.SUFFIXES:
.SUFFIXES: .c .$(OUTPUT_SUFFIX)

.c.$(OUTPUT_SUFFIX):
	cd $(dir $@) ; $(MEX) $(INCLUDE_DIRS) $(DEFINES) $(notdir $<) $(LIB_DIRS) $(LIBRARIES)

@Cmiss_Value/private/create.$(OUTPUT_SUFFIX): LIBRARIES=-lcmgui_computed_variable
@Cmiss_Value/private/get_string.$(OUTPUT_SUFFIX): LIBRARIES=-lcmgui_computed_variable
@Cmiss_Value_Matrix/private/create.$(OUTPUT_SUFFIX): LIBRARIES=-lcmgui_computed_variable
@Cmiss_Variable/private/create.$(OUTPUT_SUFFIX): LIBRARIES=-lcmgui_computed_variable
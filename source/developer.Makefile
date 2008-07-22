
#Some example uses of developer.Makefile, currently commented out.

#Specify my preferred default
#cmgui-wx-debug :

#Override variables in cmgui.Makefile by adding them to DEVELOPER_OPTIONS
#DEVELOPER_OPTIONS += TEMPORARY_DEVELOPMENT_FLAGS=-DENABLE_TEXTURE_TILING
#DEVELOPER_OPTIONS += WX_DIR=/home/blackett/cmiss/wxWidgets-2.8.6/bin/

#DEVELOPER_OPTIONS += WX_DEBUG_FLAG=no

#DEVELOPER_OPTIONS += USE_LIBGDCM=true

DEVELOPER_OPTIONS += COMPILER=msvc
#DEVELOPER_OPTIONS += PERL_INTERPRETER=false
#DEVELOPER_OPTIONS += IMAGEMAGICK=false
#DEVELOPER_OPTIONS += USE_XML2=false
#DEVELOPER_OPTIONS += USE_ITK=false
DEVELOPER_OPTIONS += ITK_BINDIR=$(CMISS_ROOT)/itk/builds/InsightToolkit-2.8.1/i386-win32-msvc


#DEVELOPER_OPTIONS += TMPDIR=C:/tmp


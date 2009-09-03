
#Some example uses of developer.Makefile, currently commented out.

#Specify my preferred default
cmgui-wx-debug :

#Override variables in cmgui.Makefile by adding them to DEVELOPER_OPTIONS
#DEVELOPER_OPTIONS += TEMPORARY_DEVELOPMENT_FLAGS=-DENABLE_TEXTURE_TILING
#DEVELOPER_OPTIONS += WX_DIR=/home/blackett/cmiss/wxWidgets-2.8.6/bin/
#DEVELOPER_OPTIONS += WX_DIR=/home/hsorby/development/cmiss/wxWidgets/bin/

#DEVELOPER_OPTIONS += WX_DEBUG_FLAG=no

#DEVELOPER_OPTIONS += USE_LIBGDCM=true

#DEVELOPER_OPTIONS += COMPILER=msvc
#DEVELOPER_OPTIONS += USE_PERL_INTERPRETER=false
#DEVELOPER_OPTIONS += USE_IMAGEMAGICK=false
#DEVELOPER_OPTIONS += USE_XML2=false
#DEVELOPER_OPTIONS += USE_ITK=false
#DEVELOPER_OPTIONS += ITK_BINDIR=$(CMISS_ROOT)/itk/builds/InsightToolkit-2.8.1/i386-win32-msvc

#DEVELOPER_OPTIONS += TMPDIR=C:/tmp

DEVELOPER_OPTIONS += WX_DIR=/home/andre/std-libs/wxWidgets-2.8.10/bin/
DEVELOPER_OPTIONS += ITK_SRCDIR=/home/andre/eBonz/cmiss/itk/include/InsightToolkit
DEVELOPER_OPTIONS += ITK_BINDIR=/home/andre/eBonz/cmiss/itk/lib/InsightToolkit
DEVELOPER_OPTIONS += ITK_INC="-I/home/andre/eBonz/cmiss/itk -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/Algorithms -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/BasicFilters -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/Common -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/Numerics/Statistics -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/Utilities/vxl/vcl -I/home/andre/eBonz/cmiss/itk/include/InsightToolkit/Utilities/vxl/core -I/home/andre/eBonz/cmiss/itk/Utilities/vxl/vcl -I/home/andre/eBonz/cmiss/itk/Utilities/vxl/core/"

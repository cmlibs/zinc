# Zinc Python package initialisation file.
__version__ = "@Zinc_VERSION@"
__revision__ = "@ZINC_DEVELOPER_VERSION@"

# The zinc.dll/zinc.dylib files are in the path where zinc got installed to;
# we need to add this to the PATH in order to have the import directive find the dependent dlls.
import os
os.environ['PATH'] = r'@NATIVE_PATH@' + os.pathsep + os.environ['PATH']

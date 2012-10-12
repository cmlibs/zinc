#**************************************************************************
#FILE : makefile

#LAST MODIFIED : 27 January 2009

#DESCRIPTION :

#makefile for wrapping the cmiss api file functions for 
#octave using swig.
#==========================================================================

make:
	swig -octave -o cmiss_command_data_wrap.cpp cmiss_command_data.i
	mkoctfile -I$(CMISS_ROOT)/cmgui/source -c cmiss_command_data_wrap.cpp 
	mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui cmiss_command_data_wrap.o -o cmiss_command_data.oct
	
	swig -octave -I$(CMISS_ROOT)/cmgui/source -o cmiss_region_wrap.cpp cmiss_region.i
	mkoctfile -I$(CMISS_ROOT)/cmgui/source -c cmiss_region_wrap.cpp 
	mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui cmiss_region_wrap.o -o cmiss_region.oct

	swig -octave -I$(CMISS_ROOT)/cmgui/source -o cmiss_field_wrap.cpp cmiss_field.i
	mkoctfile -I$(CMISS_ROOT)/cmgui/source -c cmiss_field_wrap.cpp 
	mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui cmiss_field_wrap.o -o cmiss_field.oct
	
	#swig -octave -I$(CMISS_ROOT)/cmgui/source -o cmiss_node_wrap.cpp cmiss_node.i
	#mkoctfile -I$(CMISS_ROOT)/cmgui/source -c cmiss_node_wrap.cpp 
	#mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui cmiss_node_wrap.o -o cmiss_node.oct

	swig -octave -I$(CMISS_ROOT)/cmgui/source -o cmiss_element_wrap.cpp cmiss_element.i
	mkoctfile -I$(CMISS_ROOT)/cmgui/source -c cmiss_element_wrap.cpp 
	mkoctfile -L$(CMISS_ROOT)/cmgui/lib/i686-linux/cmgui-wx-debug/ -lcmgui cmiss_element_wrap.o -o cmiss_element.oct

	
clean:
	rm -f cmiss_command_data.oct cmiss_command_data_wrap.o cmiss_command_data_wrap.cpp octave-core	

	rm -f cmiss_region.oct cmiss_region_wrap.o cmiss_region_wrap.cpp

	rm -f cmiss_field.oct cmiss_field_wrap.o cmiss_field_wrap.cpp
	
	#rm -f cmiss_node.oct cmiss_node_wrap.o cmiss_node_wrap.cpp

	rm -f cmiss_element.oct cmiss_element_wrap.o cmiss_element_wrap.cpp
	
	rm -f grid.exelem

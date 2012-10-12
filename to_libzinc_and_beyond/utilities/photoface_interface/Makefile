CFLAGS=-g
LDFLAGS=-g -lm
test_photoface: test_photoface.o libphotoface_cmiss.a

libphotoface_cmiss.a: photoface_cmiss.o photoface_cmiss_utilities.o
	ar rc libphotoface_cmiss.a photoface_cmiss.o photoface_cmiss_utilities.o jpeg_obj/*.o

CMGUI_ROOT=/home/blackett/cmgui
CMGUI_SOURCE=$(CMGUI_ROOT)/source
CMGUI_OBJECTS=$(CMGUI_ROOT)/object/cmgui/linux_g/
test_photoface_memorycheck: test_photoface_memorycheck.o libphotoface_cmiss_memorycheck.a
	cc -o test_photoface_memorycheck test_photoface_memorycheck.o libphotoface_cmiss_memorycheck.a $(CMGUI_OBJECTS)general/debug_memory_check.o $(CMGUI_OBJECTS)user_interface/message.o $(CMGUI_OBJECTS)general/compare.o $(LDFLAGS) 

libphotoface_cmiss_memorycheck.a: photoface_cmiss_memorycheck.o photoface_cmiss_utilities.o
	ar rc libphotoface_cmiss_memorycheck.a photoface_cmiss_memorycheck.o photoface_cmiss_utilities.o jpeg_obj/*.o

photoface_cmiss_memorycheck.o: photoface_cmiss.c photoface_cmiss.h
	cc -I $(CMGUI_SOURCE) -DMEMORY_CHECKING -c -o photoface_cmiss_memorycheck.o photoface_cmiss.c

test_photoface_memorycheck.o: test_photoface.c photoface_cmiss.h
	cc -I $(CMGUI_SOURCE) -DMEMORY_CHECKING -c -o test_photoface_memorycheck.o test_photoface.c


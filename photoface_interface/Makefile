CFLAGS=-g
LDFLAGS=-g
test_photoface: test_photoface.o libphotoface_cmiss.a

libphotoface_cmiss.a: photoface_cmiss.o photoface_cmiss_utilities.o
	ar rc libphotoface_cmiss.a photoface_cmiss.o photoface_cmiss_utilities.o jpeg_obj/*.o
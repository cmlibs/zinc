
#CFLAGS = -DIBM -DGL_API
CFLAGS = -g -DGL_API

#LIB = -lMrm -lXm -lXt -lX11 -lgl -lPW -lm
LIB = -lgl_s -limage -lgutil -lXirisw -lMrm -lXm_s -lXt_s -lX11_s -lPW -lm

OBJS =  drawing.o drawuserdef.o face_window.o fetogt.o finite_element.o \
	finite_element_to_graphics_object.o geometry.o gmain.o graphics_interface.o \
	graphics_library.o graphics_object.o graphics_shell.o graphics_window.o \
	gtbasis.o import_finite_element.o loaddata.o makegtobj.o matrix.o \
	rendergl.o renderloop.o robo_window.o scrsave.o smtvcmd.o \
	../general/debug.o ../general/mystring.o

all : cm3d graphics_window.uid face_window.uid robo_window.uid

cm3d : $(OBJS)
	cc -o cm3d $(OBJS) $(LIB)

drawing.o : drawing.c drawing.h graphics_library.h graphics_window.h \
../general/debug.h ../general/system.h ../general/user_interface.h

drawuserdef.o :  drawuserdef.c graphics_object.h

face_window.o : face_window.c drawing.h face_window.h graphics_library.h \
graphics_window.h ../general/debug.h ../general/system.h \
../general/user_interface.h

fetogt.o :  fetogt.c fetogt.h finite_element.h graphics_object.h

finite_element.o :  finite_element.c finite_element.h ../general/debug.h \
../general/system.h ../general/user_interface.h

finite_element_to_graphics_object.o : finite_element_to_graphics_object.c \
finite_element_to_graphics_object.h finite_element.h geometry.h \
graphics_object.h ../general/debug.h ../general/system.h

geometry.o : geometry.c geometry.h ../general/debug.h ../general/system.h

gmain.o :  gmain.c drawing.h fetogt.h finite_element.h \
finite_element_to_graphics_object.h graphics_object.h global_database.h \
graphics_library.h graphics_window.h import_finite_element.h \
../general/debug.h ../general/system.h ../general/user_interface.h

graphics_interface.o : graphics_interface.c global_database.h \
graphics_interface.h graphics_library.h graphics_window.h ../general/debug.h \
../command/command_window.h ../general/system.h

graphics_library.o : graphics_library.c graphics_library.h ../general/debug.h

graphics_shell.o : graphics_shell.c graphics_interface.h \
../general/debug.h ../command/command_window.h ../general/system.h \
../general/user_interface.h

graphics_window.o : graphics_window.c drawing.h graphics_window.h \
../general/debug.h ../general/system.h ../general/user_interface.h

gtbasis.o :  gtbasis.c finite_element.h

graphics_object.o : graphics_object.c graphics_object.h ../general/debug.h \
../general/system.h

import_finite_element.o :  import_finite_element.c finite_element.h \
import_finite_element.h ../general/debug.h ../general/mystring.h \
../general/system.h ../general/user_interface.h

loaddata.o :  loaddata.c graphics_object.h global_database.h \
../general/debug.h ../general/system.h ../general/user_interface.h

makegtobj.o : makegtobj.c graphics_object.h makegtobj.h ../general/debug.h \
../general/system.h

matrix.o :  matrix.c graphics_object.h

rendergl.o :  rendergl.c graphics_object.h

renderloop.o :  renderloop.c drawing.h graphics_library.h makegtobj.h \
../general/debug.h ../general/system.h ../general/user_interface.h

robo_window.o : robo_window.c drawing.h graphics_library.h graphics_window.h \
robo_window.h ../general/debug.h ../general/system.h ../general/user_interface.h

scrsave.o : scrsave.c port.h image.h

smtvcmd.o : smtvcmd.c

../general/debug.o : ../general/debug.c
	cc -c ../general/debug.c -o debug.o
	mv debug.o ../general

../general/mystring.o : ../general/mystring.c
	cc -c ../general/mystring.c -o mystring.o
	mv mystring.o ../general

drawing.h : geometry.h graphics_library.h graphics_object.h
	touch -c drawing.h

fetogt.h : finite_element.h graphics_object.h
	touch -c fetogt.h

finite_element.h : geometry.h
	touch -c finite_element.h

finite_element_to_graphics_object : finite_element.h graphics_object.h
	touch -c finite_element_to_graphics_object.h

global_database.h : drawing.h finite_element.h graphics_library.h \
graphics_object.h
	touch -c global_database.h

graphics_interface.h : graphics_window.h
	touch -c graphics_interface.h

graphics_window.h : drawing.h
	touch -c graphics_window.h

graphics_object.h : geometry.h graphics_library.h
	touch -c graphics_object.h

face_window.uid : face_window.uil
	uil face_window.uil -o face_window.uid

graphics_window.uid : graphics_window.uil
	uil graphics_window.uil -o graphics_window.uid

robo_window.uid : robo_window.uil
	uil robo_window.uil -o robo_window.uid

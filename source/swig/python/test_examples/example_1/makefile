# example: wrap an entire header

make: 
	swig -python -c++ egfunc.i
	g++ -c -Wall egfunc_wrap.cxx egfunc.cc -I/usr/include/python2.5
	g++ -shared egfunc_wrap.o egfunc.o -o _egfunc.so

clean:
	rm -f egfunc.o egfunc.py egfunc.pyc _egfunc.so egfunc_wrap.cxx egfunc_wrap.o


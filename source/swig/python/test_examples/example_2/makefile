# example: wrap an one function in a script

make: 
	swig -python -c++ egarith.i
	g++ -c -Wall egarith_wrap.cxx egfunc.cc -I/usr/include/python2.5
	g++ -shared egarith_wrap.o egfunc.o -o _egarith.so

clean:
	rm -f egfunc.o egarith.py egarith.pyc _egarith.so egarith_wrap.cxx egarith_wrap.o

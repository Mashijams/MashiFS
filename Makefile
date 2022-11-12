MashiFS: fssh.o disk.o FS.o
	g++ fssh.o disk.o FS.o -o MashiFS

fssh.o: fssh.cpp disk.h FS.h types.h
	g++ -c fssh.cpp

disk.o: disk.cpp disk.h
	g++ -c disk.cpp

FS.o: FS.cpp FS.h
	g++ -c FS.cpp

clean:
	rm *.o MashiFS
BLUE='\033[1;34m'
GREEN='\033[1;32m'
CYAN='\033[0;36m'

MashiFS: fssh.o disk.o FS.o
	g++ fssh.o disk.o FS.o -o MashiFS
	$(info Successfully compiled MashiFS)
	$(info To create new disk with MashiFS use : Create (Diskname) (TotalBlocks))
	$(info To mount an existing disk with MashiFS use : Mount (Diskname))

fssh.o: fssh.cpp disk.h FS.h types.h
	g++ -c fssh.cpp

disk.o: disk.cpp disk.h
	g++ -c disk.cpp

FS.o: FS.cpp FS.h
	g++ -c FS.cpp

clean:
	rm *.o MashiFS
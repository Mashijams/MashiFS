BLUE='\033[1;34m'
GREEN='\033[1;32m'
CYAN='\033[0;36m'

MashiFS: fssh.o disk.o FS.o
	g++ fssh.o disk.o FS.o -o MashiFS
	$(info Successfully compiled MashiFS)
	$(info To create new disk with MashiFS use : Create (Diskname) (TotalBlocks))
	$(info To mount an existing disk with MashiFS use : Mount (Diskname))

fssh.o: src/fssh.cpp include/disk.h include/FS.h include/types.h
	g++ -c src/fssh.cpp

disk.o: src/disk.cpp include/disk.h
	g++ -c src/disk.cpp

FS.o: src/FS.cpp include/FS.h
	g++ -c src/FS.cpp

clean:
	rm *.o MashiFS
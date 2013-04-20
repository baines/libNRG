CXXFLAGS := -I../../include
LDFLAGS := ../../libnrg.a

all: entityhelper_test socket_address_test

entityhelper_test: entityhelper_test.cpp ../../libnrg.a
	g++ $(CXXFLAGS) $< $(LDFLAGS) -o $@ -lrt -lz
	
socket_address_test: socket_address_test.cpp ../../libnrg.a
	g++ $(CXXFLAGS) $< $(LDFLAGS) -o $@ -lrt -lz

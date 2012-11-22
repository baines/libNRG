CXXFLAGS := -I../include
LDFLAGS := ../libnrg.a

all: test

test: test.cpp ../libnrg.a
	g++ $(CXXFLAGS) test.cpp $(LDFLAGS) -o test -lrt

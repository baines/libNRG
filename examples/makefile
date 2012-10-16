CXXFLAGS := -I../include
LDFLAGS := -L.. -l:libnrg.a

all: test

test: test.cpp
	g++ $(CXXFLAGS) test.cpp $(LDFLAGS) -o test

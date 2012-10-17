CXXFLAGS := -I../include
LDFLAGS := -L.. -l:libnrg.a

all: test

test: test.cpp ../libnrg.a
	g++ $(CXXFLAGS) test.cpp $(LDFLAGS) -o test

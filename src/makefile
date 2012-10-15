builddir := ../build
libname := libnrg.so
headers := $(wildcard *.h)
sources := $(wildcard *.cpp)
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := -I../include

CXX := g++
CXXFLAGS := -DPIC -fPIC -ffast-math -fno-exceptions -fno-rtti -U_FORTIFY_SOURCE
LDFLAGS := -shared -Wl,-soname,$(libname).0

all: ../$(libname)

../$(libname): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

$(builddir)/%.o: %.cpp $(headers)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(includes) -c $< -o $@

clean:
	$(RM) $(objects) ../$(libname)

.PHONY: all clean


builddir := ../build
libname := libnrg
libname_shared := $(libname).so
libname_static := $(libname).a
headers := $(wildcard *.h)
sources := $(wildcard *.cpp)
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := -I../include

CXX := g++
CXXFLAGS := -DPIC -fPIC -ffast-math -fno-exceptions -fno-rtti -U_FORTIFY_SOURCE
LDFLAGS := -shared -Wl,-soname,$(libname_shared).0

all: ../$(libname_shared) ../$(libname_static)

../$(libname_shared): $(objects)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@

../$(libname_static): $(objects)
	$(AR) rcs $@ $^

$(builddir)/%.o: %.cpp $(headers)
	mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(includes) -c $< -o $@

clean:
	$(RM) $(objects) ../$(libname_shared) ../$(libname_static)

.PHONY: all clean


builddir := ../build
libname := libnrg
libname_shared := $(libname).so.0
libname_static := $(libname).a
headers := $(wildcard *.h) $(wildcard ../include/*.h)
sources := $(wildcard *.cpp)
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := -I../include

CXX := g++-4.8
CXXFLAGS := -pedantic -std=c++11 -g -Wall -fPIC -DBUILDING_NRG -fvisibility=hidden -ffast-math \
-fno-exceptions -U_FORTIFY_SOURCE -fno-stack-protector -fstrict-aliasing
LDFLAGS := -lrt -lz -shared -Wl,-soname,$(libname_shared)

all: ../$(libname_shared) ../$(libname_static)

../$(libname_shared): $(objects)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

../$(libname_static): $(objects)
	$(AR) rcs $@ $^

$(builddir)/%.o: %.cpp $(headers)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(includes) -c $< -o $@

clean:
	$(RM) $(objects) ../$(libname_shared) ../$(libname_static)

.PHONY: all clean


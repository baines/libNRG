ifdef WINDOWS
    suffix_shared := dll
    suffix_static := mingw.a
    CXX := i686-w64-mingw32-g++
    LDFLAGS := -shared -lws2_32
else
    CXX := g++
    CXXFLAGS := -fPIC
    #-Wl,--version-script=../nrg_symbols.map
    suffix_shared := so.0
    suffix_static := a
    LDFLAGS := -lrt -lz -shared -Wl,-soname,$(libname_shared)
endif

builddir := ../build
libname := libnrg
libname_shared := $(libname).$(suffix_shared)
libname_static := $(libname).$(suffix_static)
headers := $(wildcard *.h) $(wildcard ../include/*.h)
sources := $(wildcard *.cpp)
objects := $(patsubst %.cpp,$(builddir)/%.o,$(sources))
includes := -I../include

CXXFLAGS += -Wuninitialized -pedantic -Wsign-promo -std=c++11 -g -Wall -DBUILDING_NRG \
-ffast-math -fvisibility-inlines-hidden -fno-exceptions \
-U_FORTIFY_SOURCE -fno-stack-protector -fstrict-aliasing 

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


This is the code for LibNRG - Networking for Real-time Games which was created
by myself, Alex Baines for my third year computer science project at Warwick
University.

This library provides networking functionality than can be used to create 
video-games which are playable over networks such as the internet.
For an example of one such game look in the examples/ballduel directory.

You can also get the specification and final report I produced about LibNRG at 
university here: http://abaines.me.uk/other/libnrg/ .

Header files for the library are stored in the include/ directory, and .cpp
source files along with private header files are stored in the src/ directory.

LibNRG currently only compiles on Linux, and requires zlib unless you comment
out the #define NRG_ENABLE_ZLIB_COMPRESSION line in include/nrg_config.h. It
also requires a recent compiler with c++11 support, like gcc-4.7 or greater.

Speaking of zlib, LibNRG is licensed under the zlib open source license, so you
can use it in commercial games, statically link it, or make modifications under
the terms of this license. See LICENSE.txt in this directory for more details.

There's still a lot of things I would like to change / improve with the library
so I wouldn't recommend using it in any serious projects yet. There are also a
few bits that are currently broken like reassembling packets and replay files.


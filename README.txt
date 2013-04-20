This is the code for LibNRG - Networking for Real-time Games which was created
by myself, Alex Baines for my third year computer science project at Warwick
University.

This library provides networking functionality than can be used to create 
video-games which are playable over networks such as the internet.
For an example of one such game look in the examples/ballduel directory.

Header files for the library are stored in the include/ directory, and .cpp
source files along with private header files are stored in the src/ directory.

To compile the library simply run make - If you're reading this file on the CD
release of LibNRG that accompanies my final report then firstly, let me say
that you are looking great today ;) - but more importantly you'll need to copy
everything off the CD to a writable directory if you want to compile the code.

LibNRG currently only compiles on Linux, and requires zlib unless you comment
out the #define NRG_ENABLE_ZLIB_COMPRESSION line in include/nrg_config.h.
Speaking of zlib, LibNRG is licensed under the zlib open source license, so you
can use it in commercial games, statically link it, or make modifications under
the terms of this license. See LICENSE.txt in this directory for more details.

That's it, bye!
-- Alex.


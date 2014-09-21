Introduction
------------

This is the code for LibNRG (Networking for Real-time Games) which was created
by myself, Alex Baines, for my third year computer science project at Warwick
University.

This library provides a high-level of abstraction over UDP networking that can
be used to implement client / server online multi-player in video games.

LibNRG currently compiles on Linux or Windows (via mingw). It requires a recent 
compiler with c++11 support, like gcc-4.7 or newer, or i686-w64-mingw32-g++ 
v4.7+ for windows.

There's still a lot of limitations and things I would like to change / improve 
with the library, so I wouldn't recommend using it in any *serious* projects yet.

Documentation
-------------

There is an initial Doxygen API reference available at **http://abaines.me.uk/libNRG/**
however it is currently rather incomplete.

There's a full working example game too in the examples/ballduel directory,
that's probably of more use at the moment.

Basic Usage
-----------

You basically make a class that inherits from nrg::Entity or EntityHelper, put
some nrg::Fields in it, and then the server will automatically send any changes
you make to those fields to its clients.

e.g.

    class MyClass : public nrg::EntityHelper<MyClass, SOME_ID_NUMBER> {
    	nrg::Field<int> int_field;
    	nrg::Field<string> string_field;
    	nrg::Field<int[10]> arrays_work_too;
    	[ e.t.c. ]
    };

There are also nrg::Messages which are more like traditional one-shot packets, 
check out the examples directory for more complete example code.

License
--------

It's **zlib** licensed, so statically linking / modifying / redistributing is fine
as long as you don't claim you made the original version, and you call your
version something else. See LICENSE.txt for the proper license text.

I'm also happy to take pull requests / patches / suggestions.

Other stuff worth mentioning
----------------------------

+ Loosely based on Quake III Arena's style of networking.
+ Client runs 1 tick behind the server and can interpolate between the latest 2
snapshots it recieved.
+ Automatic endian conversion.
+ Variable length integer support.
+ Only sends changes in data when necessary.
+ IPv6 aware.
+ Low latency, uses SO_TIMESTAMP for better packet timing accuracy.
+ Can capture client-side replays (but this was broken with protocol changes and I need to fix it!).
+ Handles ICMP errors.
+ C++11 template wizardry.
+ Different packet exchanges like handshaking, playing, e.t.c are each encapsulated
in their own State class. This still needs fleshing out, but in the future it could
support user-created states like lobbies / multiple games per server.


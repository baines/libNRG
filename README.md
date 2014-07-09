Introduction
------------

This is the code for LibNRG (Networking for Real-time Games) which was created
by myself, Alex Baines, for my third year computer science project at Warwick
University.

This library provides a high-level of abstraction over UDP networking that can
be used to implement client / server online multi-player in video games.

There isn't proper documentation for it yet, but there is a full working example
game in the examples/ballduel directory. You can also get a good overview from 
the LibNRG specification and final report that I produced at university here:
http://abaines.me.uk/other/libnrg/ .

LibNRG currently only compiles on Linux, and requires zlib unless you comment
out the #define NRG_ENABLE_ZLIB_COMPRESSION line in include/nrg_config.h. It
also requires a recent compiler with c++11 support, like gcc-4.7 or newer.

The library is also licensed under the zlib open source license, so you can use 
it in commercial games, statically link it, or make modifications under the 
terms of this license. See LICENSE.txt in this directory for more details.

There's still a lot of limitations and things I would like to change / improve 
with the library, so I wouldn't recommend using it in any serious projects yet.

Using the entity abstraction
----------------------------

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

There are also nrg::Messages which are more like traditional one-shot packets.

Other stuff worth mentioning
----------------------------

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


This is the example game that was created alongside LibNRG.

Compile it using GNU make with the supplied makefile.

LibSFML development headers are required to compile the client program 
(on Debian/Ubuntu apt-get install libsfml-dev, or download them from 
http://www.sfml-dev.org/).

To the run the program, launch ./game_server & followed by ./game_client.
game_client requires the libsfml runtime libraries to run, get them the same
place you got the development headers.

The screen should display two paddles and a stationary ball. Move the mouse up
and down over the window to move your paddle. Launch another ./game_client and 
the game will begin, with the second window controlling the left paddle.

An IP address or hostname can be supplied as the first shell argument to
./game_client to connect it to a server running on another host.

If the first argument is the string "replay" followed by a filename as a second
argument, then the client will play back the replay. Note that the client will
need to bind to port 9001 for this to work, so don't run a ./game_server on the
same host when trying to watch a replay.

In order to record a replay file, supply an IP / hostname to connect to as the
first argument and a filename as a second. A replay file will be recorded to the
filename given.

In the bottom right corner a lag-o-meter is displayed, this can be interpreted
as described on https://en.wikipedia.org/wiki/Lagometer. 

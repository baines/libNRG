This program tests the interpolation of LibNRG.

Compile it using GNU make with the supplied makefile.

LibSFML development headers are required to compile the client program 
(on Debian/Ubuntu apt-get install libsfml-dev, or download them from 
http://www.sfml-dev.org/).

To the run the program, launch ./game_server & followed by ./game_client.
game_client requires the libsfml runtime libraries to run, get them the same
place you got the development headers.

The screen should display a square that moves in a circular orbit. Press the "i"
key in order to toggle the interpolation on or off.

/*
  LibNRG - Networking for Real-time Games
  
  Copyright (C) 2012-2014 Alex Baines <alex@abaines.me.uk>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
/** @file
 *  Contains the PacketHeader class
 */
#ifndef NRG_PACKET_HEADER_H
#define NRG_PACKET_HEADER_H

namespace nrg {

/** Class representing a header to be prepended to packets that pass through ConnectionIn and ConnectionOut */
struct PacketHeader {
	/** Default constructor */
	PacketHeader()
	: seq_num(0)
	, flags(0)
	, version(0)
	, frag_index(0){
	
	}
	
	/** Constructor specifying all attributes */
	PacketHeader(uint16_t seq, uint8_t flags, uint8_t frag_index = 0)
	: seq_num(seq)
	, flags(flags)
	, version(0)
	, frag_index(frag_index){
	
	}

	/** Read a header from a packet \p p, returning true if successful */
	bool read(Packet& p){
		if(p.remaining() >= size){
			uint8_t ver_idx = 0;
			p.read8(ver_idx).read8(flags).read16(seq_num);
			
			version = ver_idx >> 7;
			frag_index = ver_idx & 0x1F;
			return true;
		} else {
			return false;
		}
	}
	
	/** Write this PacketHeader instance to the given packet \p p */
	void write(Packet& p){
		p.write8((version << 7) | (frag_index & 0x1F));
		p.write8(flags);
		p.write16(seq_num);
	}

	uint16_t seq_num;
	uint8_t flags;
	uint32_t version, frag_index;
	
	static const size_t size = 4;
};

}

#endif

/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008  Andrey Stroganov

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PACKETS_HPP
#define PACKETS_HPP

#include "../staticname.hpp"
#include "packettype.hpp"

namespace G2
{
	const PacketType NONE = 	0;
	const PacketType D = 		StaticName::Encoder<'D'>::result;
	const PacketType DN = 		StaticName::Encoder<'D', 'N'>::result;
	const PacketType FR = 		StaticName::Encoder<'F', 'R'>::result;
	const PacketType GU = 		StaticName::Encoder<'G', 'U'>::result;
	const PacketType H = 		StaticName::Encoder<'H'>::result;
	const PacketType LNI = 		StaticName::Encoder<'L', 'N', 'I'>::result;
	const PacketType NA = 		StaticName::Encoder<'N', 'A'>::result;
	const PacketType NH = 		StaticName::Encoder<'N', 'H'>::result;
	const PacketType NICK =	 	StaticName::Encoder<'N', 'I', 'C', 'K'>::result;
	const PacketType PI = 		StaticName::Encoder<'P', 'I'>::result;
	const PacketType PO = 		StaticName::Encoder<'P', 'O'>::result;
	const PacketType PUSH = 	StaticName::Encoder<'P', 'U', 'S', 'H'>::result;
	const PacketType Q2 = 		StaticName::Encoder<'Q', '2'>::result;
	const PacketType QA = 		StaticName::Encoder<'Q', 'A'>::result;
	const PacketType QH2 =		StaticName::Encoder<'Q', 'H', '2'>::result;
	const PacketType QHT =	 	StaticName::Encoder<'Q', 'H', 'T'>::result;
	const PacketType QK = 		StaticName::Encoder<'Q', 'K'>::result;
	const PacketType QKA = 		StaticName::Encoder<'Q', 'K', 'A'>::result;
	const PacketType QKR = 		StaticName::Encoder<'Q', 'K', 'R'>::result;
	const PacketType QNA = 		StaticName::Encoder<'Q', 'N', 'A'>::result;
	const PacketType RNA = 		StaticName::Encoder<'R', 'N', 'A'>::result;
	const PacketType SNA =	 	StaticName::Encoder<'S', 'N', 'A'>::result;
	const PacketType S = 		StaticName::Encoder<'S'>::result;
	const PacketType SZ = 		StaticName::Encoder<'S', 'Z'>::result;
	const PacketType SZR = 		StaticName::Encoder<'S', 'Z', 'R'>::result;
	const PacketType TO = 		StaticName::Encoder<'T', 'O'>::result;
	const PacketType UDP = 		StaticName::Encoder<'U', 'D', 'P'>::result;
	const PacketType UPRO =	 	StaticName::Encoder<'U', 'P', 'R', 'O'>::result;
	const PacketType URL = 		StaticName::Encoder<'U', 'R', 'L'>::result;
	const PacketType URN = 		StaticName::Encoder<'U', 'R', 'N'>::result;
	const PacketType RA = 		StaticName::Encoder<'R', 'A'>::result;
	const PacketType RELAY = 	StaticName::Encoder<'R', 'E', 'L', 'A', 'Y'>::result;
	const PacketType V = 		StaticName::Encoder<'V'>::result;

} //namespace G2

#endif //PACKETS_HPP



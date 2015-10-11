/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2009  Andrey Stroganov

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

#ifndef TRANSCEIVER_HPP
#define TRANSCEIVER_HPP
#include "types.hpp"
#include "io.hpp"
#include "trafmanager.hpp"
#include "departurecache.hpp"
#include "arrivalcache.hpp"

namespace G2 { class Packet; }

class Transceiver
{
public:
	Transceiver(ba::io_service&);
	void Start(uint16 port);
	void Send(const G2::Packet& packet, Ip::Endpoint endpoint);

private:
	void OnTimer();
	void Dispatch();
	void OnReceived(const bs::error_code& err, std::size_t bytesReceived);
	void OnSent(const bs::error_code& err, std::size_t bytesSent);
	void StartReceive();

	enum {BUFFER_SIZE = 1024};
	char receiveBuffer_[BUFFER_SIZE];

	char sendBuffer_[udp::MTU];

	ba::ip::udp::socket socket_;
	ba::deadline_timer timer_;

	DepartureCache departure_;
	ArrivalCache arrival_;
	bool sending_;
	ba::ip::udp::endpoint sender_;
	bool throttle_;
	TrafManager traffin_;
};

#endif //TRANSCEIVER_HPP

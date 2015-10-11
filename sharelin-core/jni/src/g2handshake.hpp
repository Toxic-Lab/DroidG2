/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov

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

#ifndef G2HANDSHAKE_HPP
#define G2HANDSHAKE_HPP

#include <boost/enable_shared_from_this.hpp>

#include "io.hpp"
#include "ip.hpp"

class G2Handshake: public boost::enable_shared_from_this<G2Handshake>
{
public:
	G2Handshake();
	void Connect(Ip::Endpoint);
	TcpSocketPtr GetSocket() const { return pSocket_; }
	Ip::Address SelfAddress() const { return selfAddress_; }
	Ip::Endpoint GetEndpoint() const { return endpoint_; }
	bool Connecting() const { return connecting_; }
	void Close();
	bool Alive() const { return alive_; }

private:
	void OnConnected(const bs::error_code& err);
	void OnGreetingSent(const bs::error_code& err);
	void OnGreetingResponse(const bs::error_code& err);
	void OnG2Connected(const bs::error_code& err);
	void OnTimeout(const bs::error_code& err);
	void DetachMe();
	void HandleReply();

	TcpSocketPtr pSocket_;
	ba::deadline_timer timer_;
	ba::streambuf buffer_;
	bool connecting_;
	bool closed_;
	Ip::Endpoint endpoint_;
	Ip::Address selfAddress_;
	bool alive_;
};

#endif //G2HANDSHAKE_HPP


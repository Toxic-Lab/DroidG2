/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2010  Andrey Stroganov <savthe@gmail.com>

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

#ifndef GATEKEEPER_HPP
#define GATEKEEPER_HPP

#include "io.hpp"
#include "boost/enable_shared_from_this.hpp"
#include <string>
#include <vector>
#include "ip.hpp"
#include <iostream>
#include <stdexcept>

class Gatekeeper: public boost::enable_shared_from_this<Gatekeeper>
{
public:
	typedef boost::shared_ptr< Gatekeeper > Ptr;
	Gatekeeper();
	void Push(Ip::Endpoint);
	ba::ip::tcp::socket& Socket() { return *pSock_; }
	TcpSocketPtr GetSocketPtr() const { return pSock_; }
	Ip::Endpoint Address() const { return endpoint_; }
	void Start();
	void Close(); 
	template<typename T>
	void CopyRequest(T& out) const
	{
		std::copy(message_.begin(), message_.end(), out);
	}

private:
	class MessageIncomplete: public std::exception
	{
	public:
		MessageIncomplete(): std::exception() {}
	};

	void OnData(const bs::error_code& err, std::size_t);
	void OnConnected(const bs::error_code& err);
	void OnGreetingSent(const bs::error_code& err);
	void Timeout(const bs::error_code& err);
	void DetachMe();
	void Handle();

	enum {BUFFER_SIZE = 16 * 1024};

	TcpSocketPtr pSock_;
	ba::deadline_timer timer_;
	std::string message_;
	Ip::Endpoint endpoint_;
	std::vector<char> buffer_;
	bool closed_;
};

#endif //GATEKEEPER_HPP

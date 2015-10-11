/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2008-2009 Andrey Stroganov <savthe@gmail.com>

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

#include <signal.h>

#include <cstdlib>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <android/log.h>

#include "debug.hpp"
#include "system.hpp"

void OnShutdown(int n)
{
	System::Shutdown();
}

void InitSignalHandler()
{
	struct sigaction sa;

	sigset_t sigset;
	sigemptyset(&sigset);
	sigaddset(&sigset, SIGHUP);
	sigprocmask(SIG_BLOCK, &sigset, 0);
	sa.sa_handler = OnShutdown;

	sigaction(SIGINT, &sa, 0);
	sigaction(SIGTERM, &sa, 0);
}

void OnTimer(const boost::system::error_code& err, boost::asio::deadline_timer* pTimer, SystemImpl* pSys)
{
	if(!err)
	{
		pSys->OnTimer(std::time(0));
		pTimer->expires_from_now(boost::posix_time::milliseconds(100));
		pTimer->async_wait(boost::bind(OnTimer, boost::asio::placeholders::error, pTimer, pSys));
	}
}

//int main(int argc, char *argv[])
//{
//	int verbLevel = 1;
//
//	std::srand(time(0));
//	bool daemon = false;
//	for (int i = 1; i < argc; ++i)
//	{
//		if(std::strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0 )
//		{
//			std::cout << "List of options:" << std::endl;
//			std::cout << "-h, --help           This help list" << std::endl;
//			std::cout << "-d, --daemon         Daemon mode (work in background)" << std::endl;
//			std::cout << "-v n, --verbose n    Set verbose level to n" << std::endl;
//			return 0;
//
//		}
//		else if(std::strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--daemon") == 0 )
//		{
//			daemon = true;
//			verbLevel = 0;
//		}
//		if(std::strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--verbose") == 0 )
//		{
//			if( i == argc - 1 )
//			{
//				std::cout << "Use -v n, where n -- verbose level";
//				return 1;
//			}
//			verbLevel = std::atoi(argv[++i]);
//		}
//	}
//
//	if(daemon && fork() > 0) return 0;
//
//	boost::asio::io_service io;
//	SystemImpl sys(io);
//	sys.VerboseLevel(verbLevel);
//
//	boost::asio::deadline_timer timer(io, boost::posix_time::milliseconds(10));
//	timer.async_wait(boost::bind(OnTimer, boost::asio::placeholders::error, &timer, &sys));
//	InitSignalHandler();
//
//	sys.Run();
//
//	return 0;
//}

int another_main(){
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Entering \'another_main\"");
	int verbLevel = 1;

	std::srand(time(0));
	bool daemon = false;

	//if(daemon && fork() > 0) return 0;

	boost::asio::io_service io;
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Boost ASIO initialize...");
	SystemImpl sys(io);

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "set verbose");
	sys.VerboseLevel(verbLevel);

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "deadline timer");
	boost::asio::deadline_timer timer(io, boost::posix_time::milliseconds(10));
	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "async wait");
	timer.async_wait(boost::bind(OnTimer, boost::asio::placeholders::error, &timer, &sys));

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Before init signal handler.");
	InitSignalHandler();

	//__android_log_write(ANDROID_LOG_DEBUG, "sharelin", "Starting the sharelin main system.");
	sys.Run();

	return 0;
}



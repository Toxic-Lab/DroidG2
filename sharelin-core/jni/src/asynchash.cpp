/* 
    Sharelin - BSD/Linux terminal gnutella2-client
    Copyright (C) 2010  Andrey Stroganov <savthe@gmail.com>

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

#include "asynchash.hpp"
#include "hash/sha1.h"
#include "hash/ttr.h"
#include "hash/md5.h"
#include "hash/ed2khash.h"
#include "asyncfile.hpp"
#include <boost/thread.hpp>
#include <ctime>

namespace async_hash {

void Task::Run()
{
	AsyncFile f;
	try{ f.Open(path_, fio::read); }
	catch( std::exception& e )
	{
		result_.message = e.what();
		result_.status = FILE_ERROR;
		return;
	}
			
 	SHA1Context 	ctx_sha1;
 	md5_state_t 	ctx_md5;
 	TT_CONTEXT 	ctx_ttr;
 	ed2k_ctx_t	ctx_ed2k;
 
 	SHA1Reset	( &ctx_sha1 );
 	md5_init	( &ctx_md5 );
 	tt_init		( &ctx_ttr );
 	ed2k_init 	( &ctx_ed2k );

	const bool doSHA1 = mask_ & Hashes::Mask::SHA1;
	const bool doTTR = mask_ & Hashes::Mask::TTR;
	const bool doMD5 = mask_ & Hashes::Mask::MD5;
	const bool doED2K = mask_ & Hashes::Mask::ED2K;

	const file_offset_t BUF_SIZE = 1024;
    	char buf[BUF_SIZE];

	uint amount = 0;
	std::time_t now = std::time(0);
	std::time_t startTime = std::time(0);
 
	for(file_offset_t i = 0; i < range_.Length();)
	{
		if(Aborted()) return;

		const file_offset_t n = std::min(BUF_SIZE, range_.Length() - i);

		try { f.Read(buf, n, range_.Offset() + i); }
		catch (std::exception& e)
		{
			result_.message = e.what();
			result_.status = FILE_ERROR;
			return;
		}

 		if(doSHA1) SHA1Input(&ctx_sha1, (uint8_t*)buf, n);
 		if(doMD5) md5_append(&ctx_md5, (md5_byte_t*)buf, n);
 		if(doTTR) tt_update(&ctx_ttr, (byte*)buf, n);
 		if(doED2K) ed2k_update(&ctx_ed2k, (unsigned char*)buf, n);

		i += n;
		amount += n;

		if(rate_ && amount >= rate_)
		{
			while(now == std::time(0))
				boost::this_thread::sleep(boost::posix_time::milliseconds(1));
			now = std::time(0);
			amount = 0;
		}
	}

	const std::time_t duration = std::time(0) - startTime;
	result_.rate = range_.Length() / (duration + 1);

	f.Close();

	assert(Hashes::SHA1::Size() <= BUF_SIZE);
	assert(Hashes::TTR::Size() <= BUF_SIZE);
	assert(Hashes::MD5::Size() <= BUF_SIZE);
	assert(Hashes::ED2K::Size() <= BUF_SIZE);

	SHA1Result(&ctx_sha1, (uint8_t*)buf);
	if(doSHA1) result_.sha1.Assign(buf);
 
	md5_finish(&ctx_md5, (md5_byte_t*)buf);
 	if(doMD5) result_.md5.Assign(buf);
 
	tt_digest(&ctx_ttr, (byte*)buf);
	if(doTTR) result_.ttr.Assign(buf);
 
 	ed2k_finish(&ctx_ed2k, (byte*)buf);
	if(doED2K) result_.ed2k.Assign(buf);

	result_.status = GOOD;
}

} //namespace async_hash

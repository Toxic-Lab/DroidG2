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

#include "uploadmanager.hpp"
#include "system.hpp"
#include "foreach.hpp"
#include "settings.hpp"
#include "gatekeeper.hpp"
#include "uploader_impl.hpp"
#include "history.hpp"

UploadManager::UploadManager()
{
}

void UploadManager::Shutdown()
{
	foreach(Uploader::Ptr p, uploaders_)
		p->Close();
	uploaders_.clear();
}

void UploadManager::OnTimer( std::time_t now )
{
	std::vector< Uploader::Ptr > v = uploaders_;
	foreach( Uploader::Ptr p, v )
		p->OnTimer( now );
}

void UploadManager::Detach( Uploader::Ptr p )
{
	for( uint i = 0; i < uploaders_.size(); ++i )
		if( uploaders_[i] == p )
		{
			System::LogDev() << "Detaching uploader " << p->Address() << std::endl;
			History::Upload u;
			u.path = p->Path();
			u.sent = p->Sent();
			u.addr = p->Address();
			u.nick = p->Nick();
			u.client = p->ClientName();
			if(!u.path.empty() && u.sent)
				System::GetHistory()->Add(u);
			uploaders_.erase( uploaders_.begin() + i );
			break;
		}
	UpdateQueue();
}

void UploadManager::Accept(const Gatekeeper* pGatekeeper)
{
	System::LogBas() << "Got upload request from " << pGatekeeper->Address() << std::endl;

	uint count = 0;
	foreach( Uploader::Ptr p, uploaders_ )
		if( pGatekeeper->Address() == p->Address() ) ++count;

	if( count >= System::GetSettings()->net.uploadsPerHost ) 
	{
		System::LogAdv() << "Refusing uploader connection " << pGatekeeper->Address() << " (per host max)" << std::endl;
		return;
	}

	if( uploaders_.size() > System::GetSettings()->net.queueSize + System::GetSettings()->net.maxUploads)
	{
		System::LogAdv() << "Refusing uploader connection " << pGatekeeper->Address() << " (max possible uploads)" << std::endl;
		return;
	}

	Uploader::Ptr p( new UploaderImp(pGatekeeper) );
	uploaders_.push_back( p );
	UpdateQueue();
	p->Start();
}

void UploadManager::UpdateQueue()
{
	const uint max = System::GetSettings()->net.maxUploads;
	const uint queueSize = uploaders_.size() <= max ? 0 : uploaders_.size() - max;

	for(unsigned i = 0; i < uploaders_.size(); ++i)
	{
		Uploader::Ptr p = uploaders_[i];
		if(i < max) p->SetQueue(0, queueSize);
		else p->SetQueue(i + 1 - max, queueSize);	
	}

}


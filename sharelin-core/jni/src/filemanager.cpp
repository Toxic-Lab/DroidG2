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

#include "filemanager.hpp"

void FileManager::Transfer(const std::string& from, const std::string& to)
{
	Pointers::iterator i = pointers_.find(from);	
	if(i == pointers_.end()) return;
	AsyncFilePtr p = i->second;
	fio::Mode mode = p->GetMode();
	p->Close();
	try { p->Open(to, mode); }
	catch( std::exception& e )
	{
		p->Open(from, mode);
		throw;
	}
}

SharedFile FileManager::Open(const std::string& path, fio::Mode mode)
{
	AsyncFilePtr p;

	Pointers::iterator i = pointers_.find(path);
	if(i != pointers_.end())
	{
		p = i->second;
		if((mode & i->second->GetMode()) != mode)
		{
			fio::Mode oldMode = p->GetMode();
			p->Close();
			try {
				p->Open(path, mode);
			}
			catch (AsyncFile::Error&)
			{
				p->Open(path, oldMode);
				throw;
			}
		}
	}
	else
	{
		p = AsyncFilePtr(new AsyncFile);
		p->Open(path, mode);
		pointers_[path] = p;
	}

	return SharedFile(this, p);
}

void FileManager::Close(AsyncFilePtr& pFile)
{
	if(!pFile) return;

	Pointers::iterator i = pointers_.find(pFile->PathName());
	pFile.reset();
	if(i != pointers_.end() && i->second.unique())
		pointers_.erase(i);
}


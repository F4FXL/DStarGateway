/*
 *   Copyright (C) 2010-2013,2015,2018 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
 
#include <stdio.h>
#include <cstdio>
#include <curl/curl.h>

#include "XLXHostsFileDownloader.h"
#include "Log.h"
 
size_t CXLXHostsFileDownloader::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

/* wxHTTP randomly crashes when called on a worker thread, this must be called from main thread ! */
std::string CXLXHostsFileDownloader::download(const std::string & xlxHostsFileURL)
{
    CURL *curl;
    FILE *fp;
    bool ok = false;
	std::string outFileName;
	char outFileNameBuf[] = "/tmp/XLXHostFile_XXXXXX";

	CLog::logInfo("Downloading XLX host file from %s", xlxHostsFileURL.c_str());

    curl = curl_easy_init();
    if (curl) {
        int filedes = mkstemp(outFileNameBuf);
		if(filedes > 0 && (fp = fdopen(filedes,"wb")) != NULL) {
			curl_easy_setopt(curl, CURLOPT_URL, xlxHostsFileURL.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
			CURLcode res = curl_easy_perform(curl);
			ok = res == 0;
			/* always cleanup */
			curl_easy_cleanup(curl);
			fclose(fp);
		}
	}
    

	if(ok) {
		outFileName = std::string(outFileNameBuf);
	} else {
		CLog::logError("Failed to download XLx Host file from %s", xlxHostsFileURL.c_str());
	}

	return outFileName;
}

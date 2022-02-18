/*
 *   Copyright (c) 2022 by Geoffrey Merck F4FXL / KC3FRA
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

/*
 * Losely based on https://github.com/jirihnidek/daemon/blob/master/src/daemon.c
 */

#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/file.h>

#include "Daemon.h"
#include "Log.h"

int CDaemon::m_pid_fd = -1;
std::string CDaemon::m_pidFileName("");

DAEMONIZE_RESULT CDaemon::daemonize(const std::string& pidFile, const std::string& userName)
{
	// get user
	struct passwd* user = nullptr;
	if(!userName.empty() && getuid() == 0) {
		user = getpwnam(userName.c_str());
		if(user == nullptr) {
			CLog::logFatal("Failed to get %s user", userName.c_str());
			return DR_FAILURE;
		}
	}

	// Create PID file if needed
	if (!pidFile.empty()) {	
		auto tempFd = tryGetLock(pidFile);
		if (tempFd < 0) {
			CLog::logFatal("Failed to acquire lock on pidfile %s : %s", pidFile.c_str(), strerror(errno));
			return DR_PIDFILE_FAILED;
		}
		releaseLock(tempFd, "");

		if(user != nullptr) {
			int res = chown(pidFile.c_str(), user->pw_uid, user->pw_gid);
			if(res != 0) {
				CLog::logFatal("Failed to set ownership of pidfile to user %s : %s", userName.c_str(), strerror(errno));
				return DR_FAILURE;
			}
		}
	}

	// change process ownership
	if(user != nullptr) {
		if(setgid(user->pw_gid) != 0) {
			CLog::logFatal("Failed to set %s GID : %s", userName.c_str(), strerror(errno));
			return DR_FAILURE;
		}

		if(setuid(user->pw_uid) != 0) {
			CLog::logFatal("Failed to set %s UID : %s", userName.c_str(), strerror(errno));
			return DR_FAILURE;
		}

		// Double check it worked (AKA Paranoia)
		if (setuid(0) != -1){
			CLog::logFatal("It's possible to regain root - something is wrong!, exiting");
			return DR_FAILURE;
		}
	}

	pid_t pid = 0;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		CLog::logFatal("Forking failed, exiting");
		return DR_FAILURE;
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		return DR_PARENT;
	}

	// On success: The child process becomes session leader
	if (setsid() < 0) {
		CLog::logFatal("Failed to set session id, exiting");
        return DR_FAILURE;
	}

	/* Ignore signal sent from child to parent process */
	signal(SIGCHLD, SIG_IGN);

#ifdef DOUBLE_FORK
	// Fork off for the second time. Some litterature says it is best to fork 2 times so that the process never can open a terminal.
	// However it messes up systemd, event when unit is set as forking
	pid = fork();

	// An error occurred
	if (pid < 0) {
		CLog::logFatal("Second forking failed, exiting");
		return DR_FAILURE;
	}

	// Success: Let the parent terminate
	if (pid > 0) {
		return DR_PARENT;
	}
#endif

	// Set new file permissions
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	if(chdir("/") != 0) {
		CLog::logFatal("Faild to cd, exiting");
		return DR_FAILURE;
	}

#ifdef CLOSE_FILE_DESC
	// Close all open file descriptors
	for (int fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}

	// Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2)
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");
#endif

	// Try to write PID of daemon to lockfile
	if (!pidFile.empty())
	{
		m_pid_fd = tryGetLock(pidFile);
		if (m_pid_fd < 0) {
			/* Can't open lockfile */
			return DR_PIDFILE_FAILED;
		}
		if (lockf(m_pid_fd, F_TLOCK, 0) < 0) {
			/* Can't lock file */
			return DR_PIDFILE_FAILED;
		}

        m_pidFileName.assign(pidFile);

		/* Get current PID */
        char str[256];
		sprintf(str, "%d\n", getpid());
		/* Write PID to lockfile */
		write(m_pid_fd, str, strlen(str));
	}

    return DR_CHILD;
}

void CDaemon::finalise()
{
    releaseLock(m_pid_fd, m_pidFileName);

    if(!m_pidFileName.empty()) {
        unlink(m_pidFileName.c_str());
    }
}

int CDaemon::tryGetLock( const std::string& file )
{
    mode_t m = umask( 0 );
    int fd = open( file.c_str(), O_RDWR|O_CREAT, 0640 );
    umask( m );
    if( fd >= 0 && flock( fd, LOCK_EX | LOCK_NB ) < 0 ) {
        close( fd );
        fd = -1;
    }
    return fd;
}


void CDaemon::releaseLock(int fd, const std::string& file)
{
    if( fd < 0 )
        return;
    remove(file.c_str());
    close(fd);
}
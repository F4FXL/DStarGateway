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
#include <sys/stat.h>
#include <fcntl.h>

#include "Daemon.h"
#include "Log.h"

int CDaemon::m_pid_fd = -1;
std::string CDaemon::m_pidFileName("");

DAEMONIZE_RESULT CDaemon::daemonize(const std::string& pidFile)
{
	pid_t pid = 0;
	int fd;

	/* Fork off the parent process */
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		return DR_FAILURE;
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		return DR_PARENT;
	}

	/* On success: The child process becomes session leader */
	if (setsid() < 0) {
        return DR_FAILURE;
	}

	/* Ignore signal sent from child to parent process */
	signal(SIGCHLD, SIG_IGN);

	/* Fork off for the second time*/
	pid = fork();

	/* An error occurred */
	if (pid < 0) {
		return DR_FAILURE;
	}

	/* Success: Let the parent terminate */
	if (pid > 0) {
		return DR_PARENT;
	}

	if (setsid() < 0) {
        return DR_FAILURE;
	}

	/* Set new file permissions */
	umask(0);

	/* Change the working directory to the root directory */
	/* or another appropriated directory */
	chdir("/");

	/* Close all open file descriptors */
	for (fd = sysconf(_SC_OPEN_MAX); fd > 0; fd--) {
		close(fd);
	}

	/* Reopen stdin (fd = 0), stdout (fd = 1), stderr (fd = 2) */
	stdin = fopen("/dev/null", "r");
	stdout = fopen("/dev/null", "w+");
	stderr = fopen("/dev/null", "w+");

	/* Try to write PID of daemon to lockfile */
	if (!pidFile.empty())
	{
        CLog::logInfo("pidfile");
		
		m_pid_fd = open(pidFile.c_str(), O_RDWR|O_CREAT, 0640);
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

void CDaemon::finalize()
{
    if(m_pid_fd != -1) {
        lockf(m_pid_fd, F_ULOCK, 0);
		close(m_pid_fd);
    }

    if(!m_pidFileName.empty()) {
        unlink(m_pidFileName.c_str());
    }
}
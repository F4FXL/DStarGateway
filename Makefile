#
#   Copyright (C) 2021 by Geoffrey Merck F4FXL / KC3FRA
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

export BIN_DIR=/usr/local/bin
export CFG_DIR=/usr/local/etc/
export DATA_DIR=/usr/local/share/dstargateway.d/
export LOG_DIR=/var/log/dstargateway/

# choose this if you want debugging help
CPPFLAGS=-g -ggdb -W -Wall -Werror -std=c++17
# or, you can choose this for a much smaller executable without debugging help
#CPPFLAGS=-W -Wall -Werror -std=c++17

LDFLAGS:=-lcurl -pthread

ifeq ($(USE_GPSD), 1)
CPPFLAGS+= -DUSE_GPSD
LDFLAGS+= -lgps
endif

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

.PHONY: all
all: dstargateway

dstargateway : GitVersion.h $(OBJS) 
	g++ $(CPPFLAGS) -o dstargateway $(OBJS) $(LDFLAGS)

%.o : %.cpp
	g++ $(CPPFLAGS) -MMD -MD -c $< -o $@

GitVersion.h : FORCE 
ifneq ("$(wildcard .git/index)","")
	@echo "#pragma once" > /tmp/$@
	@echo "#include <string>" >> /tmp/$@
	@echo "const std::string gitversion(\"$(shell git rev-parse --short HEAD)\");" >> /tmp/$@
else
	@echo "#pragma once" > /tmp/$@
	@echo "#include <string>" >> /tmp/$@
	@echo "const std::string gitversion(\"0000000\");" >> /tmp/$@
endif
	@cmp -s /tmp/$@ $@; \
	RETVAL=$$?; \
	if [ $$RETVAL -ne 0 ]; then \
		echo "Git version has changed"; \
		cp -f /tmp/$@ $@; \
	fi; \
	rm /tmp/$@;

.PHONY: clean
clean:
	$(RM) GitVersion.h $(OBJS) $(DEPS) dstargateway

-include $(DEPS)

# install, uninstall and removehostfiles need root priviledges
.PHONY: newhostfiles
newhostfiles :
	@echo "Fetching new host files"
	@mkdir -p $(DATA_DIR)
	@wget http://www.pistar.uk/downloads/DExtra_Hosts.txt -nv -O $(DATA_DIR)/DExtra_Hosts.txt
	@wget http://www.pistar.uk/downloads/DCS_Hosts.txt -nv -O $(DATA_DIR)/DCS_Hosts.txt
	@wget http://www.pistar.uk/downloads/DPlus_Hosts.txt -nv -O $(DATA_DIR)/DPlus_Hosts.txt

.PHONY: install
install : dstargateway
# create user for daemon
	@useradd --user-group -M --system dstar --shell /bin/false || true

# Create directories
	@mkdir -p $(CFG_DIR)
	@mkdir -p $(DATA_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(LOG_DIR)
	@chown dstar:dstar $(LOG_DIR)

# Install data
	$(MAKE) -C Data install
	@chown -R dstar:dstar $(DATA_DIR)

# copy and adjust config
	@cp -fn example.cfg $(CFG_DIR)/dstargateway.cfg
	@sed -i "s|path=/var/log/dstargateway/|path=$(LOG_DIR)|g" $(CFG_DIR)/dstargateway.cfg
	@sed -i "s|data=/usr/local/share/dstargateway.d/|data=$(DATA_DIR)|g" $(CFG_DIR)/dstargateway.cfg

# copy binary
	@cp -f dstargateway $(BIN_DIR)

# SystemD service install
	@cp -f debian/dstargateway.service /lib/systemd/system/
	@sed -i "s|%CFG_DIR%|$(CFG_DIR)|g" /lib/systemd/system/dstargateway.service
	systemctl enable dstargateway.service
	@systemctl daemon-reload
	@echo "\n\n"
	@echo "Install complete, edit $(CFG_DIR)dstargateway.cfg and start the daemon with 'systemctl start dstargateway.service'"
	@echo "\n\n"

.PHONY: uninstall
uninstall :
	systemctl stop dstargateway.service || true
	systemctl disable dstargateway.service || true
	@rm -f /lib/systemd/system/dstargateway.service
	@systemctl daemon-reload
	@rm -f $(BIN_DIR)/dstargateway
#	@rm -f $(CFG_DIR)/dstargateway.cfg
# Only remove folder content
	@rm -f $(DATA_DIR)/* 

.PHONY: removehostfiles
removehostfiles :
	@rm -f $(DATA_DIR)/DExtra_Hosts.txt
	@rm -f $(DATA_DIR)/DCS_Hosts.txt
	@rm -f $(DATA_DIR)/DPlus_Hosts.txt



FORCE:
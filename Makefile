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

ifeq ($(ENABLE_DEBUG), 1)
# choose this if you want debugging help
export CPPFLAGS=-g -rdynamic -DBOOST_STACKTRACE_USE_ADDR2LINE -DDEBUG_DSTARGW -no-pie -fno-pie -ggdb -W -Wall -Werror -std=c++17
export LDFLAGS=-ldl -no-pie -fno-pie
else
# or, you can choose this for a much smaller executable without debugging help
export CPPFLAGS=-W -O3 -Wall -Werror -std=c++17
endif

export CC=g++
export LDFLAGS+= -lcurl -pthread

ifeq ($(USE_GPSD), 1)
export CPPFLAGS+= -DUSE_GPSD
export LDFLAGS+= -lgps
endif


.PHONY: all
all: DStarGateway/dstargateway  DGWRemoteControl/dgwremotecontrol #tests

APRS/APRS.a: BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C APRS

Common/Common.a: VersionInfo/GitVersion.h APRS/APRS.a BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C Common

BaseCommon/BaseCommon.a: FORCE
	$(MAKE) -C BaseCommon

DStarBase/DStarBase.a: BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C DStarBase

DStarGateway/dstargateway :  VersionInfo/GitVersion.h $(OBJS) APRS/APRS.a Common/Common.a DStarBase/DStarBase.a IRCDDB/IRCDDB.a BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C DStarGateway

DGWRemoteControl/dgwremotecontrol: VersionInfo/GitVersion.h $(OBJS) DStarBase/DStarBase.a BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C DGWRemoteControl

IRCDDB/IRCDDB.a: VersionInfo/GitVersion.h BaseCommon/BaseCommon.a FORCE
	$(MAKE) -C IRCDDB

VersionInfo/GitVersion.h: FORCE
	$(MAKE) -C VersionInfo

.PHONY: clean
clean:
	$(MAKE) -C Tests clean
	$(MAKE) -C APRS clean
	$(MAKE) -C Common clean
	$(MAKE) -C BaseCommon clean
	$(MAKE) -C DStarBase clean
	$(MAKE) -C DStarGateway clean
	$(MAKE) -C IRCDDB clean
	$(MAKE) -C VersionInfo

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
install : DStarGateway/dstargateway DGWRemoteControl/dgwremotecontrol
# install remote control
	$(MAKE) -C DGWRemoteControl install
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

#install executables
	$(MAKE) -C DStarGateway install

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

.PHONY tests:
tests : VersionInfo/GitVersion.h $(OBJS) APRS/APRS.a Common/Common.a DStarBase/DStarBase.a IRCDDB/IRCDDB.a BaseCommon/BaseCommon.a FORCE
	@$(MAKE) -C Tests dstargateway_tests

.PHONY run-tests:
run-tests: tests
	@$(MAKE) -C Tests run-tests

FORCE:
	@true

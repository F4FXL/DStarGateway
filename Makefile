# Copyright (c) 2021 by Geoffrey Merck F4FXL / KC3FRA

# if you change these locations, make sure the dstargateway.service file is updated!
BINDIR=/usr/local/bin
CFGDIR=/usr/local/etc/dstargateway.d/

# choose this if you want debugging help
CPPFLAGS=-g -ggdb -W -Wall -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"
# or, you can choose this for a much smaller executable without debugging help
#CPPFLAGS=-W -Wall -std=c++11 -DCFG_DIR=\"$(CFGDIR)\"

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dstargateway :  GitVersion.h $(OBJS)
	g++ $(CPPFLAGS) -o dstargateway $(OBJS) -lconfig++ -pthread

%.o : %.cpp
	g++ $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY: clean

clean:
	$(RM) GitVersion.h $(OBJS) $(DEPS) dstargateway

-include $(DEPS)

# install, uninstall and removehostfiles need root priviledges
newhostfiles :
	/usr/bin/wget http://www.pistar.uk/downloads/DExtra_Hosts.txt && sudo /bin/mv -f DExtra_Hosts.txt $(CFGDIR)
	/usr/bin/wget http://www.pistar.uk/downloads/DCS_Hosts.txt && sudo /bin/mv -f DCS_Hosts.txt $(CFGDIR)

install : dstargateway
	/bin/cp -f dstargateway.cfg $(CFGDIR)
	/bin/cp -f dstargateway $(BINDIR)
	/bin/cp -f dstargateway.service /lib/systemd/system
	systemctl enable dstargateway.service
	systemctl daemon-reload
	systemctl start dstargateway.service

uninstall :
	systemctl stop dstargateway.service
	systemctl disable dstargateway.service
	/bin/rm -f /lib/systemd/system/dstargateway.service
	systemctl daemon-reload
	/bin/rm -f $(BINDIR)/dstargateway
	/bin/rm -f $(CFGDIR)/dstargateway.cfg

removehostfiles :
	/bin/rm -f $(CFGDIR)/DExtra_Hosts.txt
	/bin/rm -f $(CFGDIR)/DCS_Hosts.txt

GitVersion.h : force
ifneq ("$(wildcard .git/index)","")
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@
else
	echo "const char *gitversion = \"0000000000000000000000000000000000000000\";" > $@
endif

force:
	@true
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwtimeserver: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwtimeserver $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../DStarBase -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwtimeserver

.PHONY install:
install: dgwtimeserver
# copy executable
	@cp -f dgwtimeserver $(BIN_DIR)

# copy and adjust config
	@cp -fn example.cfg $(CFG_DIR)/dgwtimeserver.cfg
	@sed -i "s|path=/var/log/dstargateway/|path=$(LOG_DIR)|g" $(CFG_DIR)/dgwtimeserver.cfg
	@sed -i "s|data=/usr/local/share/dstargateway.d/|data=$(DATA_DIR)|g" $(CFG_DIR)/dgwtimeserver.cfg

# SystemD service install
	@cp -f ../debian/dgwtimeserver.service /lib/systemd/system/
	@sed -i "s|%CFG_DIR%|$(CFG_DIR)|g" /lib/systemd/system/dgwtimeserver.service
	systemctl enable dgwtimeserver.service
	@systemctl daemon-reload
	@echo "\n"
	@echo "DGWTimeserver Install complete, edit $(CFG_DIR)dstargateway.cfg and start the daemon with 'systemctl start dgwtimeserver.service'"
	@echo "\n"

../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

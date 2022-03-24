SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dstargateway: ../VersionInfo/GitVersion.h $(OBJS) ../APRS/APRS.a ../IRCDDB/IRCDDB.a ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a  ../Common/Common.a
	$(CC) $(CPPFLAGS) -o dstargateway $(OBJS) ../Common/Common.a ../APRS/APRS.a ../DStarBase/DStarBase.a ../IRCDDB/IRCDDB.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../APRS -I../Common -I../BaseCommon -I../DStarBase -I../IRCDDB -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dstargateway

.PHONY install:
install: dstargateway
# copy executable
	@cp -f dstargateway $(BIN_DIR)

# copy and adjust config
	@cp -fn example.cfg $(CFG_DIR)/dstargateway.cfg
	@sed -i "s|path=/var/log/dstargateway/|path=$(LOG_DIR)|g" $(CFG_DIR)/dstargateway.cfg
	@sed -i "s|data=/usr/local/share/dstargateway.d/|data=$(DATA_DIR)|g" $(CFG_DIR)/dstargateway.cfg

# SystemD service install
	@cp -f ../debian/dstargateway.service /lib/systemd/system/
	@sed -i "s|%CFG_DIR%|$(CFG_DIR)|g" /lib/systemd/system/dstargateway.service
	systemctl enable dstargateway.service
	@systemctl daemon-reload
	@echo "\n"
	@echo "DStarGateway Install complete, edit $(CFG_DIR)dstargateway.cfg and start the daemon with 'systemctl start dstargateway.service'"
	@echo "\n"

../APRS/APRS.a:
../Common/Common.a:
../DStarBase/DStarBase.a:
../BaseCommon/BaseCommon.a:
../IRCDDB/IRCDDB.a:
../VersionInfo/GitVersion.h:

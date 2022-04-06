SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwvoicetransmit: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../APRS/APRS.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwvoicetransmit $(OBJS) ../DStarBase/DStarBase.a ../APRS/APRS.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../APRS -I../DStarBase -I../VersionInfo -DCFG_DIR='"$(CFG_DIR)"' $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwvoicetransmit

.PHONY install:
install: dgwvoicetransmit
# copy executable
	@cp -f dgwvoicetransmit $(BIN_DIR)

../APRS/APRS.a:
../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

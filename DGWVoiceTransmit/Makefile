SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwvoicetransmit: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwvoicetransmit $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../DStarBase -I../VersionInfo -DCFG_DIR='"$(CFG_DIR)"' $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwvoicetransmit

.PHONY install:
install: dgwvoicetransmit
# copy executable
	@cp -f dgwvoicetransmit $(BIN_DIR)

../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwremotecontrol: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwremotecontrol $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../DStarBase -I../VersionInfo -DCFG_DIR='"$(CFG_DIR)"' $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwremotecontrol

.PHONY install:
install: dgwremotecontrol
# copy executable
	@cp -f dgwremotecontrol $(BIN_DIR)

# copy and adjust config
	@cp -fn example.cfg $(CFG_DIR)/dgwremotecontrol.cfg

../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

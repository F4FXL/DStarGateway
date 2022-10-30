SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwtexttransmit: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwtexttransmit $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a $(LDFLAGS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../DStarBase -I../VersionInfo -DCFG_DIR='"$(CFG_DIR)"' $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwtexttransmit

.PHONY install:
install: dgwtexttransmit
# copy executable
	@cp -f dgwtexttransmit $(BIN_DIR)

../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

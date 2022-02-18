SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dgwtimeserver: ../VersionInfo/GitVersion.h $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a
	$(CC) $(CPPFLAGS) -o dgwtimeserver $(OBJS) ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a $(LDFLAGS)
	echo bla

%.o : %.cpp
	$(CC) -I../BaseCommon -I../DStarBase -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@

.PHONY clean:
clean:
	$(RM) *.o *.d dgwtimeserver

.PHONY install:
install: dgwtimeserver
# copy executable
	@cp -f dgwtimeserver $(BIN_DIR)

../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../VersionInfo/GitVersion.h:

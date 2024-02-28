SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

Common.a: $(OBJS) ../APRS/APRS.a ../BaseCommon/BaseCommon.a ../DStarBase/DStarBase.a ../IRCDDB/IRCDDB.a ../VersionInfo/GitVersion.h
	$(AR) rcs Common.a $(OBJS)

%.o : %.cpp
	$(CC) -I../APRS -I../BaseCommon -I../DStarBase -I../IRCDDB -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

.PHONY clean:
clean:
	$(RM) *.o *.d Common.a

../APRS/APRS.a:
../BaseCommon/BaseCommon.a:
../DStarBase/DStarBase.a:
../IRCDDB/IRCDDB.a:
../VersionInfo/GitVersion.h:

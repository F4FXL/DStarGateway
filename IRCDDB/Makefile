SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

IRCDDB.a: $(OBJS) ../VersionInfo/GitVersion.h ../BaseCommon/BaseCommon.a
	$(AR) rcs IRCDDB.a $(OBJS)

%.o : %.cpp
	$(CC) -I../BaseCommon -I../VersionInfo $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

.PHONY clean:
clean:
	$(RM) *.o *.d IRCDDB.a

../BaseCommon/BaseCommon.a:
../VersionInfo/GitVersion.h:

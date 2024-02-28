SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

DStarBase.a: $(OBJS)
	$(AR) rcs DStarBase.a $(OBJS)

%.o : %.cpp
	$(CC) -I../BaseCommon $(CPPFLAGS) -MMD -MD -c $< -o $@
-include $(DEPS)

clean:
	$(RM) *.o *.d DStarBase.a

../BaseCommon/BaseCommon.a:

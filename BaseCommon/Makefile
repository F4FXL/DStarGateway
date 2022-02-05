SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

BaseCommon.a: $(OBJS)
	$(AR) rcs BaseCommon.a $(OBJS)

%.o : %.cpp
	$(CC) $(CPPFLAGS) -MMD -MD -c $< -o $@

clean:
	$(RM) *.o *.d BaseCommon.a
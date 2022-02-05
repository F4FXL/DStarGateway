SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

APRS.a: $(OBJS)
	$(AR) rcs APRS.a $(OBJS)

%.o : %.cpp
	$(CC) -I../BaseCommon $(CPPFLAGS) -MMD -MD -c $< -o $@

clean:
	$(RM) *.o *.d APRS.a
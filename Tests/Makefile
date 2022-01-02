
SRCS = $(wildcard *.cpp) $(wildcard ../*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

.PHONY tests:
tests: $(OBJS)
	$(CC) $(CPPFLAGS) -o dstargateway_tests $(OBJS) $(LDFLAGS) -lgtest -lgtest_main

%.o : %.cpp
	$(CC) $(CPPFLAGS) -DUNIT_TESTS  -I../ -MMD -MD -c $< -o $@

-include $(DEPS)

.PHONY clean :
clean :
	@$(RM) $(OBJS) $(DEPS) dstargateway_tests


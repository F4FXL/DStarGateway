SRCS = $(wildcard *.cpp) $(wildcard */*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

# GoogleTest is normally taken from the system (libgtest-dev). Set GTEST_DIR to a
# prefix holding include/ and lib/ if it is installed somewhere else, e.g.
#   make GTEST_DIR=/path/to/gtest
ifdef GTEST_DIR
# -isystem keeps GoogleTest's own headers warning free, just like an install into /usr/include
GTEST_CPPFLAGS = -isystem $(GTEST_DIR)/include
GTEST_LDFLAGS  = -L$(GTEST_DIR)/lib
endif
GTEST_LIBS = -lgtest -lgtest_main

dstargateway_tests: ../VersionInfo/GitVersion.h $(OBJS) ../APRS/APRS.a ../IRCDDB/IRCDDB.a ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a  ../Common/Common.a
	$(CC) $(CPPFLAGS) -o dstargateway_tests $(OBJS) ../Common/Common.a ../APRS/APRS.a ../DStarBase/DStarBase.a ../IRCDDB/IRCDDB.a ../BaseCommon/BaseCommon.a $(LDFLAGS) $(GTEST_LDFLAGS) $(GTEST_LIBS)

%.o : %.cpp
	$(CC) $(CPPFLAGS) $(GTEST_CPPFLAGS) -DUNIT_TESTS -I../APRS -I../Common -I../BaseCommon -I../DStarBase -I../IRCDDB -I../VersionInfo -MMD -MD -c $< -o $@

-include $(DEPS)

.PHONY run-tests: dstargateway_tests
	./dstargateway_tests

.PHONY clean :
clean :
	find . -name "*.o" -type f -delete
	find . -name "*.d" -type f -delete
	$(RM) *.o *.d dstargateway_tests

../APRS/APRS.a:
../Common/Common.a:
../DStarBase/DStarBase.a:
../BaseCommon/BaseCommon.a:
../IRCDDB/IRCDDB.a:
../VersionInfo/GitVersion.h:

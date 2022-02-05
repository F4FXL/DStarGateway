SRCS = $(wildcard *.cpp) $(wildcard */*.cpp)
OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

dstargateway_tests: ../VersionInfo/GitVersion.h $(OBJS) ../APRS/APRS.a ../IRCDDB/IRCDDB.a ../DStarBase/DStarBase.a ../BaseCommon/BaseCommon.a  ../Common/Common.a
	$(CC) $(CPPFLAGS) -o dstargateway_tests $(OBJS) ../Common/Common.a ../APRS/APRS.a ../DStarBase/DStarBase.a ../IRCDDB/IRCDDB.a ../BaseCommon/BaseCommon.a $(LDFLAGS) -lgtest -lgtest_main 

%.o : %.cpp
	$(CC) $(CPPFLAGS) -DUNIT_TESTS -I../APRS -I../Common -I../BaseCommon -I../DStarBase -I../IRCDDB -I../VersionInfo -MMD -MD -c $< -o $@

-include $(DEPS)

.PHONY run-tests: dstargateway_tests 
	./dstargateway_tests

.PHONY clean :
clean :
	@$(RM) *.o *.d dstargateway_tests

../APRS/APRS.a:
../Common/Common.a:
../DStarBase/DStarBase.a:
../BaseCommon/BaseCommon.a:
../IRCDDB/IRCDDB.a:
../VersionInfo/GitVersion.h:

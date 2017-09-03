CXX = g++
#CXXFLAGS = -g -L/opt/gtest/lib -lgtest -lgtest_main -lpthread
#INCS = -I./ -I../../include -I/opt/gtest/include
CXXFLAGS = -g -L/usr/lib -lgtest -lgtest_main -lpthread
INCS = -I./ -I../../include -I/usr/include/
OBJS = func.o funcTest.o

testAll: $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) -o testAll  main.cpp $(OBJS) $(INCS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCS)

clean:
	rm testAll *.o 
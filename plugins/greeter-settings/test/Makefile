CXX=g++
CXXFLAGS=`pkg-config --cflags glibmm-2.4` -g -I../
LDFLAGS=`pkg-config --libs glibmm-2.4`

SOURCES=$(wildcard *.cpp)
SOURCES+=../greeter-settings-manager.cpp
OBJS=$(subst .cpp,.o,$(SOURCES))

TARGET=test

$(TARGET): $(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LDFLAGS)

$(OBJS):%.o:%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)

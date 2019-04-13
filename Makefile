
CXX      = clang++
CXXFLAGS = -Wall -Wextra -std=c++11
LDFLAGS  = -g3

# the build target executable:
TARGET = proxy
DEPS = cache.cpp

all: $(TARGET)

$(TARGET): $(TARGET).o $(DEPS)
	$(CXX) $(LDFLAGS) $(DEPS) -o $(TARGET) $(TARGET).cpp

clean: 
	#rm $(TARGET)
	rm *.o
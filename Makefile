CXX = clang++
CXXFLAGS = -g3 -Wall -Wextra -std=c++11
LDFLAGS  = -g3

INCLUDES = $(shell echo *.h)

all: proxy testcache

%.o: %.c $(INCLUDES)
	$(CXX) $(CXXFLAGS) -c $< -o $@

testcache:  testcache.o cache.o
	    $(CXX) $(LDFLAGS) $^ -o $@

proxy:  proxy.o cache.o sockets.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f testcache proxy *.o

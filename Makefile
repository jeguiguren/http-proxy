CXX = clang++
CXXFLAGS = -g3 -Wall -Wextra -std=c++11
LDFLAGS  = -g3


all: proxy testcache

testcache:  testcache.o cache.o
	    $(CXX) $(LDFLAGS) $^ -o $@

proxy:  proxy.o cache.o sockets.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f testcache proxy *.o

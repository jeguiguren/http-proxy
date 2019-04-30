CXX = clang++
CXXFLAGS = -g3 -Wall -Wextra -std=c++11
LDFLAGS  = -g3


all: proxy
	
proxy:  proxy.o cache.o sockets.o util.o
	$(CXX) $(LDFLAGS) $^ -o $@

clean:
	rm -f testcache proxy *.o

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O3

TARGET = tun
SRCS = src/tun.cpp \
       src/upf.cpp \
       src/event_loop.cpp \
       src/sockets.cpp \
       src/arp.cpp \
       src/downlink.cpp \
       src/uplink.cpp \
       src/ue_storage.cpp \
       src/net_utils.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

-include $(DEPS)

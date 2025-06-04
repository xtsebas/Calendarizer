CXX       := g++
CXXFLAGS  := -g -std=c++17 -I.

SRCS := \
  $(wildcard process/*.cpp) \
  $(wildcard utils/*.cpp) \
  scheduler/scheduler.cpp \
  $(wildcard scheduler/SJF/*.cpp) \
  $(wildcard scheduler/RoundRobin/*.cpp) \
  $(wildcard scheduler/FIFO/*.cpp) \
  $(wildcard scheduler/SRTF/*.cpp) \
  $(wildcard scheduler/Priority/*.cpp) \
  main.cpp
OBJS      := $(SRCS:.cpp=.o)
TARGET    := calendarizer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

CXX       := g++
CXXFLAGS  := -g -std=c++17 -I.

SRCS      := $(wildcard process/*.cpp utils/*.cpp scheduler/scheduler.cpp scheduler/SJF/*.cpp scheduler/RoundRobin/*.cpp main.cpp)
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

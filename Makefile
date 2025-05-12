# Makefile para Calendarizer

# 1) Variables
CXX       := g++
CXXFLAGS  := -g -std=c++17 -I.
SRCS      := $(wildcard process/*.cpp utils/*.cpp) main.cpp
OBJS      := $(SRCS:.cpp=.o)
TARGET    := calendarizer

# 2) Regla por defecto
all: $(TARGET)

# 3) Cómo enlazar el ejecutable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 4) Cómo compilar cada .cpp a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 5) Limpieza de artefactos
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

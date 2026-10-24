CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -Wextra
TARGET    := myshell
SOURCES   := command.cc tokenizer.cc
OBJECTS   := $(SOURCES:.cc=.o)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS)

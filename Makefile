CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
TARGET = 	
SOURCE = cubing_tools.cpp

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: clean
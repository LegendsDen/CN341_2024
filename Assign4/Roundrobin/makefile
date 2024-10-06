# Makefile for compiling islip program

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -O2

# Source file
SRC = RR.cpp

# Output executable
OUT = RR.exe

# Target to build the executable
all: $(OUT)

# Rule to build the executable
$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

# Clean up generated files
clean:
	del /f $(OUT)

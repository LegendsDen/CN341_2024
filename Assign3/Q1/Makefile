# Makefile for compiling server and client programs

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++11 -Wall -O2

# Linker flags (for linking required libraries)
LDFLAGS = -lws2_32 

# Source files
SRV_SRC = Q1_server.cpp
CLI_SRC = Q1_client.cpp

# Output executables
SRV_OUT = Q1_server.exe
CLI_OUT = Q1_client.exe

# Target to build both server and client
all: $(SRV_OUT) $(CLI_OUT)

# Rule to build server
$(SRV_OUT): $(SRV_SRC)
	$(CXX) $(CXXFLAGS) $(SRV_SRC) -o $(SRV_OUT) $(LDFLAGS)

# Rule to build client
$(CLI_OUT): $(CLI_SRC)
	$(CXX) $(CXXFLAGS) $(CLI_SRC) -o $(CLI_OUT) $(LDFLAGS)

# Clean up generated files
clean:
	del /f $(SRV_OUT) $(CLI_OUT)

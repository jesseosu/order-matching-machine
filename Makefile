# Makefile for Order Matching Engine

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -pthread
TARGET = order_matching_engine
SRC = order_matching_engine.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

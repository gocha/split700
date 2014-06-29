#!/bin/make
CC = gcc
CFLAGS = -O2 -Wall -DGSFOPT
CXX = g++
CXXFLAGS = -O2 -Wall -DGSFOPT
LDFLAGS = -lm
TARGET = split700
SRCS = $(TARGET).cpp
OBJS := $(SRCS:.cpp=.o)

.PHONY: all clean
.SUFFIXES: .c .cpp .o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $^
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $^

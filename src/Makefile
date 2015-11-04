#!/bin/make
CC = gcc
CFLAGS = -O2 -Wall -std=c99
CXX = g++
CXXFLAGS = -O2 -Wall -std=c++11
LDFLAGS = -lm
TARGET = split700
SRCS = split700.cpp SPCFile.cpp SPCSampDir.cpp WavWriter.cpp
OBJS := $(SRCS:.cpp=.o)
SRCS_BRR2WAV = brr2wav.cpp SPCFile.cpp SPCSampDir.cpp WavWriter.cpp
OBJS_BRR2WAV := $(SRCS_BRR2WAV:.cpp=.o)

.PHONY: all clean brr2wav
.SUFFIXES: .c .cpp .o

all: $(TARGET) brr2wav

$(TARGET): $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

brr2wav: $(OBJS_BRR2WAV)
	$(CXX) -o $@ $(OBJS_BRR2WAV) $(LDFLAGS)

clean:
	$(RM) $(TARGET)
	$(RM) $(OBJS)
	$(RM) $(OBJS_BRR2WAV)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $^
.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $^

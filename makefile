#make a makefile for the program


#variables

CC = gcc
CFLAGS = -Wall -g

ORCHESTRATOR_DIR = ./src/orchestrator
BUILD_DIR = ./build
OBJR_DIR = objects

all: main

main: $(BUILD_DIR)/$(OBJR_DIR)/main.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/main $(BUILD_DIR)/$(OBJR_DIR)/main.o

$(BUILD_DIR)/$(OBJR_DIR)/main.o: $(ORCHESTRATOR_DIR)/main.c
	$(CC) $(CFLAGS) -c $(ORCHESTRATOR_DIR)/main.c -o $(BUILD_DIR)/$(OBJR_DIR)/main.o
	
clean:
	rm -f $(BUILD_DIR)/$(OBJR_DIR)/*.o main
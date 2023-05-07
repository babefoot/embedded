#make a makefile for the program


#variables

CC = gcc
CFLAGS = -Wall -g

ORCHESTRATOR_DIR = ./src/orchestrator
HARDWARE_MANAGER_DIR = ./src/hardwareManager
BUILD_DIR = ./build
OBJR_DIR = objects
LIB_DIR = ./lib

all: main

main: $(BUILD_DIR)/$(OBJR_DIR)/main.o $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/main $(BUILD_DIR)/$(OBJR_DIR)/main.o $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o -lwiringPi

$(BUILD_DIR)/$(OBJR_DIR)/main.o: $(ORCHESTRATOR_DIR)/main.c
	$(CC) $(CFLAGS) -c $(ORCHESTRATOR_DIR)/main.c -o $(BUILD_DIR)/$(OBJR_DIR)/main.o

$(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o: $(HARDWARE_MANAGER_DIR)/hardwareManager.c execLed
	$(CC) $(CFLAGS) -c $(HARDWARE_MANAGER_DIR)/hardwareManager.c -o $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o

execLed:
	cd ./$(LIB_DIR)/rpi_ws281x && scons
	cd ../..
	
clean:
	rm -f $(BUILD_DIR)/$(OBJR_DIR)/*.o $(BUILD_DIR)/main $(LIB_DIR)/rpi_ws281x/test 
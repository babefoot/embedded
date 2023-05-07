#make a makefile for the program


#variables

CC = gcc
CFLAGS = -Wall -g

SRC= ./src
ORCHESTRATOR_DIR = ./src/orchestrator
HARDWARE_MANAGER_DIR = ./src/hardwareManager
SRVCONNECTION_DIR = ./src/serverConnection
BUILD_DIR = ./build
OBJR_DIR = objects
LIB_DIR = ./lib


main: $(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o $(BUILD_DIR)/$(OBJR_DIR)/main.o $(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o  $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o $(BUILD_DIR)/$(OBJR_DIR)/mq.o
	$(CC) $(CFLAGS) $(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o $(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o $(BUILD_DIR)/$(OBJR_DIR)/mq.o  $(BUILD_DIR)/$(OBJR_DIR)/main.o -o $(BUILD_DIR)/main -L$(LIB_DIR)/libws $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o -lwiringPi -lwsclient

$(BUILD_DIR)/$(OBJR_DIR)/main.o: $(SRC)/main.c 
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $(BUILD_DIR)/$(OBJR_DIR)/main.o

$(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o: $(SRVCONNECTION_DIR)/serverConnection.c
	$(CC) $(CFLAGS) -c $(SRVCONNECTION_DIR)/serverConnection.c -o $(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o

$(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o: $(HARDWARE_MANAGER_DIR)/hardwareManager.c execLed
	$(CC) $(CFLAGS) -c $(HARDWARE_MANAGER_DIR)/hardwareManager.c -o $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o

$(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o: $(ORCHESTRATOR_DIR)/orchestrator.c
	$(CC) $(CFLAGS) -c $(ORCHESTRATOR_DIR)/orchestrator.c -o $(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o

$(BUILD_DIR)/$(OBJR_DIR)/mq.o: $(LIB_DIR)/libMQ/mq.c
	$(CC) $(CFLAGS) -c $(LIB_DIR)/libMQ/mq.c -o $(BUILD_DIR)/$(OBJR_DIR)/mq.o

execLed:
	cd ./$(LIB_DIR)/rpi_ws281x && scons
	cd ../..

clean:
	rm -f $(BUILD_DIR)/$(OBJR_DIR)/*.o $(BUILD_DIR)/main $(LIB_DIR)/rpi_ws281x/test 
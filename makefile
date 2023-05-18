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
	$(CC) $(CFLAGS) $(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o $(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o $(BUILD_DIR)/$(OBJR_DIR)/mq.o  $(BUILD_DIR)/$(OBJR_DIR)/main.o -o $(BUILD_DIR)/main -L$(LIB_DIR)/libArmws2 -L$(LIB_DIR)/libRfid $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o -lwiringPi -lwsclient -lRfid -lbcm2835

$(BUILD_DIR)/$(OBJR_DIR)/main.o: $(SRC)/main.c 
	$(CC) $(CFLAGS) -c $(SRC)/main.c -o $(BUILD_DIR)/$(OBJR_DIR)/main.o

$(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o: $(SRVCONNECTION_DIR)/serverConnection.c
	$(CC) $(CFLAGS) -c $(SRVCONNECTION_DIR)/serverConnection.c -o $(BUILD_DIR)/$(OBJR_DIR)/serverConnection.o

$(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o: $(HARDWARE_MANAGER_DIR)/hardwareManager.c  #execLed
	$(CC) $(CFLAGS) -c $(HARDWARE_MANAGER_DIR)/hardwareManager.c -o $(BUILD_DIR)/$(OBJR_DIR)/hardwareManager.o

$(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o: $(ORCHESTRATOR_DIR)/orchestrator.c
	$(CC) $(CFLAGS) -c $(ORCHESTRATOR_DIR)/orchestrator.c -o $(BUILD_DIR)/$(OBJR_DIR)/orchestrator.o



# uniquement pour le RFID
#$(BUILD_DIR)/$(OBJR_DIR)/mq.o: $(LIB_DIR)/libMQ/mq.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libMQ/mq.c -o $(BUILD_DIR)/$(OBJR_DIR)/mq.o
#
#$(BUILD_DIR)/$(OBJR_DIR)/RFID/config.o: $(LIB_DIR)/libRfid/config.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libRfid/config.c -o $(BUILD_DIR)/$(OBJR_DIR)/RFID/config.o
#
#$(BUILD_DIR)/$(OBJR_DIR)/RFID/rc522.o: $(LIB_DIR)/libRfid/rc522.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libRfid/rc522.c -o $(BUILD_DIR)/$(OBJR_DIR)/RFID/rc522.o
#
#$(BUILD_DIR)/$(OBJR_DIR)/RFID/rfid_sensor.o: $(LIB_DIR)/libRfid/rfid_sensor.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libRfid/rfid_sensor.c -o $(BUILD_DIR)/$(OBJR_DIR)/RFID/rfid_sensor.o
#
#$(BUILD_DIR)/$(OBJR_DIR)/RFID/rfid.o: $(LIB_DIR)/libRfid/rfid.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libRfid/rfid.c -o $(BUILD_DIR)/$(OBJR_DIR)/RFID/rfid.o
#
#$(BUILD_DIR)/$(OBJR_DIR)/RFID/value.o: $(LIB_DIR)/libRfid/value.c
#	$(CC) $(CFLAGS) -c $(LIB_DIR)/libRfid/value.c -o $(BUILD_DIR)/$(OBJR_DIR)/RFID/value.o	

#ar rcs lib/libRfid/libRfid.a build/objects/RFID/config.o build/objects/RFID/rc522.o build/objects/RFID/rfid_sensor.o build/objects/RFID/rfid.o build/objects/RFID/value.o



#execLed:
#	cd ./$(LIB_DIR)/rpi_ws281x && scons
#	cd ../..

clean:
	rm -f $(BUILD_DIR)/$(OBJR_DIR)/*.o $(BUILD_DIR)/main
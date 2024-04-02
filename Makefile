# SPDX-License-Identifier: Apache-2.0
# ******************************************************************************
#
# @file			Makefile
#
# @brief        Makefile for Jack CXL Fabric Management CLI Tool
#
# @copyright    Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
#
# @date         Mar 2024
# @author       Barrett Edwards <code@jrlabs.io>
#
# ******************************************************************************

CC=gcc
CFLAGS= -g3 -O0 -Wall -Wextra
MACROS=-D JACK_VERBOSE
INCLUDE_DIR=/usr/local/include
LIB_DIR=/usr/local/lib
INCLUDE_PATH=-I $(INCLUDE_DIR) -I /usr/include/glib-2.0 -I /usr/lib/`uname -m`-linux-gnu/glib-2.0/include/
LIB_PATH=-L $(LIB_DIR)
LIBS=-l mctp -l fmapi -l emapi -l ptrqueue -l arrayutils -l uuid -l timeutils -l cxlstate -l pciutils
TARGET=jack

all: $(TARGET)

$(TARGET): main.c options.o ctrl_handler.o emapi_handler.o fmapi_handler.o cmd_encoder.o
	$(CC)    $^ $(CFLAGS) $(MACROS) $(INCLUDE_PATH) $(LIB_PATH) $(LIBS) -o $@

cmd_encoder.o: cmd_encoder.c cmd_encoder.h
	$(CC) -c $< $(CFLAGS) $(MACROS) $(INCLUDE_PATH) -o $@  

fmapi_handler.o: fmapi_handler.c fmapi_handler.h
	$(CC) -c $< $(CFLAGS) $(MACROS) $(INCLUDE_PATH) -o $@  

emapi_handler.o: emapi_handler.c emapi_handler.h
	$(CC) -c $< $(CFLAGS) $(MACROS) $(INCLUDE_PATH) -o $@  

ctrl_handler.o: ctrl_handler.c ctrl_handler.h
	$(CC) -c $< $(CFLAGS) $(MACROS) $(INCLUDE_PATH) -o $@  

options.o: options.c options.h
	$(CC) -c $< $(CFLAGS) $(MACROS) $(INCLUDE_PATH) -o $@  

clean:
	rm -rf ./*.o ./*.a $(TARGET)

doc: 
	doxygen

install: jack
	sudo cp $(TARGET) /usr/local/bin/
	sudo cp completion.bash /etc/bash_completion.d/$(TARGET)-completion.bash

# List all non file name targets as PHONY
.PHONY: all clean doc install

# Variables 
# $^ 	Will expand to be all the sensitivity list
# $< 	Will expand to be the frist file in sensitivity list
# $@	Will expand to be the target name (the left side of the ":" )
# -c 	gcc will compile but not try and link 

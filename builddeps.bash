#!/bin/bash
# ******************************************************************************
#
# @file			builddeps.bash 
#
# @brief        script to clone and build dependencies
#
# @copyright    Copyright (C) 2024 Jackrabbit Founders LLC. All rights reserved.
#
# @date         Apr 2024
# @author       Barrett Edwards <code@jrlabs.io>
#
# ******************************************************************************

ARG=$1
CUR=`pwd`
HOST=https://github.com/
ORG=JackrabbitLabs
REPOS="array_utils time_utils pci_utils ptr_queue emapi fmapi cxl_state mctp"

SRC=src
INC=include
LIB=lib

if [ "${ARG}" = "clean" ] ; then 
	echo "Removing build dependencies"
	rm -rf ${SRC}
	rm -rf ${INC}
	rm -rf ${LIB}
	exit
fi

# Create source directory if not present
if [ ! -d ${SRC} ] ; then 
	mkdir ${SRC}
fi
# Create include directory if not present
if [ ! -d ${INC} ] ; then 
	mkdir ${INC}
fi
# Create lib directory if not present
if [ ! -d ${LIB} ] ; then 
	mkdir ${LIB}
fi

# Clone sub module repos into DST directory 
for REPO in $REPOS ; do 
	echo "Cloning build dependencies"
	git clone ${HOST}${ORG}/${REPO}.git ${SRC}/${REPO}
done

# Build sub modules 
for REPO in $REPOS ; do 
	echo "Building dependencies" 
	cd ${SRC}/${REPO}/
	make INCLUDE_DIR=${CUR}/${INC} LIB_DIR=${CUR}/${LIB} CFLAGS="-g3 -O0" install 
	cd ${CUR}
done

# Issue completion message
echo "Completed building dependencies"


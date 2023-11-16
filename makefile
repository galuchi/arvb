# Set project directory one level above of Makefile directory. $(CURDIR) is a GNU make variable containing the path to the current working directory
PROJDIR := $(CURDIR)
SOURCEDIR := $(PROJDIR)/utils

OBJECTS = $(PROJDIR)/*.c

RESULT = $(PROJDIR)/rabisco.out

COMPILER = gcc

all:
	$(COMPILER) -g $(OBJECTS) -o $(RESULT)

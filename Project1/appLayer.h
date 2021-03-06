#ifndef APPLAYER
#define APPLAYER

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "StateMachine.h"

typedef struct applicationLayer {
	int fileDescriptor;	/*Descritor correspondente à porta de série*/
	int status;	/*TRANSMITTER | RECEIVER*/
	struct termios oldPortConfiguration;
	int targetFileDescriptor;
	char * fileName;
	unsigned int selectedFrameSize;
} applicationLayer;

//GETS
applicationLayer * getAppLayer (int fd, int mode);

int getFileDescriptor(applicationLayer *a);

int getStatus(applicationLayer *a);

struct termios getOldPortConfiguration(applicationLayer *a);

int getTargetDescriptor(applicationLayer *a);

char * getFileName(applicationLayer *a);

unsigned int getSelectedFrameSize(applicationLayer *a);

//DEFINES
void defineSelectedFrameSize(applicationLayer *a, unsigned int size);

void defineFileName(applicationLayer *a, char * name);

void setTargetDescriptor(applicationLayer *a, int fd);

void defineOldPortAttr(applicationLayer *a, struct termios b);

void defineFileDescriptor(applicationLayer *a, int fd);

void defineStatus(applicationLayer *a, int status);

void resetPortConfiguration(applicationLayer *a);

#endif

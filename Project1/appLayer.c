#include "appLayer.h"

applicationLayer * getAppLayer (int fd, int mode) {
	applicationLayer * a = (applicationLayer*) malloc(sizeof(applicationLayer));

	a->fileDescriptor = fd;
	a->status = mode;

	return a;
}

int getTargetDescriptor(applicationLayer *a) {
	return a->targetFileDescriptor;
}

void setTargetDescriptor(applicationLayer *a, int fd) {
	a->targetFileDescriptor = fd;
}

int getFileDescriptor(applicationLayer *a){
	return a->fileDescriptor;
}

int getStatus(applicationLayer *a){
	return a->status;
}

struct termios getOldPortConfiguration(applicationLayer *a){
	return a->oldPortConfiguration;
}

char * getFileName(applicationLayer *a) {
	return a->fileName;
}

unsigned int getSelectedFrameSize(applicationLayer *a) {
	return a->selectedFrameSize;
}

void defineSelectedFrameSize(applicationLayer *a, unsigned int size) {
	a->selectedFrameSize = size;
}
void defineFileName(applicationLayer *a, char * name) {
	a->fileName = (char *)malloc(strlen(name));
	strcpy(a->fileName, name);
}

void defineOldPortAttr(applicationLayer *a, struct termios b) {
	a->oldPortConfiguration = b;
}

void defineFileDescriptor(applicationLayer *a, int fd){
	a->fileDescriptor = fd;
}

void defineStatus(applicationLayer *a, int status){
	a->status = status;
}

void resetPortConfiguration(applicationLayer *a) {
	if(tcsetattr(a->fileDescriptor,TCSANOW, &a->oldPortConfiguration) == -1) {
		perror("signal tcsetattr");
		exit(-1);
	}
}

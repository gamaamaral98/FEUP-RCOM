#ifndef FTPCONTROLLER_H
#define FTPCONTROLLER_H

#include "url.h"

#define FRAME_LENGTH 1024

#define TIMEOUT_MAX_TRIES 3

#define SERVICE_READY_NEW_USER 220
#define SERVICE_NEED_PASSWORD 331
#define SERVICE_USER_LOGGEDIN 230
#define SERVICE_ENTERING_PASSIVE_MODE 227
#define SERVICE_FILE_OK 150
#define SERVICE_END_OF_DATA_CONNECTION 226
#define SERVICE_LOGOUT 221

typedef struct ftpController
{
  int controlFd;
  int dataFd;
  char *passiveIp;
  int passivePort;
} ftpController;

ftpController *getController();

void setFtpControlFileDescriptor(ftpController *x, int fd);

int setDataFileDescriptor(ftpController *x);

int setPassiveIpAndPort(ftpController *x, int *ipInfo);

int startConnection(char *ip, int port);

int ftpSendCommand(ftpController *connection, char *command);

int ftpExpectCommand(ftpController *connection, int expectation);

int retriveMessageFromServer(ftpController *connection, int expectation, char *message);

int login(ftpController *connection, url *link);

int *parsePassiveIp(char *serverMessage);

int enterPassiveMode(ftpController *connection);

int requestFile(ftpController *connection, url *link);

int downloadFile(ftpController *connection, url *link);

int logout(ftpController *connection);
#endif

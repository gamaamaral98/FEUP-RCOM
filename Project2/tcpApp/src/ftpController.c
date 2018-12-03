#include "ftpController.h"
#include "url.h"
#include "utilities.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <sys/types.h>

int flag = 0, tries = 1, maxTries = 1;

void timeOutWarning()
{
  flag = 0;
  printf("\nAtempting to connect %d/%d\n", tries, TIMEOUT_MAX_TRIES);
}

ftpController *getController()
{
  ftpController *x = malloc(sizeof(ftpController));

  return x;
}

void setFtpControlFileDescriptor(ftpController *x, int fd)
{
  x->controlFd = fd;
}

int setPassiveIpAndPort(ftpController *x, int *ipInfo)
{

  if ((sprintf(x->passiveIp, "%d.%d.%d.%d", ipInfo[0], ipInfo[1], ipInfo[2], ipInfo[3])) < 0)
  {
    printf("Failed to set passive ip address.\n");
    return FAIL;
  }

  x->passivePort = ipInfo[4] * 256 + ipInfo[5];

  return SUCCESS;
}

int setDataFileDescriptor(ftpController *x)
{
  if ((x->dataFd = startConnection(x->passiveIp, x->passivePort)) < 0)
  {
    printf("Cannot connect data socket");
    return FAIL;
  }

  return SUCCESS;
}

int startConnection(char *ip, int port)
{
  int fd;
  struct sockaddr_in serverAddr;

  bzero((char *)&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(ip); /*32 bit Internet address network byte ordered*/
  serverAddr.sin_port = htons(port);

  fd = socket(AF_INET, SOCK_STREAM, 0);

  if (fd < 0)
  {
    perror("Failed to open socket!");
    return FAIL;
  }

  if (connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    perror("Failed to connect to server");
    return FAIL;
  }

  return fd;
}

int ftpSendCommand(ftpController *connection, char *command)
{
  if (write(connection->controlFd, command, strlen(command)) != strlen(command))
  {
    return FAIL;
  }

  //printf("\nCOMMAND SENT: %s -> %ld\n", command, strlen(command));

  return SUCCESS;
}

int ftpExpectCommand(ftpController *connection, int expectation)
{
  int code = -1;
  int response;
  char frame[FRAME_LENGTH];
  char *codeAux = (char *)malloc(3);

  do
  {

    memset(frame, 0, FRAME_LENGTH);
    memset(codeAux, 0, 3);
    read(connection->controlFd, frame, FRAME_LENGTH);
    memcpy(codeAux, frame, 3);
    code = atoi(codeAux);

    if (tries == TIMEOUT_MAX_TRIES)
    {
      printf("TIMED OUT!");
      break;
    }

    printf("%s \n", frame);

    printf("%d =?= %d\n", code, expectation);

  } while (code != expectation || frame[3] != ' ');

  //printf("RESPONSE: %d\n", code);

  response = code == expectation ? SUCCESS : FAIL;

  free(codeAux);

  return response;
}

int retriveMessageFromServer(ftpController *connection, int expectation, char *message)
{
  char frame[FRAME_LENGTH];
  char *codeAux;
  int code = -1;

  codeAux = (char *)malloc(3);

  do
  {

    memset(frame, 0, FRAME_LENGTH);
    memset(codeAux, 0, 3);
    read(connection->controlFd, frame, FRAME_LENGTH);

    //printf("FRAME -> %s\n", frame);

    memcpy(codeAux, frame, 3);
    code = atoi(codeAux);

  } while (code != expectation && frame[3] != ' ');

  free(codeAux);

  if (code != expectation)
  {
    return FAIL;
  }

  memcpy(message, frame, strlen(frame));

  return SUCCESS;
}

int login(ftpController *connection, url *link)
{
  char *userCommand, *passwordCommand, *user, *password;

  user = link->user;

  password = link->password;

  userCommand = (char *)malloc(sizeof(user) + strlen("USER \r\n"));

  sprintf(userCommand, "USER %s\r\n", user);

  (void)signal(SIGALRM, timeOutWarning);

  if (ftpSendCommand(connection, userCommand) == FAIL)
  {
    printf("ERROR Sending user command\n");
    free(userCommand);
    return FAIL;
  }

  free(userCommand);
  //printf("User command sent!\n");

  if (ftpExpectCommand(connection, SERVICE_NEED_PASSWORD) == FAIL)
  {
    printf("ERROR didn't send need password command 331\n");
    return FAIL;
  }

  passwordCommand = (char *)malloc(sizeof(password) + strlen("PASS \r\n"));

  sprintf(passwordCommand, "PASS %s\r\n", password);

  if (ftpSendCommand(connection, passwordCommand) == FAIL)
  {
    printf("ERROR Sending user command\n");
    free(passwordCommand);
    return FAIL;
  }

  free(passwordCommand);

  if (ftpExpectCommand(connection, SERVICE_USER_LOGGEDIN) == FAIL)
  {
    printf("ERROR Wrong response from server\n");
    return FAIL;
  }

  return SUCCESS;
}

int *parsePassiveIp(char *serverMessage)
{
  int *ipInfo = (int *)malloc(6 * sizeof(int));

  if ((sscanf(serverMessage, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d).", &ipInfo[0], &ipInfo[1], &ipInfo[2], &ipInfo[3], &ipInfo[4], &ipInfo[5])) < 0)
  {
    printf("Couldn't parse passive IP info.\n");
    return NULL;
  }

  return ipInfo;
}

int enterPassiveMode(ftpController *connection)
{
  char *passiveCommand;
  char serverAnswer[FRAME_LENGTH];
  int *ipInfo;

  passiveCommand = (char *)malloc(strlen("PASV \r\n") * sizeof(char));

  memcpy(passiveCommand, "PASV \r\n", strlen("PASV \r\n"));

  if (ftpSendCommand(connection, passiveCommand) == FAIL)
  {
    printf("ERROR: can't send passive command!\n");
    free(passiveCommand);
    return FAIL;
  }

  free(passiveCommand);

  if(retriveMessageFromServer(connection, SERVICE_ENTERING_PASSIVE_MODE, serverAnswer) == FAIL) {
    printf("SERVER didn't answer");
    return FAIL;
  }

  ipInfo = parsePassiveIp(serverAnswer);

  setPassiveIpAndPort(connection, ipInfo);

  printf("PASSIVE IP: %s PORT: %d\n", connection->passiveIp, connection->passivePort);

  free(ipInfo);

  return SUCCESS;
}

int requestFile(ftpController *connection, url *link)
{
  char *filePath;
  char retrServerResponse[FRAME_LENGTH];

  filePath = link->path;

  char fileRequest[strlen("RETR \r\n") + strlen(filePath)];

  sprintf(fileRequest, "RETR %s\r\n", filePath);

  if (ftpSendCommand(connection, fileRequest) == FAIL)
  {
    printf("ERROR: can't send request command!\n");
    free(fileRequest);
    return FAIL;
  }
  printf("Receive server message.\n");
  if (retriveMessageFromServer(connection, SERVICE_FILE_OK, retrServerResponse) == FAIL)
  {
    printf("ERROR: request error\n");
    return FAIL;
  }

  printf("Server message received.\n");
  if (findFileSizeInServerMessage(link, retrServerResponse) == FAIL)
  {
    printf("Error analysing server message.\n");
    return FAIL;
  }

  return SUCCESS;
}

int downloadFile(ftpController *connection, url *link)
{
  printf("0\n");
  char fileName[256];
  int fileDescriptor = openFile(fileName);
  int readBytes = 0, receivedBytes = 0;
  char frame[FRAME_LENGTH];
  double begin, delta;

  printf("1\n");

  stripFileName(link->path, fileName);

  printf("2\n");

  if (fileDescriptor == -1)
  {
    printf("Could't open %s in current directory.\n", fileName);
    return FAIL;
  }

  begin = what_time_is_it();
  do
  {
    memset(frame, 0, FRAME_LENGTH);
    readBytes = read(connection->dataFd, frame, FRAME_LENGTH);
    if (readBytes == -1)
    {
      printf("ERROR reading from socket.\n");
      return FAIL;
    }

    write_frame(fileDescriptor, frame, readBytes);

    receivedBytes += readBytes;

    progressBar(link->fileSize, receivedBytes);
  } while (receivedBytes != link->fileSize);

  delta = what_time_is_it() - begin;

  printf("\nFile Received!\nElapsed time: %.3f seconds.\nAVG Speed: %.1f bit/s\t %.3f Mb/s\n", delta, link->fileSize / delta * 8, link->fileSize / delta / 1024 / 1024);

  printf("BEFORE\n");
  if (ftpExpectCommand(connection, SERVICE_END_OF_DATA_CONNECTION) == FAIL)
  {
    printf("Failed to receive host message to close data connection\n");
    return FAIL;
  }

  close(connection->dataFd);

  close(fileDescriptor);

  return SUCCESS;
}

int logout(ftpController *connection)
{
  printf("HERE");
  printf("CALL %p\n",calloc(6 , sizeof(char)));
  exit(1);
  char * logoutCommand = (char*)malloc(6 * sizeof(char) );

  sprintf(logoutCommand, "quit\n");

  if (ftpSendCommand(connection, logoutCommand) == FAIL)
  {
    printf("ERROR: can't send logout command!\n");
    free(logoutCommand);
    return FAIL;
  }

  free(logoutCommand);

  if (ftpExpectCommand(connection, SERVICE_LOGOUT) == FAIL)
  {
    printf("HOST didn't send logout command\n");
    return FAIL;
  }

  close(connection->controlFd);

  return SUCCESS;
}

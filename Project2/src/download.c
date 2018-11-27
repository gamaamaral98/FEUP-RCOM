#include <stdio.h>
#include "url.h"
#include "utilities.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
  url *link;
  int validation;

  if (argc != 2)
  {
    printUsage();
    return 0;
  }

  validation = validURL(argv[1]);

  if (validation == FAIL)
  {
    printUsage();
    printf("\n invalid URL.\n");
    return 0;
  }

  link = getUrl();

  setMode(link, validation);

  if (parseURL(link, argv[1]) == FAIL)
  {
    printf("Error parsing URL.\n");
    exit(0);
  }

  if (extractIp(link) == FAIL)
  {
    printf("Error getting IP from URL.\n");
    exit(0);
  }

  return 0;
}
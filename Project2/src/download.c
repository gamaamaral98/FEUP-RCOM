#include <stdio.h>
#include "url.h"
#include "utilities.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printUsage();
    return 0;
  }

  int validation = validURL(argv[1]);

  if (validation == FAIL)
  {
    printUsage();
    printf("\n invalid URL.\n");
    return 0;
  }

  url *link = getUrl();

  setMode(link, validation);

  if (parseURL(link, argv[1]) == FAIL)
  {
    printf("Error parsing URL.\n");
    exit(0);
  }

  return 0;
}
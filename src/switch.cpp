#include <switch.h>
#include <stdio.h>
#include <unistd.h>
#include <SDL.h>

void initSwitch()
{
  int nsError = nsInitialize();
  if (nsError != 0) {
    return;
  }
  socketInitialize(NULL);
  nxlinkConnectToHost(true, false);
  romfsInit();
}

void deinitSwitch()
{
  romfsExit();
  socketExit();
  nsExit();
}

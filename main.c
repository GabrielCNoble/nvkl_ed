#include "main.h"


int main(int arcg, char *argv[])
{
    g_SetInitCallback(ed_Init);
    g_SetShutdownCallback(ed_Shutdown);
    g_SetMainLoopCallback(ed_Main);
    g_MainLoop();
}

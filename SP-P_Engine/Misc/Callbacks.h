#pragma once

#include <pspsdk.h>
#include <pspkernel.h>
#include <psppower.h>

int ExitCallback(int arg1, int arg2, void* common);
int CallbackThread(SceSize args, void* argp);
int SetupCallbacks(void);
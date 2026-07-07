/*
Copyright (C) 2010 Crow_bar and MDave.
*/

#include "vram.h"
#include <pspsdk.h>
#include <pspge.h>
#include <pspkernel.h>
#include <pspsysevent.h>

PSP_MODULE_INFO("VramExt", 0x1006, 1, 0);
//PSP_MAIN_THREAD_ATTR(0);
PSP_HEAP_SIZE_KB(128);


int sceGeEdramSetSize(int);

void VramSetSize(int kb)
{
	int k1 = pspSdkSetK1(0);
    sceGeEdramSetSize(kb*1024);
	pspSdkSetK1(k1);
}

int module_start(SceSize args, void *argp)
{
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	return 0;
}


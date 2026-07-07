#include "SP-P_Engine/Misc/Callbacks.h"


/* 
    Колбек на выход из игры.
    Здесь мы сохраняем данные игры и чистим мусор.
    Данный кусок кода, по большому счету, перекочевывает из проекта в проект.
    Код меняется лишь в одном месте.
*/
int ExitCallback(int arg1, int arg2, void *common)
{
    //Здесь мы можем сохранить игру, к примеру
    //Меняется только этот фрагмент

    //Выходим
    sceKernelExitGame();
    return 0;
}

/* Регистрируем наши колбеки */
int CallbackThread(SceSize args, void *argp)
{
    int cbid;

    cbid = sceKernelCreateCallback("Exit Callback", ExitCallback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();

    return 0;
}

/* Создаем поток, который будет обрабатывать колбеки */
int SetupCallbacks(void)
{
    int thid = 0;

    thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
    if(thid >= 0)
    {
        sceKernelStartThread(thid, 0, 0);
    }

    return thid;
}

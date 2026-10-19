
#include <pspkernel.h>
#include <pspdebug.h>
#include <pspctrl.h>

PSP_MODULE_INFO("Runner PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main(void)
{
    pspDebugScreenInit();

    pspDebugScreenPrintf(
        "RUNNER PSP 3D\n"
        "Kaua Soares\n\n"
        "Teste de compilacao\n"
        "Pressione START para sair.\n"
    );

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_DIGITAL);

    while (1) {
        SceCtrlData pad;
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START) {
            break;
        }

        sceKernelDelayThread(10000);
    }

    sceKernelExitGame();
    return 0;
}

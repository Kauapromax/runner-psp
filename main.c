
#include <pspuser.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <pspkernel.h>
#include <math.h>
#include <string.h>

PSP_MODULE_INFO("Runner PSP", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#define W 480
#define H 272
#define BUF 512

static unsigned int __attribute__((aligned(16))) list[262144];
static unsigned int __attribute__((aligned(16))) frame1[BUF * H];
static unsigned int __attribute__((aligned(16))) frame2[BUF * H];
static unsigned short __attribute__((aligned(16))) depth[BUF * H];

typedef struct {
    unsigned int color;
    float x, y, z;
} Vertex;

#define V(c,x,y,z) {c,x,y,z}

static Vertex cube[] = {
    V(0xff3333ff,-.5f,-.5f,-.5f), V(0xff3333ff,.5f,-.5f,-.5f), V(0xff3333ff,.5f,.5f,-.5f),
    V(0xff3333ff,-.5f,-.5f,-.5f), V(0xff3333ff,.5f,.5f,-.5f), V(0xff3333ff,-.5f,.5f,-.5f),
    V(0xff33cc33,-.5f,-.5f,.5f), V(0xff33cc33,.5f,-.5f,.5f), V(0xff33cc33,.5f,.5f,.5f),
    V(0xff33cc33,-.5f,-.5f,.5f), V(0xff33cc33,.5f,.5f,.5f), V(0xff33cc33,-.5f,.5f,.5f),
    V(0xff33ffff,-.5f,-.5f,-.5f), V(0xff33ffff,-.5f,.5f,-.5f), V(0xff33ffff,-.5f,.5f,.5f),
    V(0xff33ffff,-.5f,-.5f,-.5f), V(0xff33ffff,-.5f,.5f,.5f), V(0xff33ffff,-.5f,-.5f,.5f),
    V(0xffffaa33,.5f,-.5f,-.5f), V(0xffffaa33,.5f,.5f,-.5f), V(0xffffaa33,.5f,.5f,.5f),
    V(0xffffaa33,.5f,-.5f,-.5f), V(0xffffaa33,.5f,.5f,.5f), V(0xffffaa33,.5f,-.5f,.5f),
    V(0xffaaaaaa,-.5f,.5f,-.5f), V(0xffaaaaaa,.5f,.5f,-.5f), V(0xffaaaaaa,.5f,.5f,.5f),
    V(0xffaaaaaa,-.5f,.5f,-.5f), V(0xffaaaaaa,.5f,.5f,.5f), V(0xffaaaaaa,-.5f,.5f,.5f),
    V(0xff777777,-.5f,-.5f,-.5f), V(0xff777777,.5f,-.5f,-.5f), V(0xff777777,.5f,-.5f,.5f),
    V(0xff777777,-.5f,-.5f,-.5f), V(0xff777777,.5f,-.5f,.5f), V(0xff777777,-.5f,-.5f,.5f)
};

static int lane = 0;
static float jump = 0;
static float distanceRun = 0;
static float speed = 0.12f;
static float obsZ[3] = {-12, -22, -32};
static int obsLane[3] = {0, 1, -1};
static int score = 0;
static int gameOver = 0;

static void drawCube(float x, float y, float z,
                     float sx, float sy, float sz)
{
    ScePspFVector3 pos = {x, y, z};
    ScePspFVector3 scale = {sx, sy, sz};

    sceGumMatrixMode(GU_MODEL);
    sceGumPushMatrix();
    sceGumLoadIdentity();
    sceGumTranslate(&pos);
    sceGumScale(&scale);
    sceGumDrawArray(GU_TRIANGLES,
        GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
        sizeof(cube) / sizeof(cube[0]), 0, cube);
    sceGumPopMatrix();
}

static void drawScene(void)
{
    sceGuStart(GU_DIRECT, list);
    sceGuClearColor(0xffb9d9f5);
    sceGuClearDepth(0);
    sceGuClear(GU_COLOR_BUFFER_BIT | GU_DEPTH_BUFFER_BIT);

    sceGumMatrixMode(GU_PROJECTION);
    sceGumLoadIdentity();
    sceGumPerspective(55.0f, 16.0f / 9.0f, 0.5f, 100.0f);

    sceGumMatrixMode(GU_VIEW);
    sceGumLoadIdentity();

    sceGumMatrixMode(GU_MODEL);
    sceGumLoadIdentity();

    /* Pista e faixas */
    drawCube(0, -1.2f, -18, 3.6f, .2f, 40);
    drawCube(-.65f, -1.08f, -18, .035f, .025f, 40);
    drawCube(.65f, -1.08f, -18, .035f, .025f, 40);

    /* Personagem */
    if (!gameOver)
        drawCube((float)lane * 1.05f, -.35f + jump, -5,
                 .48f, .85f, .45f);

    /* Obstáculos */
    if (!gameOver) {
        int i;
        for (i = 0; i < 3; i++) {
            drawCube((float)obsLane[i] * 1.05f, -.55f,
                     obsZ[i], .7f, .8f, .65f);
        }
    }

    sceGuFinish();
    sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
    sceDisplayWaitVblankStart();
    sceGuSwapBuffers();
}

int main(void)
{
    SceCtrlData pad;
    unsigned int oldButtons = 0;

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    sceGuInit();
    sceGuStart(GU_DIRECT, list);
    sceGuDrawBuffer(GU_PSM_8888, (void *)0, BUF);
    sceGuDispBuffer(W, H, (void *)(BUF * H * 4), BUF);
    sceGuDepthBuffer((void *)(BUF * H * 8), BUF);
    sceGuOffset(2048 - W / 2, 2048 - H / 2);
    sceGuViewport(2048, 2048, W, H);
    sceGuDepthRange(65535, 0);
    sceGuScissor(0, 0, W, H);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuEnable(GU_DEPTH_TEST);
    sceGuDepthFunc(GU_GEQUAL);
    sceGuFrontFace(GU_CW);
    sceGuShadeModel(GU_SMOOTH);
    sceGuFinish();
    sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
    sceDisplayWaitVblankStart();
    sceGuDisplay(GU_TRUE);

    while (1) {
        int i;
        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons & PSP_CTRL_START)
            sceKernelExitGame();

        if (!gameOver) {
            if ((pad.Buttons & PSP_CTRL_LEFT) &&
                !(oldButtons & PSP_CTRL_LEFT) && lane > -1)
                lane--;

            if ((pad.Buttons & PSP_CTRL_RIGHT) &&
                !(oldButtons & PSP_CTRL_RIGHT) && lane < 1)
                lane++;

            if ((pad.Buttons & PSP_CTRL_CROSS) &&
                !(oldButtons & PSP_CTRL_CROSS) && jump < .05f)
                jump = .9f;

            if (jump > 0)
                jump -= .055f;
            if (jump < 0)
                jump = 0;

            distanceRun += speed;
            score++;

            if (distanceRun > 100) {
                distanceRun = 0;
                if (speed < .24f)
                    speed += .005f;
            }

            for (i = 0; i < 3; i++) {
                obsZ[i] += speed;

                if (obsZ[i] > -4.0f) {
                    if (obsLane[i] == lane && jump < .25f)
                        gameOver = 1;

                    obsZ[i] = -32.0f - (float)(i * 5);
                    obsLane[i] = (i + score) % 3 - 1;
                }
            }
        } else if (pad.Buttons & PSP_CTRL_CROSS) {
            lane = 0;
            jump = 0;
            score = 0;
            speed = .12f;
            gameOver = 0;
            obsZ[0] = -12;
            obsZ[1] = -22;
            obsZ[2] = -32;
        }

        oldButtons = pad.Buttons;
        drawScene();
    }

    return 0;
}


#include  <app_cfg.h>
#include  <os.h>
#include  <stdio.h>
#include  <lib_mem.h>
#include  <lib_math.h>
#include <SDL2/SDL.h>
#undef main

#include "lcd.h"

static OS_TCB        App_TaskStartTCB;
static CPU_STK_SIZE  App_TaskStartStk[APP_CFG_TASK_START_STK_SIZE];
static void App_TaskStart(void *p_arg);

static OS_TCB        gfxTCB;
static CPU_STK_SIZE  gfxStk[APP_CFG_TASK_START_STK_SIZE];
static void gfxThread(void *arg);

static int running = 0;

void drawRect(int x, int y, int width, int height, uint16_t color)
{
    int x_max = x + width;
    int y_max = y + height;
    if(x_max > lcd_screenWidth()) x_max = lcd_screenWidth();
    if(y_max > lcd_screenHeight()) y_max = lcd_screenHeight();
    uint16_t *buf = lcd_getFrameBuffer();

    for(int i=y; i < y_max; i++)
    {
        for(int j=x; j < x_max; j++)
        {
            buf[j + i*lcd_screenWidth()] = color;
        }
    }
}

void clearScreen()
{
    uint16_t *buf = lcd_getFrameBuffer();

    for(int i=0; i < lcd_screenHeight(); i++)
    {
        for(int j=0; j < lcd_screenWidth(); j++)
        {
            buf[j + i*lcd_screenWidth()] = 0xFFFF;
        }
    }
}

int  main (void)
{
    OS_ERR  err;
    running = 1;

    OSInit(&err);
    OS_CPU_SysTickInit();

    OSTaskCreate((OS_TCB     *)&App_TaskStartTCB,
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR ) App_TaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_CFG_TASK_START_PRIO,
                 (CPU_STK    *)&App_TaskStartStk[0],
                 (CPU_STK     )(APP_CFG_TASK_START_STK_SIZE / 10u),
                 (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0,
                 (OS_TICK     ) 0,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

     OSTaskCreate((OS_TCB     *)&gfxTCB,
                 (CPU_CHAR   *)"gfx",
                 (OS_TASK_PTR ) gfxThread,
                 (void       *) 0,
                 (OS_PRIO     ) APP_CFG_TASK_START_PRIO,
                 (CPU_STK    *)&gfxStk[0],
                 (CPU_STK     )(APP_CFG_TASK_START_STK_SIZE / 10u),
                 (CPU_STK_SIZE) APP_CFG_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0,
                 (OS_TICK     ) 0,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);

    printf("OSStart returned, quitting\n");
    return 0;
}


static void App_TaskStart(void *p_arg)
{

    (void) p_arg;
    OS_ERR os_err;

//     OS_CPU_SysTickInit();

    while (running)
    {
        printf("uCOS-III is running.\n");
        OSTimeDlyHMSM(0u, 0u, 1u, 0u, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }

    exit(1);
}

static void gfxThread(void *arg)
{
    (void) arg;
    OS_ERR os_err;

    int pos = 0;
    SDL_Event eventListener;

    lcd_init();

    while(1)
    {
        SDL_PollEvent(&eventListener);
        if(eventListener.type == SDL_QUIT) break;

        clearScreen();
        drawRect(0, pos, lcd_screenWidth(), 20, 0xF800);
        lcd_render();
        while(lcd_renderingInProgress()) ;
        pos += 20;
        if(pos > lcd_screenHeight() - 20) pos = 0;

        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &os_err);
    }

    running = 0;
    OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_STRICT, &os_err);
    lcd_terminate();
}
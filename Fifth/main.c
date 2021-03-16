/**
 * @file    main.c
 * @brief   Main code.
 *
 * @addtogroup Main
 * @{
 */
#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "stdio.h" 
#include "stdlib.h" 

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(512)

static THD_WORKING_AREA(waLED, 128);

/**
 * @brief Blinking Led 13
 * 
 */
static THD_FUNCTION(thdLED, arg)
{

  (void)arg;

  chRegSetThreadName("blinker");
  while (true)
  {
    palTogglePad(GPIOC, 13);
    chThdSleepMilliseconds(100);
  }
};

static thread_t *shelltp = NULL;

/**
 * @brief Fungsi Cek Serial
 * @details Mengirim "Serial OK" ke Serial
 * 
 * @param chp 
 * @param argc 
 * @param argv 
 */
static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Serial OK\n");
};

/**
 * @brief Fungsi Kalkulator Serial
 * @details Menjumlahkan Argumen 1 dan 2
 * 
 * @param chp 
 * @param argc hitungan argumen
 * @param argv vektor argumen
 */
static void cmd_calc(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;

  long result;
  long x1 = strtol(argv[0], NULL, 10);
  long x2 = strtol(argv[1], NULL, 10);
  result = x1 + x2;

  chprintf(chp, "Result: %li\n", result);
};


/**
 * @brief Array Command
 * 
 */
static const ShellCommand commands[] = {
    {"test", cmd_test},
    {"calc", cmd_calc},
    {NULL, NULL}
};

/**
 * @brief 
 * 
 */
static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SD1,
    commands
};

int main(void)
{

  halInit();
  chSysInit();

  palSetPadMode(GPIOC, 13, PAL_MODE_OUTPUT_PUSHPULL);
  palClearPad(GPIOC, 13);
  chThdCreateStatic(waLED, sizeof(waLED), NORMALPRIO, thdLED, NULL);

  palSetPadMode(GPIOA, 9, PAL_MODE_STM32_ALTERNATE_PUSHPULL); //TX
  palSetPadMode(GPIOA, 10, PAL_MODE_INPUT);                   //RX
  sdStart(&SD1, NULL);

  shellInit();

  while (true)
  {
    if (!shelltp)
      shelltp = shellCreate(&shell_cfg, SHELL_WA_SIZE, NORMALPRIO);
    else if (chThdTerminatedX(shelltp))
    {
      chThdRelease(shelltp);
      shelltp = NULL;
    }
    chThdSleepMilliseconds(100);
  }
}

/** @} */
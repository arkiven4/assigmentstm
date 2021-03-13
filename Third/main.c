#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "stdio.h" 
#include "stdlib.h" 

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(512)

static THD_WORKING_AREA(waLED, 128);
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

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  (void)argc;

  chprintf(chp, "Serial OK\n");
};

static void cmd_calc(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argc;

  long result;
  long x1 = strtol(argv[0], NULL, 10);
  long x2 = strtol(argv[1], NULL, 10);
  result = x1 + x2;

  chprintf(chp, "Result: %li\n", result);
};

static void usage(BaseSequentialStream *chp, char *p) {
  chprintf(chp, "Usage: %s\r\n", p);
}

static void list_commands(BaseSequentialStream *chp, const ShellCommand *scp) {
  chprintf(chp, "\n");
  while (scp->sc_name != NULL) {
    chprintf(chp, "%s ", scp->sc_name);
    scp++;
  }
}

static void cmd_info(BaseSequentialStream *chp, int argc, char *argv[])
{
  (void)argv;
  if (argc > 0)
  {
    usage(chp, "info");
    return;
  }
  chprintf(chp, "\n");
  chprintf(chp, "Kernel:       %s\r\n", CH_KERNEL_VERSION);
  #ifdef PORT_COMPILER_NAME
    chprintf(chp, "Compiler:     %s\r\n", PORT_COMPILER_NAME);
  #endif
    chprintf(chp, "Architecture: %s\r\n", PORT_ARCHITECTURE_NAME);
  #ifdef PORT_CORE_VARIANT_NAME
    chprintf(chp, "Core Variant: %s\r\n", PORT_CORE_VARIANT_NAME);
  #endif
  #ifdef PORT_INFO
    chprintf(chp, "Port Info:    %s\r\n", PORT_INFO);
  #endif
  #ifdef PLATFORM_NAME
    chprintf(chp, "Platform:     %s\r\n", PLATFORM_NAME);
  #endif
  #ifdef BOARD_NAME
    chprintf(chp, "Board:        %s\r\n", BOARD_NAME);
  #endif
  #ifdef __DATE__
  #ifdef __TIME__
    chprintf(chp, "Build time:   %s%s%s\r\n", __DATE__, " - ", __TIME__);
  #endif
  #endif
};

static const ShellCommand commands[] = {
    {"test", cmd_test},
    //{"info", cmd_info},
    //{"help", list_commands},
    {"calc", cmd_calc},
    {NULL, NULL}
};

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

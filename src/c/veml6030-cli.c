#include "veml6030.h"

int main(int argc, char **argv) {
  VEML6030_init("/dev/i2c-3", 0x48);
  
  uint16_t gain, it, pers, sd;
  VEML6030_access_config(&gain, &it, &pers, &sd);
  printf("Config: %04x, %04x, %04x, %04x\n", gain, it, pers, sd);

  int counter = 0;
  while (counter++ < 60) {
    double als, white;
    VEML6030_access_als(&als);
    VEML6030_access_white(&white);
    printf("ALS: %f, White: %f\n", als, white);
    sleep(1);
  }

  VEML6030_deinit();  
}

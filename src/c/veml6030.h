#ifndef VEML6030
#define VEML6030

#include <stdint.h>
#include <stdio.h>

// Print function that only prints if DEBUG is defined
#ifdef DEBUG
#define DEBUG_PRINT 1
#else
#define DEBUG_PRINT 0
#endif
#define debug_print(fd, fmt, ...) \
            do { if (DEBUG_PRINT) fprintf(fd, fmt, __VA_ARGS__); } while (0)

enum Error {
  NO_ERROR,
  ERROR_DEVICE,         // Couldn't find device
  ERROR_DRIVER,         // Driver failed to init
  ERROR_INVAL,          // Invalid argument
  ERROR_I2C             // I2C driver failed to read or write data
};

// Chip defines
enum als_gain_s {
  GAIN_1      = 0x0000,
  GAIN_2      = 0x0800,
  GAIN_0_125  = 0x1000,
  GAIN_0_25   = 0x1800
};

enum als_it_s {
  IT_25   = 0x0300,
  IT_50   = 0x0200,
  IT_100  = 0x0000,
  IT_200  = 0x0040,
  IT_400  = 0x0080,
  IT_800  = 0x00C0
};

enum als_pers_s {
  PERS_1 = 0x0000,
  PERS_2 = 0x0010,
  PERS_4 = 0x0020,
  PERS_8 = 0x0030
};

enum als_sd_s {
  SLEEP = 0x0001,
  ON    = 0x0000
};

enum psm_s {
  PSM_MODE_1 = 0x0000,
  PSM_MODE_2 = 0x0002,
  PSM_MODE_3 = 0x0004,
  PSM_MODE_4 = 0x0006
};

enum psm_en_s {
  PSM_DISABLE = 0x0000,
  PSM_ENABLE  = 0x0001
};

#define VEML6030_CONF_REG     0x00
#define VEML6030_PS_REG       0x03
#define VEML6030_ALS_REG      0x04
#define VEML6030_WHITE_REG    0x05

#define VEML6030_GAIN_MASK    0x1800
#define VEML6030_IT_MASK      0x03C0
#define VEML6030_PERS_MASK    0x0030
#define VEML6030_SD_MASK      0x0001

#define VEML6030_PSM_MASK     0x0006;
#define VEML6030_PSM_EN_MASK  0x0001;

// Set up and tear down VEML6030 interface
int VEML6030_init(const char *spi_adaptor, const int address);
int VEML6030_deinit(void);

// Fetch data from VEML6030
int VEML6030_access_als(double *als_out);
int VEML6030_access_white(double *white_out);
int VEML6030_access_config(uint16_t *gain_out,
                           uint16_t *it_out,
                           uint16_t *pers_out,
                           uint16_t *sd_out);
int VEML6030_access_ps(uint16_t *refresh_out,
                       uint16_t *psm_en_out);

// Set data in VEML6030 
int VEML6030_set_config(uint16_t gain,
                        uint16_t it,
                        uint16_t pers,
                        uint16_t sd);
int VEML6030_set_ps(uint16_t psm,
                    uint16_t psm_en);

#endif // VEML6030


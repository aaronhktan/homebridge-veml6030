#include "veml6030.h"

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <linux/types.h>
#include <sys/ioctl.h>

static int i2c_fd;
static int i2c_address;

enum als_gain_s gain_setting;
enum als_it_s it_setting;
static double lux_table[6][4] = {
  { 0.0036, 0.0072, 0.0288, 0.0576 },
  { 0.0072, 0.0144, 0.0576, 0.1152 },
  { 0.0144, 0.0288, 0.1152, 0.2304 },
  { 0.0288, 0.0576, 0.2304, 0.4608 },
  { 0.0576, 0.1152, 0.4608, 0.9216 },
  { 0.1152, 0.2304, 0.9216, 1.8432 }
};

static int i2c_smbus_access(int file, char read_write, __u8 command,
                            int size, union i2c_smbus_data *data)
{
  struct i2c_smbus_ioctl_data args;
  __s32 err;

  args.read_write = read_write;
  args.command = command;
  args.size = size;
  args.data = data;

  err = ioctl(file, I2C_SMBUS, &args);
  if (err == -1) {
    err = -errno;
  }
  return err;
}

static int i2c_write(int fd, uint8_t command, uint16_t word) {
  union i2c_smbus_data word_data;
  word_data.word = word;
  int err = i2c_smbus_access(fd, I2C_SMBUS_WRITE,
      command, I2C_SMBUS_WORD_DATA, &word_data);
  return err;
}

static int i2c_read(int fd, uint8_t command, uint16_t *word) {
  union i2c_smbus_data word_data;
  int err = i2c_smbus_access(fd, I2C_SMBUS_READ,
      command, I2C_SMBUS_WORD_DATA, &word_data);
  if (!err) {
    *word = word_data.word;
  }
  return err;
}

static int calculate_lux(uint16_t data, double *lux_out) {
  int column = -1, row = -1;
  
  switch (gain_setting) {
    case GAIN_2:
      column = 0;
      break;
    case GAIN_1:
      column = 1;
      break;
    case GAIN_0_125:
      column = 2;
      break;
    case GAIN_0_25:
      column = 3;
      break;
  }

  switch (it_setting) {
    case IT_800:
      row = 0;
      break;
    case IT_400:
      row = 1;
      break;
    case IT_200:
      row = 2;
      break;
    case IT_100:
      row = 3;
      break;
    case IT_50:
      row = 4;
      break;
    case IT_25:
      row = 5;
      break; 
  }

  debug_print(stdout, "Looking for resolution in row %d, column %d\n", row, column);
  if (column < 0 || row < 0) {
    return ERROR_INVAL;
  }

  double resolution = lux_table[row][column];
  debug_print(stdout, "Resolution: %f\n", resolution);
  *lux_out = data * resolution;
  return NO_ERROR;
} 

int VEML6030_init(const char *i2c_adaptor, const int address) {
  i2c_fd = open(i2c_adaptor, O_RDWR);
  if (i2c_fd == -1) {
    return ERROR_DEVICE;
  }
  i2c_address = address;
  int err = ioctl(i2c_fd, I2C_SLAVE, i2c_address);
  if (err) {
    return ERROR_DRIVER;
  }

  err = VEML6030_set_config(GAIN_2, IT_100, PERS_1, ON);
  if (err) {
    return ERROR_I2C; 
  }

  return NO_ERROR;
}

int VEML6030_deinit(void) {
  close(i2c_fd);
  return NO_ERROR;
}

int VEML6030_access_als(double *als_out) {
  uint16_t data;
  int err = i2c_read(i2c_fd, VEML6030_ALS_REG, &data);
  if (err) {
    return ERROR_I2C;
  }

  double als;
  err = calculate_lux(data, &als);
  if (!err) {
    *als_out = als;
  }
  return err;
}

int VEML6030_access_white(double *white_out) {
  uint16_t data;
  int err = i2c_read(i2c_fd, VEML6030_WHITE_REG, &data);
  if (err) {
    return ERROR_I2C;
  }

  double white;
  err = calculate_lux(data, &white);
  if (!err) {
    *white_out = white;
  }
  return err;
}

int VEML6030_access_config(uint16_t *gain_out, uint16_t *it_out,
                           uint16_t *pers_out, uint16_t *sd_out) {
  uint16_t data;
  int err = i2c_read(i2c_fd, VEML6030_CONF_REG, &data);
  if (err) {
    return ERROR_I2C;
  }

  *gain_out = data & VEML6030_GAIN_MASK;
  *it_out   = data & VEML6030_IT_MASK;
  *pers_out = data & VEML6030_PERS_MASK;
  *sd_out   = data & VEML6030_SD_MASK;
  return NO_ERROR;
}

int VEML6030_access_ps(uint16_t *refresh_out, uint16_t *psm_en_out) {
  uint16_t data;
  int err = i2c_read(i2c_fd, VEML6030_PS_REG, &data);
  if (err) {
    return ERROR_I2C;
  }

  *refresh_out = data & VEML6030_PSM_MASK;
  *psm_en_out = data & VEML6030_PSM_EN_MASK;
  return NO_ERROR;
}

int VEML6030_set_config(uint16_t gain, uint16_t it,
                        uint16_t pers, uint16_t sd) {
  gain_setting = gain;
  it_setting = it;
  uint16_t data = (gain | it | pers | sd) & 0xFFFF;
  return i2c_write(i2c_fd, VEML6030_CONF_REG, data);
}

int VEML6030_set_ps(uint16_t psm, uint16_t psm_en) {
  uint16_t data = (psm | psm_en) & 0x000F;
  return i2c_write(i2c_fd, VEML6030_PS_REG, data);
}


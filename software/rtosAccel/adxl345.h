#ifndef ADXL345_H
#define ADXL345_H

#include "alt_types.h"

#define ADXL345_REG_DEVID        0x00
#define ADXL345_REG_POWER_CTL    0x2D
#define ADXL345_REG_DATA_FORMAT  0x31
#define ADXL345_REG_DATAX0       0x32

int adxl345_init(void);
int adxl345_read_reg(alt_u8 reg, alt_u8 *value);
int adxl345_read_xyz(alt_16 *x, alt_16 *y, alt_16 *z);

#endif

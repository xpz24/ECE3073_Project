#include "adxl345.h"
#include "system.h"
#include "altera_avalon_spi.h"

#define ADXL345_SPI_BASE   ACCELEROMETER_BASE
#define ADXL345_SPI_SLAVE  0

/* Internal helper: write one ADXL345 register */
static int adxl345_write_reg(alt_u8 reg, alt_u8 value)
{
    alt_u8 tx[2];
    int rc;

    tx[0] = reg & 0x3F;   /* write=0, multibyte=0, addr=reg[5:0] */
    tx[1] = value;

    rc = alt_avalon_spi_command(
        ADXL345_SPI_BASE,
        ADXL345_SPI_SLAVE,
        2,
        tx,
        0,
        0,
        0
    );

    if (rc != 0) {
        return -1;
    }

    return 0;
}

/* Public: read one ADXL345 register */
int adxl345_read_reg(alt_u8 reg, alt_u8 *value)
{
    alt_u8 cmd;
    int rc;

    if (value == 0) {
        return -1;
    }

    cmd = 0x80 | (reg & 0x3F);   /* read=1, multibyte=0 */

    rc = alt_avalon_spi_command(
        ADXL345_SPI_BASE,
        ADXL345_SPI_SLAVE,
        1,
        &cmd,
        1,
        value,
        0
    );

    if (rc != 1) {
        return -1;
    }

    return 0;
}

/* Public: initialize ADXL345 */
int adxl345_init(void)
{
    if (adxl345_write_reg(ADXL345_REG_DATA_FORMAT, 0x00) != 0) {
        return -1;
    }

    if (adxl345_write_reg(ADXL345_REG_POWER_CTL, 0x08) != 0) {
        return -1;
    }

    return 0;
}

/* Public: read X, Y, Z using one multibyte read starting at DATAX0 */
int adxl345_read_xyz(alt_16 *x, alt_16 *y, alt_16 *z)
{
    alt_u8 cmd;
    alt_u8 data[6];
    int rc;

    if ((x == 0) || (y == 0) || (z == 0)) {
        return -1;
    }

    cmd = 0x80 | 0x40 | ADXL345_REG_DATAX0;   /* read=1, multibyte=1 */

    rc = alt_avalon_spi_command(
        ADXL345_SPI_BASE,
        ADXL345_SPI_SLAVE,
        1,
        &cmd,
        6,
        data,
        0
    );

    if (rc != 6) {
        return -1;
    }

    *x = (alt_16)((data[1] << 8) | data[0]);
    *y = (alt_16)((data[3] << 8) | data[2]);
    *z = (alt_16)((data[5] << 8) | data[4]);

    return 0;
}

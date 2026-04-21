#include <stdio.h>
#include "adxl345.h"
#include "priv\alt_busy_sleep.h"

int main(void)
{
    alt_16 x, y, z;
    int rc;

    printf("[ACCEL] Starting...\n");

    rc = adxl345_init();
    if (rc != 0) {
        printf("[ACCEL] Init failed\n");
        return -1;
    }

    printf("[ACCEL] Init OK\n");

    while (1) {
        rc = adxl345_read_xyz(&x, &y, &z);

        if (rc != 0) {
            printf("[ACCEL] Read failed\n");
        } else {
            printf("X=%d Y=%d Z=%d\n", x, y, z);
        }

        /* Sleep for 1 second (1,000,000 microseconds) */
        alt_busy_sleep(1000000);
    }

    return 0;
}

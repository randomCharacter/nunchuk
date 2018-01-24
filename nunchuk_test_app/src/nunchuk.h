#ifndef NUNCHUK_H
#define NUNCHUK_H

#define DEVICE_NAME "/dev/nunchuk"

typedef struct
{
    unsigned char x;
    unsigned char y;
    unsigned short x_gyro;
    unsigned short y_gyro;
    unsigned short z_gyro;
    unsigned char c;
    unsigned char z;
} nunchuk;

#endif //NUNCHUK_H

#include <stdio.h>

#include "nunchuk.h"

int main()
{
    while (1)
    {
        FILE *f;
        if ((f = fopen(DEVICE_NAME, "rb")) == NULL)
        {
            printf("fopen failed.\n");
            return -1;
        }

        nunchuk n;

        if (fread(&n, sizeof(nunchuk), 1, f) <= 0)
        {
            printf("fread failed.\n");
            return -1;
        }

        printf("%3u %3u %3u %3u %3u %u %u\n",
               (unsigned int) n.x,
               (unsigned int) n.y,
               (unsigned int) n.x_gyro,
               (unsigned int) n.y_gyro,
               (unsigned int) n.z_gyro,
               (unsigned int) n.c,
               (unsigned int) n.z);

        fclose(f);
    }
}

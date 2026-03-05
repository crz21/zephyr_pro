#include "main.h"

#include <zephyr/kernel.h>

int main(void)
{
    for (;;) {
        k_msleep(10);
    }
    return 0;
}

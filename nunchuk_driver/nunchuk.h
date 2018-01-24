#ifndef NUNCHUK_H_INCLUDED
#define NUNCHUK_H_INCLUDED

/* Includes */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/ktime.h>
#include <linux/hrtimer.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

/* GPIO base address */
#define GPIO_BASE_ADDR (0x3F200000)

/* Controller register base address */
#define BSC1_BASE_ADDR (0x3F804000)
 
/* BSC1 registers i hit */
#define BSC1_REG_C (BSC1_BASE_ADDR + 0x00000000)
#define BSC1_REG_S (BSC1_BASE_ADDR + 0x00000004)
#define BSC1_REG_DLEN (BSC1_BASE_ADDR + 0x00000008)
#define BSC1_REG_SLAVE_ADDR (BSC1_BASE_ADDR + 0x0000000C)
#define BSC1_REG_FIFO (BSC1_BASE_ADDR + 0x00000010)
#define BSC1_REG_DIV (BSC1_BASE_ADDR + 0x00000014)
#define BSC1_REG_DEL (BSC1_BASE_ADDR + 0x00000018)
#define BSC1_REG_CLKT (BSC1_BASE_ADDR + 0x0000001C)


#define GPFSEL0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000000)
#define GPFSEL1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000004)
#define GPFSEL2_BASE_ADDR (GPIO_BASE_ADDR + 0x00000008)
#define GPFSEL3_BASE_ADDR (GPIO_BASE_ADDR + 0x0000000C)
#define GPFSEL4_BASE_ADDR (GPIO_BASE_ADDR + 0x00000010)
#define GPFSEL5_BASE_ADDR (GPIO_BASE_ADDR + 0x00000014)
#define GPSET0_BASE_ADDR (GPIO_BASE_ADDR + 0x0000001C)
#define GPSET1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000020)
#define GPCLR0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000028)
#define GPCLR1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000002C)
#define GPLEV0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000034)
#define GPLEV1_BASE_ADDR (GPIO_BASE_ADDR + 0x00000038)
#define GPPUD_BASE_ADDR (GPIO_BASE_ADDR + 0x00000094)
#define GPPUDCLK0_BASE_ADDR (GPIO_BASE_ADDR + 0x00000098)
#define GPPUDCLK1_BASE_ADDR (GPIO_BASE_ADDR + 0x0000009C)

/* Using gpio as alternate */
#define GPIO_DIRECTION_ALT0 (4)

#define PULL_NONE (0)
#define PULL_DOWN (1)
#define PULL_UP (2)

//Needed GPIO
#define GPIO_02 (2)
#define GPIO_03 (3)

#define START_TRANSFER_SEND (0x00008080)
#define START_TRANSFER_RECIVE (0x00008081)
#define CLEAR_STATUS (0x00000302)
#define SETUP_CTRL_SEND (0x00008110)
#define SETUP_CTRL_RECIVE (0x00008031)

static int init_nunchuk_module(void);
static void cleanup_nunchuk_module(void);

static ssize_t nunchuk_read(struct file *, char *, size_t, loff_t *);
static int nunchuk_open(struct inode *, struct file *);
static int nunchuk_release(struct inode *, struct file *);

#endif //NUNCHUK_H_INCLUDED

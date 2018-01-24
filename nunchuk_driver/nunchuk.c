#include "nunchuk.h"

#define SUCCESS     0
#define RET_ERR     -1
#define DEVICE_NAME "nunchuk" /* Device name as it appears in /proc/devices. */
#define I2C_ADDRESS 0x52      /* I2C Address for comunication */
#define I2C_BUS     1         /* */

#define REG_NO 6
#define JOY_NO 2
#define ACC_NO 3
#define RET_SIZE 10

module_init(init_nunchuk_module);
module_exit(cleanup_nunchuk_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mario Peric, Igor Ilic, Kosta Svrdlan");

/*
 * Global variables are declared as static, so are global within the file.
 */
static int major_number;                  /* Major number assigned to our device driver. */
static int opened;                        /* Is device open */

/*
 * Structure holds pointers to functions defined by the
 * driver that perform various operations on the device.
 */
static struct file_operations fops = {
    .read = nunchuk_read,
    .open = nunchuk_open,
    .release = nunchuk_release,
};

/* Register addresses */
volatile void *reg_c = NULL;
volatile void *reg_dlen = NULL;
volatile void *reg_slave_addr = NULL;
volatile void *reg_fifo = NULL;
volatile void *reg_s = NULL;
volatile void *reg_div = NULL;

unsigned int GetGPFSELReg(char pin){
        unsigned int addr;
     if(pin >= 0 && pin <10)
        addr = GPFSEL0_BASE_ADDR;
    else if(pin >= 10 && pin <20)
        addr = GPFSEL1_BASE_ADDR;
    else if(pin >= 20 && pin <30)
        addr = GPFSEL2_BASE_ADDR;
    else if(pin >= 30 && pin <40)
        addr = GPFSEL3_BASE_ADDR;
    else if(pin >= 40 && pin <50)
        addr = GPFSEL4_BASE_ADDR;
    else /*if(pin >= 50 && pin <53) */
        addr = GPFSEL5_BASE_ADDR;
  
  return addr;
}

char GetGPIOPinOffset(char pin){
    
    if(pin >= 0 && pin <10)
        pin = pin;
    else if(pin >= 10 && pin <20)
        pin -= 10;
    else if(pin >= 20 && pin <30)
        pin -= 20;
    else if(pin >= 30 && pin <40)
        pin -= 30;
    else if(pin >= 40 && pin <50)
        pin -= 40;
    else /*if(pin >= 50 && pin <53) */
        pin -= 50;
    return pin;
}

void SetInternalPullUpDown(char pin, char value){
    
    unsigned int base_addr_gppud; 
    unsigned int base_addr_gppudclk; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of GPIO Pull-up/down Register (GPPUD). */
    base_addr_gppud = GPPUD_BASE_ADDR;
    
    /* Get base address of GPIO Pull-up/down Clock Register (GPPUDCLK). */
    base_addr_gppudclk = (pin < 32) ? GPPUDCLK0_BASE_ADDR : GPPUDCLK1_BASE_ADDR;

    /* Get pin offset in register . */
    pin = (pin < 32) ? pin : pin - 32;
    
    /* Write to GPPUD to set the required control signal (i.e. Pull-up or Pull-Down or neither
       to remove the current Pull-up/down). */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(value, addr);

    /* Wait 150 cycles ^  this provides the required set-up time for the control signal */
    
    /* Write to GPPUDCLK0/1 to clock the control signal into the GPIO pads you wish to
       modify ^  NOTE only the pads which receive a clock will be modified, all others will
       retain their previous state. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp |= mask;        
    iowrite32(tmp, addr);

    /* Wait 150 cycles ^  this provides the required hold time for the control signal */
    
    /* Write to GPPUD to remove the control signal. */
    addr = ioremap(base_addr_gppud, 4);
    iowrite32(PULL_NONE, addr);

    /* Write to GPPUDCLK0/1 to remove the clock. */
    addr = ioremap(base_addr_gppudclk, 4);
    tmp = ioread32(addr);    
    mask = 0x1 << pin;
    tmp &= (~mask);        
    iowrite32(tmp, addr);
}

void SetGpioPinDirection(char pin, char direction){

    unsigned int base_addr; 
    void *addr = NULL;
    unsigned int tmp;
    unsigned int mask;
    
    /* Get base address of function selection register. */
    base_addr = GetGPFSELReg(pin);

    /* Calculate gpio pin offset. */
    pin = GetGPIOPinOffset(pin);    
    
    /* Set gpio pin direction. */
    addr = ioremap(base_addr, 4);
    tmp = ioread32(addr);

    mask = ~(0b111 << (pin*3));
    tmp &= mask;

    mask = (direction & 0b111) << (pin*3);
    tmp |= mask;

    iowrite32(tmp, addr);
}

int send_data(char *buff, int n){

    unsigned int temp;
    short int i;

    /* Clear S reg */
    iowrite32(CLEAR_STATUS, reg_s);

    /* Setup C reg */
    iowrite32(SETUP_CTRL_SEND, reg_c);

    /* Write to FIFO reg */
    for(i = 0; i < n; i++){
        iowrite32((unsigned int)buff[i], reg_fifo);
    }

    /* Setup DLEN reg */
    iowrite32((unsigned int)n, reg_dlen);
    
    /* Starting transfer */
    iowrite32(START_TRANSFER_SEND, reg_c);

    /* Polling */
    do {
        temp = ioread32(reg_s);
    } while(!(temp & (1 << 1))); // While !DONE

    temp = ioread32(reg_s);
    temp &= 1 << 8;
    
    /* If there is error transfer */
    if(temp)
        return RET_ERR;
    else
        return SUCCESS;

}

int receive_data(char *buff, int n){

    unsigned int temp;
    unsigned int temp_d;
    unsigned short i;

    i = 0;

    memset(buff, '\0', n);
    
    /* Clear status register before new transmision */
    iowrite32(CLEAR_STATUS, reg_s);

    /* Ready C reg for read, clear fifo */
    iowrite32(SETUP_CTRL_RECIVE, reg_c);

    /* Set expected data length */
    iowrite32((unsigned int)n, reg_dlen);
    
    /* Start transfer */
    iowrite32(START_TRANSFER_RECIVE, reg_c);

    /* Waiting for DONE = 1 */
    do{
        temp = ioread32(reg_s);
    }while(!(temp & (1 << 1)));

    /* Reading data from fifo while there is some */
    do{
        temp = ioread32(reg_s);
        temp &= 1<<5;
        temp_d = ioread32(reg_fifo);
        buff[i] = temp_d;
        i++;
        if(i == n)
            break;                  
    }while(temp);

    temp = ioread32(reg_s);
    temp &= 1 << 8;
    
    if(temp)
        return RET_ERR;
    else
        return SUCCESS;

}

/*
 * Send handshake signal
 */
static int send_handshake(void)
{
    int status;

    char first_reg[] = {0xF0, 0x55};  // Bytes to send to initialize first register
    char second_reg[] = {0xFB, 0x00}; // Bytes to send to initialize second register

    status = send_data(first_reg, ARRAY_SIZE(first_reg));

    if (status < 0) {
        return RET_ERR;
    }

    mdelay(10);

    status = send_data(second_reg, ARRAY_SIZE(second_reg));

    if (status < 0) {
        return RET_ERR;
    }

    return SUCCESS;
}

/*
 * Read register values into buffer
 */
static int read_registers(char *buff, int buf_size)
{
    char reg_addr = 0x00;

    int status;
    status = send_data(&reg_addr, 1);

    if (status < 0)
    {
        return RET_ERR;
    }

    return receive_data(buff, buf_size);
}

/*
 * This function is called when the module is loaded.
 */
static int init_nunchuk_module(void)
{
    /* Register new chardev */
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
    {
        printk(KERN_ALERT "Registering char device failed with %d\n", major_number);

        return major_number;
    }

    printk(KERN_INFO "'sudo mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major_number);

    /* Allocating memory */
    if (request_mem_region(BSC1_BASE_ADDR, 0x20, DEVICE_NAME) == NULL) {
        printk(KERN_INFO "Data memory not available\n");
        return RET_ERR;
    }

    /* Setting pull up, and pin alt functions */
    SetGpioPinDirection(GPIO_02, GPIO_DIRECTION_ALT0);
    SetGpioPinDirection(GPIO_03, GPIO_DIRECTION_ALT0);
    SetInternalPullUpDown(GPIO_02, PULL_UP);
    SetInternalPullUpDown(GPIO_03, PULL_UP);

    /* Remaping registers virtual addreses to physical */
    reg_c = ioremap(BSC1_REG_C, 4);
    reg_dlen = ioremap(BSC1_REG_DLEN, 4);
    reg_slave_addr = ioremap(BSC1_REG_SLAVE_ADDR, 4);
    reg_fifo = ioremap(BSC1_REG_FIFO, 4);
    reg_s = ioremap(BSC1_REG_S, 4);
    reg_div = ioremap(BSC1_REG_DIV, 4);

    /* Set slave address */
    iowrite32(I2C_ADDRESS, reg_slave_addr);

    /* Set Frequency Divider */
    iowrite32(0x000009C4, reg_div);

    /* Send handshake to nunchuk */
    send_handshake();

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded.
 */
static void cleanup_nunchuk_module(void)
{
    /* Free memory */
    release_mem_region(BSC1_BASE_ADDR, 0x20);

    /* Free major number */
    unregister_chrdev(major_number, DEVICE_NAME);
    SetInternalPullUpDown(GPIO_02, PULL_NONE);
    SetInternalPullUpDown(GPIO_03, PULL_NONE);
}

/*
 * Called when a process tries to open the device file, like "cat /dev/nunchuk".
 */
static int nunchuk_open(struct inode *inode, struct file *file)
{
    if (opened)
    {
        return -EBUSY;
    }

    opened++;

    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int nunchuk_release(struct inode *inode, struct file *file)
{
    opened--;

    module_put(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to read from it.
 */
static ssize_t nunchuk_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int status = 0;
    u8 reg_values[REG_NO];
    u8 joystick[JOY_NO];
    u16 accelerometer[ACC_NO];
    char c_pressed = 0;
    char z_pressed = 0;

    if (read_registers(reg_values, REG_NO) < 0)
    {
        printk(KERN_INFO "Error reading registers: %0x", status);
        return 0;
    }
    /* Get values of C and Z buttons */
    c_pressed = reg_values[5] & 0x02;
    c_pressed >>= 1;
    c_pressed ^= 0x01;

    z_pressed = reg_values[5] & 0x01;
    z_pressed ^= 0x01;

    /* Get values of accelerometers */
    accelerometer[0] = reg_values[2] << 2;
    accelerometer[0] |= (reg_values[5] & 0x0C) >> 2;

    accelerometer[1] = reg_values[3] << 2;
    accelerometer[1] |= (reg_values[5] & 0x30) >> 4;

    accelerometer[2] = reg_values[4] << 2;
    accelerometer[2] |= (reg_values[5] & 0x60) >> 6;

    /* Get values of joysticks */
    joystick[0] = reg_values[0];
    joystick[1] = reg_values[1];

    put_user(joystick[0], buffer + 0);
    put_user(joystick[1], buffer + 1);
    put_user(accelerometer[0], buffer + 2);
    put_user(accelerometer[0]>>8, buffer + 3);
    put_user(accelerometer[1], buffer + 4);
    put_user(accelerometer[1]>>8, buffer + 5);
    put_user(accelerometer[2], buffer + 6);
    put_user(accelerometer[2]>>8, buffer + 7);
    put_user(c_pressed, buffer + 8);
    put_user(z_pressed, buffer + 9);

    return RET_SIZE;
}

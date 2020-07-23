#ifndef _LED_IOCTL_H

#define _LED_IOCTL_H

#include <linux/ioctl.h>

#define GPIO_DRV_MAGIC  'l'
#define GPIO_SELECT_LED _IOW(GPIO_DRV_MAGIC, 1, int)

//TODO 2: Define the IOCTLS
#define GPIO_SET_LED	_IOW(GPIO_DRV_MAGIC, 2, int)
#define GPIO_GET_LED	_IOR(GPIO_DRV_MAGIC, 3, int)

#define GPIO_DRV_MAX_NR 3
#endif /* _LED_IOCTL_H */

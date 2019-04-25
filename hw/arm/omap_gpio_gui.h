#ifndef _OMAP_GPIO_GUI_H
#define _OMAP_GPIO_GUI_H

#include "qemu-common.h"

//#define OMAP_GPIO_GUI_PIPE_NAME "externalpipe"
#define OMAP_GPIO_GUI_PIPE_NAME "omap_gpio_gui_pipe"

int omap_gpio_gui_attach(DeviceState *gpio);

#endif

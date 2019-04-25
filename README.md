To build (for beagle using softmmu) use:

```
./configure --disable-werror --target-list=arm-softmmu
make
```

The base source needs `--disable-werror` for this target...

Status changes in pipe are read and written in this format:

```
struct gpio_op {
	uint8_t pin;
	uint8_t value;
}
```

Named pipe `"omap_gpio_gui_pipe"` receives and sends input status changes for the
**beagle** board.

`"hw/arm/omap_gpio_gui.h"` contains the function to attach gpio to a pipe, it must be called on a `DeviceState*` already containing a `NULL` named I/O gpio.

For example it is called at the end of beagle initialization `beagle_common_init`
on the `cpu->gpio` device.

- Dario Fagotto


Read the documentation in qemu-doc.html or on http://wiki.qemu-project.org

- QEMU team

#include "omap_gpio_gui.h"

#include "qemu-common.h"
#include "sysemu/sysemu.h"
#include "hw/arm/omap.h"
#include "hw/devices.h"
#include "hw/irq.h"
#include "sysemu/char.h"

// #include "qemu-char.h"
// missing, using qemu_chr_fe_write instead of qemu_chr_write

#define CONSUME(buf, size) (--(size), *((buf)++))

struct gpio_opaque {
  DeviceState* gpio;
  CharDriverState* pipe;
  int nOutPins;
  int nInPins;
  int pendingPin;
};

static void omap_gpio_gui_pin_irq(void* opaque, int n, int level) {
  struct gpio_opaque* opq = opaque;
  uint8_t buf[2] = { n, level };
  qemu_chr_fe_write(opq->pipe, buf, sizeof(buf));
}

// IOCanReadHandler
static int omap_gpio_gui_can_read(void* opaque) {
  struct gpio_opaque* opq = opaque;
  return opq->nInPins * 2;
}

// IOReadHandler
static void omap_gpio_gui_read(void* opaque, const uint8_t* buf, int size) {
  struct gpio_opaque* opq = opaque;
  int pin;

  pin = opq->pendingPin;
  while(size >= 0) {
    if(pin >= 0) {
      qemu_set_irq(qdev_get_gpio_in(opq->gpio, pin), CONSUME(buf, size));
      pin = -1;
    } else {
      pin = CONSUME(buf, size);
    }
  }
  opq->pendingPin = pin;
}

int omap_gpio_gui_attach(DeviceState *gpio) {
  /* Get pipe IO for gui */
  CharDriverState* pipe = NULL;
  /*
  {
    ChardevBackend pipe_backend = {
      .kind = CHARDEV_BACKEND_KIND_PIPE,
      .pipe = (ChardevHostdev*)"omap_gpio_gui_pipe"
    };
    ChardevReturn* pipe_ret = qmp_chardev_add(pipe_backend.pipe, &pipe_backend, NULL);
    pipe = pipe_ret->chr;
    qapi_free_ChardevReturn(pipe_ret);
  }
  */
  pipe = qemu_chr_new(OMAP_GPIO_GUI_PIPE_NAME, "pipe:" OMAP_GPIO_GUI_PIPE_NAME, NULL);
  if(pipe == NULL) {
    return -1; // No pipe
  }

  // Get I/O size
  int nIn = 0;
  int nOut = 0;
  {
    NamedGPIOList *ngl;

    QLIST_FOREACH(ngl, &gpio->gpios, node) {
      // Match NULL
      if(ngl->name == NULL) {
        nIn = ngl->num_in;
        nOut = ngl->num_out;
        break;
      }
    }
  }

  // Prepare opaque structure
  struct gpio_opaque* opaque = malloc(sizeof(struct gpio_opaque)); // Should delete on deatach?
  *opaque = (struct gpio_opaque) {
    .gpio = gpio,
    .pipe = pipe,
    .nInPins = nIn,
    .nOutPins = nOut,
    .pendingPin = -1,
  };

  // Add reader to pipe
  qemu_chr_add_handlers(
    opaque->pipe,
    omap_gpio_gui_can_read, // fd_can_read
    omap_gpio_gui_read,     // fd_read
    NULL,                   // fd_event
    opaque                  // opaque
  );

  // Connect irqs to pins to send trough pipe
  qemu_irq* irqs = qemu_allocate_irqs(omap_gpio_gui_pin_irq, opaque, opaque->nOutPins);
  for(int i = 0; i < opaque->nOutPins; ++i) {
    qdev_connect_gpio_out(opaque->gpio, i, *irqs);
    irqs++;
  }

  return 0;
}

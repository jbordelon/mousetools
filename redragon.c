#include <stdlib.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
Jose Bordelon
22 abril 2024
Versión 1.0
Programa para enviar teclas < x al presionar los botones 5 y 6 del mouse redragon
al programa que se encuentre en ejecución en primer plano
se compila con gcc
*/

void send_key(int uinput_fd, int keycode, int press) {
  struct input_event ev;

  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_KEY;
  ev.code = keycode;
  ev.value = press;  // 1 for key press, 0 for key release
  write(uinput_fd, &ev, sizeof(struct input_event));
  // indica al sistema que se ha terminado de enviar eventos y que debe procesarlos todos juntos
  memset(&ev, 0, sizeof(struct input_event));
  ev.type = EV_SYN;
  ev.code = SYN_REPORT;
  ev.value = 0;
  write(uinput_fd, &ev, sizeof(struct input_event));
}

int main(void) {
  struct input_event ev;
  int fd = open("/dev/input/event5", O_RDONLY);
  if (fd == -1) {
    perror("No se pudo abrir el dispositivo");
    return 1;
  }

  int uinput_fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(uinput_fd < 0) {
    perror("No se pudo abrir uinput");
    return 1;
  }

  // Le dice al kernel que el dispositivo de entrada virtual que se está creando puede enviar eventos de tecla
  ioctl(uinput_fd, UI_SET_EVBIT, EV_KEY);
  // Le dice al kernel que el dispositivo de entrada virtual puede enviar las teclas x <
  ioctl(uinput_fd, UI_SET_KEYBIT, KEY_X);      // x
  ioctl(uinput_fd, UI_SET_KEYBIT, KEY_102ND);  // <

  // Configura la descripción de un dispositivo de entrada virtual que se va a crear con uinput
  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor  = 0x1;
  uidev.id.product = 0x1;
  uidev.id.version = 1;

  // Crea el dispositivo de entrada virtual utilizando la interfaz uinput del kernel de Linux
  write(uinput_fd, &uidev, sizeof(uidev));
  ioctl(uinput_fd, UI_DEV_CREATE);

  while (1) {
    read(fd, &ev, sizeof(struct input_event));
    if(ev.type == 1) { // Si es un evento de botón
      if(ev.code == BTN_SIDE ) {  // 275
        send_key(uinput_fd, KEY_X, ev.value);
      } else if(ev.code == BTN_EXTRA ){  // 276
        send_key(uinput_fd, KEY_102ND, ev.value);
      }
    }
  }

  ioctl(uinput_fd, UI_DEV_DESTROY);
  close(uinput_fd);
  close(fd);
  return 0;
}

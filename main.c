#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ui.h"
#include "can.h"
#include "config.h"

struct data {
  struct can *can;
  struct ui *ui;
};

static int ignition = 0;
static int reverse = 0;
uint32_t malicious_canid = CONFIG_MALICIOUS_CANID;
static struct data args;

static void ignition_cb(struct ui *ui)
{
  unsigned char ign_msg[] = "f00f00";

  if(ignition) {
	  can_send(args.can, 0x123, ign_msg, 6);
  } else {
	  can_send(args.can, 0x321, ign_msg, 6);
  }
  ignition = !ignition;
  
  return;
}

static void reverse_cb(struct ui *ui)
{
  unsigned char rev_msg[] = "00f000f0";

  if(reverse) {
	  can_send(args.can, 0x234, rev_msg, 8);
  } else {
	  can_send(args.can, 0x432, rev_msg, 8);
  }
  reverse = !reverse;
  
  return;
}

static int update(struct data *data, gint fd, GdkInputCondition condition)
{
  uint32_t can_id;
  int can_len;
  char buffer[32];
  int ret;
  time_t now;

  now = time(NULL);
  ret = can_poll(data->can, &can_id, &can_len, buffer, sizeof(buffer));

  if(can_id == malicious_canid) {
	  ui_set_status(data->ui, "HACKED", CONFIG_COLOR_HACKED);
  }
  
  if(ret > 0) {
    ui_append_frame(data->ui, can_id, can_len, buffer, now);
  }

  return(1);
}

int main(int argc, char *argv[])
{
  int err;
  FILE *fd;

  args.can = NULL;
  args.ui = NULL;
  
  if((err = ui_init(argc, argv, &(args.ui))) < 0) {
    fprintf(stderr, "ui_init: %s\n", strerror(-err));
    return(-err);
  }

  if((err = can_init(&(args.can))) < 0) {
    fprintf(stderr, "can_init: %s\n", strerror(-err));
    return(-err);
  }

  if((fd = fopen(CONFIG_CFG_PATH, "r"))) {
	  err = fread(&malicious_canid, sizeof(uint32_t), 1, fd);

	  if(err < 1) {
		  err = errno;
		  perror("fread");
	  }
	  fclose(fd);
	  fd = NULL;
  }

  ui_add_input(args.ui, *((int*)(args.can)), (GdkInputFunction)update, &args);
  ui_set_status(args.ui, "Secure", CONFIG_COLOR_SECURE);
  ui_set_canid(args.ui, malicious_canid);
  ui_set_callback(args.ui, UI_EVENT_IGNITION, ignition_cb);
  ui_set_callback(args.ui, UI_EVENT_REVERSE, reverse_cb);
  ui_draw(args.ui);
  
  return(err);
}

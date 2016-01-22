#ifndef _CANDY_UI_H
#define _CANDY_UI_H

#include <stdint.h>
#include <time.h>
#include <gtk/gtk.h>

struct ui;

typedef void (ui_callback_t)(struct ui*);

enum ui_event {
  UI_EVENT_IGNITION = 0,
  UI_EVENT_REVERSE,
  UI_EVENT_MAX
};

typedef enum ui_event ui_event_t;

int  ui_init(int, char**, struct ui**);
int  ui_add_idle(struct ui*, void(*)(void*), void*);
int  ui_add_input(struct ui*, int, GdkInputFunction, void*);
int  ui_append_frame(struct ui*, uint32_t, int, char*, time_t);
void ui_set_canid(struct ui*, uint32_t);
void ui_set_status(struct ui*, const char*, unsigned int);
void ui_draw(struct ui*);

void ui_set_callback(struct ui*, ui_event_t, ui_callback_t*);

#endif /* _CANDY_UI_H */

#ifndef _CANDY_CAN_H
#define _CANDY_CAN_H

struct can;

int can_init(struct can**);
int can_poll(struct can*, uint32_t*, int*, char*, size_t);
int can_send(struct can*, uint32_t, unsigned char*, size_t);

#endif /* _CANDY_CAN_H */

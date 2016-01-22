#ifndef __CONFIG_H
#define __CONFIG_H

#define CONFIG_CAN_NONBLOCK 0
#define CONFIG_CAN_IFACES "can0","vcan0"

#define CONFIG_MALICIOUS_CANID 0x123
#define CONFIG_CFG_PATH "./candy.conf"
#define CONFIG_COLOR_SECURE 0x11b500
#define CONFIG_COLOR_HACKED 0xea1e1e

#endif /* __CONFIG_H */

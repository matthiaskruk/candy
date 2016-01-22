#include <sys/socket.h>
#include <linux/can.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include "can.h"
#include "config.h"

static const char *iface[] = {
	CONFIG_CAN_IFACES,
	NULL
};

struct can {
	int fd;
	struct sockaddr_can addr;
};

int can_init(struct can **c)
{
	struct can *cc;
	int err;
#if defined(CONFIG_CAN_NONBLOCK) && CONFIG_CAN_NONBLOCK == 1
	int flag = 1;
#endif
	
	if(!(cc = malloc(sizeof(*cc)))) {
		return(-ENOMEM);
	}

	memset(cc, 0, sizeof(*cc));

	err = -EINVAL;
	cc->fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);

	if(cc->fd < 0) {
		err = -errno;
		perror("socket");
		return(err);
	}

	cc->addr.can_family = AF_CAN;
	if(bind(cc->fd, (struct sockaddr*)&(cc->addr), sizeof(cc->addr)) < 0) {
		err = -errno;
		perror("bind");
		close(cc->fd);
		free(cc);
		return(err);
	}

#if defined(CONFIG_CAN_NONBLOCK) && CONFIG_CAN_NONBLOCK == 1
	if(fcntl(cc->fd, F_SETFL, O_NONBLOCK, &flag, sizeof(flag)) < 0) {
		perror("fcntl");
		err = 0;
	}
#endif
	
	*c = cc;
	return(0);
}

int can_poll(struct can *c, uint32_t *cid, int *clen, char *buffer, size_t size)
{
	struct can_frame frm;
	int ret, nbytes;

	nbytes = read(c->fd, &frm, sizeof(frm));

	if(nbytes < 0) {
		ret = -errno;
		if(errno != EAGAIN && errno != EWOULDBLOCK) {
			perror("read");
		}
	} else {
		int i;

		if(nbytes < sizeof(frm)) {
			fprintf(stderr, "can_poll: Partial frame\n");
			return(-EBADMSG);
		}

		nbytes = sizeof(frm.data);

		if(size < nbytes) {
			nbytes = size;
		}
    
		memcpy(buffer, &(frm.data), nbytes);
    
		printf("CAN:");
		for(i = 0; i < sizeof(frm); i++) {
			printf(" %02x", ((unsigned char*)&frm)[i]);
		}
		printf("\n");
		ret = nbytes;
		*cid = frm.can_id;
		*clen = frm.can_dlc;
	}
  
	return(ret);
}

int can_send(struct can *c, uint32_t cid, unsigned char *buf, size_t len)
{
	struct can_frame frm;
	struct sockaddr_can addr;
	int e;
	int ifidx;
	struct ifreq ifr;
  
	if(!c || !buf || len > sizeof(frm.data)) {
		return(-EINVAL);
	}

	memset(&frm, 0, sizeof(frm));
	memset(&addr, 0, sizeof(addr));
	
	frm.can_id = cid;
	frm.can_dlc = len;
	memcpy(&(frm.data), buf, len);
	e = -ENODEV;

	for(ifidx = 0; iface[ifidx]; ifidx++) {
		snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), iface[ifidx]);
		if(ioctl(c->fd, SIOCGIFINDEX, &ifr) < 0) {
			if(errno == ENODEV) {
				continue;
			}

			e = errno;
			perror("ioctl");
			break;
		}
		addr.can_ifindex = ifr.ifr_ifindex;
		addr.can_family = AF_CAN;

		if((e = sendto(c->fd, &frm, sizeof(frm), 0, (struct sockaddr*)&addr, sizeof(addr))) < 0) {
			e = -errno;
			perror("sendto");
		}
	}

	return(e);
}

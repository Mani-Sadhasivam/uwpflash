#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "interface.h"
#include "command.h"

struct packet_header
{
	short type;
	short size;
};

#define le2be16(num) \
	(num >> 8) | (num << 8)

int send_command(int cmd, int timeout)
{
	struct packet_header pkt_hdr;

	pkt_hdr.type = le2be16(cmd);
	pkt_hdr.size = 0;

	return intf_send((unsigned char *)&pkt_hdr,
			sizeof(pkt_hdr), 1, timeout);
}

int cmd_connect(int timeout)
{
	return send_command(BSL_CMD_CONNECT, timeout);
}

int cmd_check_bandrate(int timeout) {
	unsigned char check_val = 0x7E;

	return intf_send(&check_val, sizeof(check_val),
			0, timeout);
}

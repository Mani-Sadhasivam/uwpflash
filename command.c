#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "interface.h"
#include "command.h"

int send_command(int cmd, int timeout)
{
	struct packet_header pkt_hdr;

	pkt_hdr.type = le16(cmd);
	pkt_hdr.size = 0;

	return intf_send((unsigned char *)&pkt_hdr,
			sizeof(pkt_hdr), 1, timeout);
}

int send_data(int type, unsigned char *data,
		int len, int timeout)
{
	int i;
	int ret;
	struct packet_header *pkt_hdr;
	unsigned char *buf;
	int buf_len = len + sizeof(struct packet_header);
	unsigned int *src, *dest;

	buf = malloc(buf_len);
	if (buf == NULL) {
		printf("out of memory.\n");
		return -1;
	}

	pkt_hdr = (struct packet_header *)buf;
	pkt_hdr->type = le16(type);
	pkt_hdr->size = le16(len);

	src = (unsigned int *)data;
	dest = (unsigned int *)(buf + sizeof(struct packet_header));
#if 0

	for (i = 0; i < (len / 4); i++) {
		*dest++ = (unsigned int)le32(*src++);
	}
#endif
	memcpy((void *)dest, (void *)src, len); 

	ret = intf_send(buf, buf_len, 1, timeout);
	if (ret) {
		free(buf);
		return -1;
	}

	free(buf);
	return 0;
}

struct packet_start
{
	unsigned int addr;
	unsigned int len;
};

int cmd_send_start(unsigned int addr, unsigned int len)
{
	struct packet_start pkt_start;

	pkt_start.addr = le32(addr);
	pkt_start.len = le32(len);

	return send_data(BSL_CMD_START_DATA, (unsigned char *)&pkt_start,
			sizeof(pkt_start), 1000);
}

int cmd_send_data(unsigned char * addr, unsigned int len)
{
	return send_data(BSL_CMD_MIDST_DATA, addr, len, 1000);
}

int cmd_connect(int timeout)
{
	return send_command(BSL_CMD_CONNECT, timeout);
}

int cmd_stop(int timeout)
{
	return send_command(BSL_CMD_END_DATA, timeout);
}

int cmd_exec(int timeout)
{
	return send_command(BSL_CMD_EXEC_DATA, timeout);
}


int cmd_check_bandrate(int timeout) {
	unsigned char check_val = 0x7E;

	return intf_send(&check_val, sizeof(check_val),
			0, timeout);
}

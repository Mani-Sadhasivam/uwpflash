#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#include "interface.h"
#include "command.h"
#include "crc.h"

static sem_t sem_recv;
static struct interface *intf;
extern struct interface uart_intf;

int intf_recv(unsigned char *data, unsigned int len, int timeout)
{
	int ret, i;
	int recv_len = 0;
	char *crc_data;
	int crc_len;
	struct timespec wait_time;
	int data_start = -1;
	int data_end = -1;

	wait_time.tv_sec = timeout;
	wait_time.tv_nsec = 0;

	ret = sem_timedwait(&sem_recv, &wait_time);
	if (ret == ETIMEDOUT) {
		return -2;	/* Timeout */
	} else if (ret) {
		printf("sem_wait failed.\n");
		return -1;
	}

	if (intf->recv == NULL) {
		printf("recv is null.\n");
		return -1;
	}

	recv_len = intf->recv(data, len);
	if (recv_len <= 0)
		return recv_len;

	for (i = 0; i < recv_len; i++) {
		if (data[i] == FLAG_BYTE) {
			if (data_start == -1)
				data_start = i;
			else
				data_end = i;
		}
	}

	if ((data_start != -1) && (data_end != -1)) {
		ret = decode_msg(data + data_start, data_end - data_start + 1,
				&crc_data, &crc_len, 1);
		printf("decode return: %d.\n", ret);
		if (ret != 1) {
			printf("decode error: %d.\n", ret);
			ret= -1;
			goto err_out;
		}

		for(i = 0; i < crc_len; i++) {
			printf("%02x ", crc_data[i]);
		}
		printf("\n");
	}

	memcpy(data, crc_data, crc_len);
	ret = crc_len;

err_out:
	free(crc_data);

	return ret;
}

int intf_send(unsigned char *data, unsigned int len,
		int need_crc, int timeout)
{
	int ret;
	char *crc_data;
	int crc_len;

	if (need_crc) {
	if (!encode_msg(data, len, &crc_data, &crc_len, 1)) {
		printf("crc failed.\n");
		return -1;
	}
	} else {
		crc_data = data;
		crc_len = len;
	}

	if (intf->send == NULL) {
		printf("send is null.\n");
		ret = -1;
		goto err_out;
	}

	if (intf->send(crc_data, crc_len)) {
		ret = -1;
		goto err_out;
	}

	ret = 0;

err_out:
	if (need_crc)
		free(crc_data);

	return ret;
}

void notify_recv(void)
{
	//printf("data recv.\n");
	sem_post(&sem_recv);
}

int intf_init(char *type, char *dev)
{
	if (strcmp(type, "UART") == 0) {
		intf = &uart_intf;
	} else {
		printf("unkown interface type %s.\n", type);
		return -1;
	}

	sem_init(&sem_recv, 0, 0);

	intf->register_notify(notify_recv);

	return intf->init(dev);
}


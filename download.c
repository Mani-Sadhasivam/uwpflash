#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>

#include "command.h"

#define MAX_DATA_LEN 528
enum DL_STATE {
	DL_INIT,
	DL_CHECK_BAND,
	DL_CONNECT,
	DL_SEND_HEADER,
	DL_SEND_BIN,
	DL_FINISH,
	DL_EXECUTE,
	DL_SUCCESS,
	DL_TIMEOUT,
	DL_TERMINATE,
};

struct dl_file
{
	int fd;
	int is_fdl;
	unsigned char *fdl_buf;
	unsigned int addr;
	unsigned int pos;
	unsigned int len;
	unsigned int print_percent;
};

static unsigned char *data_buf;
static struct dl_file dl_file;
static enum DL_STATE dl_state;
static sem_t dl_sem;
static sem_t flash_sem;
static int dl_timeout;

pthread_t tid_flash;
pthread_t tid_recv;

#define RECV_BUF_LEN	1024

void dl_set_state(enum DL_STATE state, int timeout) {
	dl_state = state;
	dl_timeout = timeout;

	//printf("state: %d, timeout: %d.\n", state, timeout);
	sem_post(&dl_sem);
}

void *dl_recv_thread(void *arg)
{
	int len;
	char buf[RECV_BUF_LEN];
	struct packet_header *pkt_hdr;
	
	while (1) {
		len = RECV_BUF_LEN;
		len = intf_recv(buf, len, 5);
		if (len > 0) {
			pkt_hdr = (struct packet_header *)buf;
			pkt_hdr->type = le16(pkt_hdr->type);
			pkt_hdr->size = le16(pkt_hdr->size);

			//printf("pkt type: 0x%02x.\n", pkt_hdr->type);
			//printf("pkt size: %d.\n", pkt_hdr->size);

			if ((pkt_hdr->type == BSL_REP_VER) ||
				(pkt_hdr->type == BSL_REP_ACK)){
				switch (dl_state) {
				case DL_CHECK_BAND:
				case DL_CONNECT:
				case DL_SEND_HEADER:
				case DL_FINISH:
				case DL_EXECUTE:
					printf("Done\n");
					fflush(stdout);
					dl_set_state(dl_state + 1, 5);
					break;
				case DL_SEND_BIN:
					dl_set_state(DL_SEND_BIN, 5);
					break;
				default:
					break;
				}
			}
		}
	}
}

#define MAX_PERCENT	100

int dl_send_data(void)
{
	int i;
	struct dl_file *pfile = &dl_file;
	unsigned int data_len;
	unsigned int dl_percent;

	if (pfile->pos == 0) {
		for (i = 0; i < MAX_PERCENT; i++) {
			printf("-");
		}
		fflush(stdout);
		for (i = 0; i < MAX_PERCENT; i++) {
			printf("\b");
		}
	}

	data_len = pfile->len - pfile->pos;

	if (data_len > MAX_DATA_LEN)
		data_len = MAX_DATA_LEN;

	if (pfile->is_fdl) {
		memcpy(data_buf, pfile->fdl_buf + pfile->pos, data_len);
	} else {
		lseek(pfile->fd, pfile->pos, SEEK_SET);

		data_len = read(pfile->fd, data_buf, data_len);
		//printf("pos: %d, len: %d.\n", pfile->pos, data_len);
	}

	pfile->pos += data_len;

	dl_percent = pfile->pos * 100 / pfile->len;
	for (i = pfile->print_percent; i < dl_percent; i++) {
		printf(">");
		fflush(stdout);
	}
	pfile->print_percent = dl_percent;

	cmd_send_data(data_buf, data_len);

	if (pfile->pos == pfile->len) {
		dl_set_state(DL_FINISH, 5);
		return -1;
	}

	return 0;
}

void *dl_flash_thread(void *arg)
{
	int ret;
	int is_timeout;
	struct timespec wait_time;

	while (1) {
		is_timeout = 0;
		clock_gettime(CLOCK_REALTIME, &wait_time);
		wait_time.tv_sec += dl_timeout;
		ret = sem_timedwait(&dl_sem, &wait_time);
		if ((ret == -1) && (errno == ETIMEDOUT)) {
			is_timeout = 1;
			//printf("state %d timeout.\n", dl_state);
		}

		switch (dl_state) {
		case DL_INIT:
			break;
		case DL_CHECK_BAND:
//			if (is_timeout)
//				dl_set_state(DL_CONNECT, 5);
			//sleep(1);
			cmd_check_bandrate(1000);
			break;
		case DL_CONNECT:
			if (is_timeout)
				dl_set_state(DL_TIMEOUT, 5);

			printf("* CONNECTING...\t\t\t");
			fflush(stdout);
			cmd_connect(1000);
			break;
		case DL_SEND_HEADER:
			if (is_timeout)
				dl_set_state(DL_TIMEOUT, 5);

			printf("* START...\t\t\t");
			fflush(stdout);
			cmd_send_start(dl_file.addr, dl_file.len);
			break;
		case DL_SEND_BIN:
			if (is_timeout)
				dl_set_state(DL_TIMEOUT, 5);
			dl_send_data();
			break;
		case DL_FINISH:
			if (is_timeout)
				dl_set_state(DL_TIMEOUT, 5);
			printf("\n* STOP...\t\t\t");
			fflush(stdout);
			cmd_stop();
			break;
		case DL_EXECUTE:
			if (is_timeout)
				dl_set_state(DL_TIMEOUT, 5);
			printf("* FLASH...\t\t\t");
			fflush(stdout);
			cmd_exec();
			break;
		case DL_SUCCESS:
			printf("* SUCCESS\n\n");
			sem_post(&flash_sem);
			break;
		case DL_TIMEOUT:
			sem_post(&flash_sem);
			break;
		default:
			break;
		}

	}
}

int dl_wait(void)
{
	int ret;
	struct timespec wait_time;

	clock_gettime(CLOCK_REALTIME, &wait_time);
	wait_time.tv_sec += 60;
	ret = sem_timedwait(&flash_sem, &wait_time);
	if ((ret == -1) && (errno == ETIMEDOUT)) {
		printf("TIMEOUT\n");
		return -1;
	}

	if (dl_state != DL_SUCCESS)
		return -1;

	return 0;
}

int dl_flash_fdl(unsigned char *fdl, unsigned int len, unsigned int addr)
{
	struct dl_file *pfile = &dl_file;

	memset((void*)pfile, 0, sizeof(struct dl_file));
	pfile->is_fdl = 1;
	pfile->addr = addr;
	pfile->len = len;
	pfile->fdl_buf = fdl;

	printf("download FDL (%d bytes)\n", pfile->len);
	printf("* CHECKING BANDRATE...  \t");
	fflush(stdout);
	dl_set_state(DL_CHECK_BAND, 1);

	return dl_wait();
}

int dl_flash(char *fname, unsigned int addr)
{
	int ret;
	struct dl_file *pfile = &dl_file;
	memset((void*)pfile, 0, sizeof(struct dl_file));

	pfile->addr = addr;
	pfile->fd = open(fname, O_RDONLY);
	if (pfile->fd < 0) {
		perror(fname);
		return -1;
	}

	if ((pfile->len = lseek(pfile->fd, 0, SEEK_END)) < 0)
	{
		perror("lseek file failure!");
		return -1;
	}
	lseek(pfile->fd, 0, SEEK_SET);

	printf("download file %s (%d bytes)\n", fname, pfile->len);

	dl_set_state(DL_CONNECT, 5);
	
	return dl_wait();
}

int dl_init(void)
{
	int ret;

	dl_state = DL_INIT;
	sem_init(&dl_sem, 0, 0);
	sem_init(&flash_sem, 0, 0);
	dl_timeout = 5;

	data_buf = malloc(MAX_DATA_LEN);
	if (data_buf == NULL) {
		printf("out of memory.\n");
		return -1;
	}

	ret = pthread_create(&tid_recv, NULL, dl_recv_thread, NULL);
	if (ret) {
		perror("create pthread failed");
		return ret;
	}

	ret = pthread_create(&tid_flash, NULL, dl_flash_thread, NULL);
	if (ret) {
		perror("create pthread failed");
		return ret;
	}

	return 0;
}

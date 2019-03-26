#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "command.h"
#include "interface.h"
#define VERSION	"0.0.1"

struct params {
	char *dev;
	char *file;
	char *intf_type;
	unsigned int addr;
};

static struct params params = {
	.intf_type = "UART",
	.dev = "/dev/ttyUSB0",
	.file = "zephyr.bin",
	.addr = 0x2000000,
};

int main(int argc,char **argv)
{
	struct params *p = &params;
	int opt;
	int ret;
	opterr = -1;


	while ((opt = getopt (argc, argv, "t:d:vf:a:")) != -1) {
		switch (opt) {
			case 't':
				p->intf_type = optarg;
				break;
			case 'v':
				printf("UNISOC uwpflash tool v" VERSION "\n");
				break;
			case 'f':
				p->file = optarg;
				break;
			case 'a':
				sscanf(optarg,"0x%x", &p->addr);
				printf("download address: 0x%x.\n", p->addr);
				break;
			default:
				perror("Invalid parameter.");
				return -1;
		}
	}

	ret = intf_init(p->intf_type, p->dev);
	if (ret < 0) {
		perror("init interface failed");
		return ret;
	}

	int len = 1024;
	char buf[1024];
	int i = 10;
	while (i--) {
		len = 1024;
		cmd_check_bandrate(1000);
		len = intf_recv(buf, len, 1);
		printf("main recv %d bytes.\n", len);
		sleep(1);
	}

	sleep(1);
	
	return 0;
}

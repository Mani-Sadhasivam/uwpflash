#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "command.h"
#include "interface.h"
#include "fdl.h"

#define VERSION	"0.0.1"
#define AUTHOR "Dong Xiang <dong.xiang@unisoc.com>"

struct fobject {
	char *fname;
	unsigned int addr;
	struct fobject *next;
};

struct fobject * fobj_alloc(void)
{
	struct fobject *fobj;

	fobj = (struct fobject *)malloc(sizeof(*fobj));
	if (fobj == NULL) {
		printf("out of memory.\n");
		return NULL;
	}

	memset((void *)fobj, 0, sizeof(*fobj));
	fobj->next = NULL;

	return fobj;
}

int fobj_insert(struct fobject **flist, struct fobject *fobj)
{
	fobj->next = (*flist);
	(*flist) = fobj;

	return 0;
}

struct params {
	char *dev;
	char *intf_type;
	struct fobject *flist;
};

static struct params params = {
	.intf_type = "UART",
	.dev = "/dev/ttyUSB0",
	.flist = NULL,
};

static struct fobject *fobj;

void help(void) {
	printf("-h\t\t show this message.\n");
	printf("-t <type>\t interface type.\n");
	printf("\t\t default: UART\n");
	printf("-d <dev> \t device name.\n");
	printf("\t\t default: /dev/ttyUSB0\n");
	printf("-f <file>\t image file name.\n");
	printf("-a <address>\t flash address.\n");

	exit(0);
}

int main(int argc,char **argv)
{
	struct params *p = &params;
	int opt;
	int ret;
	opterr = -1;

	printf("UNISOC uwpflash tool.\n");
	printf("Version:\t" VERSION "\n");
	printf("Author:\t\t" AUTHOR "\n\n");

	while ((opt = getopt (argc, argv, "t:d:vf:a:h")) != -1) {
		switch (opt) {
			case 'h':
				help();
				break;
			case 't':
				p->intf_type = optarg;
				break;
			case 'v':
				printf("UNISOC uwpflash tool v" VERSION "\n");
				break;
			case 'd':
				p->dev = optarg;
				break;
			case 'f':
				if (fobj == NULL) {
					fobj = fobj_alloc();
					if (fobj == NULL) {
						return -1;
					}
				}
				fobj->fname = optarg;
				break;
			case 'a':
				if (fobj == NULL)
					continue;
				sscanf(optarg,"0x%x", &fobj->addr);
				fobj_insert(&(p->flist), fobj);
				fobj = NULL;
				break;
			default:
				help();
		}
	}

	printf("Interface: %s\n", p->intf_type);
	printf("Device: %s\n", p->dev);
	printf("\n");

	if (p->flist == NULL) {
		printf("None file(s) need to flash.\n\n");
		help();
	}

	ret = intf_init(p->intf_type, p->dev);
	if (ret < 0) {
		perror("init interface failed");
		return ret;
	}

	ret = dl_init();
	if (ret < 0) {
		printf("download init failed.\n");
		return ret;
	}
	

	ret = dl_flash_fdl(fdl, sizeof(fdl), 0x100000);
	if (ret < 0) {
		printf("download file failed.\n");
		return ret;
	}

	for (fobj = p->flist; fobj != NULL; fobj = fobj->next) {
		ret = dl_flash(fobj->fname, fobj->next);
		if (ret < 0) {
			printf("download file %s failed.\n", fobj->fname);
			return ret;
		}
	}

	return 0;
}

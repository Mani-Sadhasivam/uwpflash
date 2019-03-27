#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

#include "interface.h"

int tty_fd = -1;
notify_func notify = NULL;

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
					   B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, 
					  115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };

void set_speed(int fd, int speed){
	int   i; 
	int   status; 
	struct termios   Opt;
	tcgetattr(fd, &Opt); 
	for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++) { 
		if  (speed == name_arr[i]) {     
			tcflush(fd, TCIOFLUSH);     
			cfsetispeed(&Opt, speed_arr[i]);  
			cfsetospeed(&Opt, speed_arr[i]);   
			status = tcsetattr(fd, TCSANOW, &Opt);  
			if  (status != 0) {        
				perror("tcsetattr fd1");  
				return;     
			}    
			tcflush(fd,TCIOFLUSH);   
		}  
	}
}

int set_parity(int fd,int databits,int stopbits,int parity)
{ 
	struct termios options; 
	if  ( tcgetattr( fd,&options)  !=  0) { 
		perror("SetupSerial 1");     
		return -1;
	}
	options.c_cflag &= ~CSIZE; 
	switch (databits) /*设置数据位数*/
	{   
		case 7:		
			options.c_cflag |= CS7; 
			break;
		case 8:     
			options.c_cflag |= CS8;
			break;   
		default:    
			perror("Unsupported data size");
			return -1;
	}
	switch (parity) 
	{   
		case 'n':
		case 'N':    
			options.c_cflag &= ~PARENB;   /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */ 
			break;  
		case 'o':   
		case 'O':     
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/  
			options.c_iflag |= INPCK;             /* Disnable parity checking */ 
			break;  
		case 'e':  
		case 'E':   
			options.c_cflag |= PARENB;     /* Enable parity */    
			options.c_cflag &= ~PARODD;   /* 转换为偶效验*/     
			options.c_iflag |= INPCK;       /* Disnable parity checking */
			break;
		case 'S': 
		case 's':  /*as no parity*/   
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;  
		default:   
			perror("Unsupported parity");    
			return -1;
	}  
	/* 设置停止位*/  
	switch (stopbits)
	{   
		case 1:    
			options.c_cflag &= ~CSTOPB;  
			break;  
		case 2:    
			options.c_cflag |= CSTOPB;  
			break;
		default:    
			perror("Unsupported stop bits");  
			return -1;
	} 
	/* Set input parity option */ 
	if (parity != 'n')   
		options.c_iflag |= INPCK; 
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/   
	options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
	if (tcsetattr(fd,TCSANOW,&options) != 0)   
	{ 
		perror("SetupSerial 3");   
		return -1;
	} 
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/

	return 0;
}

int uart_recv(char *buf, int len);
void *uart_thread(void *arg)
{
	int ret;
	fd_set fs_read;
	struct timeval tv_timeout;

	int recv_len = 1024;
	char recv_buf[recv_len];

	//printf("tty_fd: %d.\n", tty_fd);

	while(1)
	{
		FD_ZERO(&fs_read);
		FD_SET(tty_fd, &fs_read);
		tv_timeout.tv_sec = 10; //time out : unit sec
		tv_timeout.tv_usec = 0;

		ret = select(tty_fd + 1, &fs_read, NULL,
				NULL, &tv_timeout);

		if (ret == -1) {
			perror("select()");
			continue;
		} else if(ret &&  FD_ISSET(tty_fd, &fs_read)) {
			if (notify) notify();
		} else {
			//printf("select timeout");
		}
	}

}

int uart_init(char *dev)
{
	int fd, ret;
	pthread_t tid;

	fd = open(dev, O_RDWR|O_NOCTTY);
	if (fd < 0) {
		perror(dev);
		return fd;
	}

	if (flock(fd, LOCK_EX|LOCK_NB)) {
		perror("device opened by other process");
		return -1;
	}

	set_speed(fd, 115200);
	if (set_parity(fd, 8, 1, 'N')) {
		perror("set parity error.");
		return -1;
	}

	tty_fd = fd;
	uart_clear();

	ret = pthread_create(&tid, NULL, uart_thread, NULL);
	if (ret) {
		perror("create pthread failed");
		return ret;
	}

	return 0;
}
int uart_send(char *data, int len)
{
	int send;
#if 0
	int i;
	printf("uart send %d bytes.\n", len);
	for(i = 0; i < len; i++) {
		printf("%02x ", (unsigned char)data[i]);
	}
	printf("\n");
#endif
	send = write(tty_fd, data, len);
	if (send == len)
		return 0;

	return -1;
}

int uart_recv(char *buf, int len)
{
	int i;
	int readlen = 0;

	readlen = read(tty_fd, buf, len);

#if 0
	printf("uart recv %d bytes.\n", readlen);
	if (readlen > 0) {
		for(i = 0; i < readlen; i++) {
			printf("%02x ", (unsigned char)buf[i]);
		}
		printf("\n");
	}
#endif
	return readlen;
}

int uart_clear(void) {
	tcflush(tty_fd, TCOFLUSH);
	return 0;
}

int uart_drain(void) {
	tcdrain(tty_fd);
	return 0;
}

int uart_register_notify(notify_func func)
{
	notify = func;
	return 0;
}

struct interface uart_intf = {
	.name = "UART",
	.init = uart_init,
	.send = uart_send,
	.recv = uart_recv,
	.register_notify = uart_register_notify,
};


typedef void (*notify_func)(void);
struct interface {
	char *name;
	int (*init)(char *dev);
	int (*send)(char *data, int len);
	int (*recv)(char *data, int len);
	int (*register_notify)(notify_func func);
};

extern int intf_init(char *type, char *dev);
extern int intf_send(unsigned char *data, unsigned int len,
		int need_crc, int timeout);
extern int intf_recv(unsigned char *data, unsigned int len,
		int timeout);

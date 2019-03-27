# uwpflash
UNISOC flash tool.

# usage

-h               show this message.
-t <type>        interface type.
				 default: UART
-d <dev>         device name.
				 default: /dev/ttyUSB0
-f <file>        image file name.
-a <address>     flash address.

# example

uwpflash -t UART -d /dev/ttyUSB0 -f kernel.bin -a 0x2000000

obj-m += chario-module.o
chario-module-objs += chario.o nvme-core.o nvme-scsi.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

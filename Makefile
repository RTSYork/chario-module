obj-m += charfs-module.o
charfs-module-objs += charfs.o nvme-core.o nvme-scsi.o

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean

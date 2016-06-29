#ifndef CHARFS_H
#define CHARFS_H

#include <linux/ioctl.h>

struct charfs_phys_io {
    __u64 address;
    size_t length;
};

#define CHARFS_IOCTL_READ_PHYS  _IOR('N', 0x50, struct charfs_phys_io *)
#define CHARFS_IOCTL_WRITE_PHYS _IOW('N', 0x51, struct charfs_phys_io *)

#endif //CHARFS_H

#ifndef CHARIO_H
#define CHARIO_H

#include <linux/ioctl.h>

struct chario_phys_io {
    __u64 address;
    size_t length;
};

#define CHARIO_IOCTL_READ_PHYS  _IOR('N', 0x50, struct chario_phys_io *)
#define CHARIO_IOCTL_WRITE_PHYS _IOW('N', 0x51, struct chario_phys_io *)

#endif //CHARIO_H

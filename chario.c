/**
 * Based on http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
 */

//#define DEBUG 1

#include "linux_includes.h"

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/kobject.h>        // Using kobjects for the sysfs bindings
#include <asm/uaccess.h>          // Required for the copy to user function

#include "nvme-core.h"
#include "chario.h"

#define MAX_BYTES 4194304 // 4MiB - Maximum transfer size supported in single user NVMe request

#define  DEVICE_NAME "chardisk0"     ///< The device will appear at /dev/chardisk0 using this value
#define  CLASS_NAME  "chario"     ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Russell Joyce");   ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("CharIO test"); ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  charioClass  = NULL;  ///< The device-driver class struct pointer
static struct device* charioDevice = NULL;  ///< The device-driver device struct pointer
static char   attrsName[] = "attrs";

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static loff_t  dev_llseek(struct file *, loff_t, int);
static long    dev_ioctl(struct file *, unsigned int, unsigned long);

static ssize_t test_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t test_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
	.llseek = dev_llseek,
	.unlocked_ioctl = dev_ioctl
};

static struct kobj_attribute test_attr = __ATTR(test, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, test_show, test_store);

static struct attribute *chario_attrs[] = {
	&test_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.name  = attrsName,
	.attrs = chario_attrs,
};

static struct kobject *chario_kobj;

/*
static int rq_test(void) {

	struct nvme_dev *dev = chario_nvme_get_current_dev();
	struct nvme_ns *ns = list_first_entry(&dev->namespaces, struct nvme_ns, list);
	struct nvme_queue *queue = dev->queues[1];

	struct request_queue rq_queue = {
		.queuedata = &ns
	};

	struct blk_mq_hw_ctx hctx = {
		.queue = &rq_queue,
		.driver_data = queue
	};

	struct request *req = kmalloc(sizeof(struct request) + sizeof(struct nvme_cmd_info), GFP_KERNEL);

//	if (req == NULL) {
//		pr_err("CharIO: No memory to allocate request");
//		return 1;
//	}

	struct nvme_cmd_info *cmd = blk_mq_rq_to_pdu(req);





	const struct blk_mq_queue_data bd = {
		.rq = req,
		.last = true,
		.list = NULL
	};


	return chario_nvme_queue_rq(&hctx, &bd);
}
*/
/*
static int submit_cmd_test(void) {
	struct nvme_dev *dev = chario_nvme_get_current_dev();
	struct nvme_queue *queue = dev->queues[0];

	struct nvme_rw_command rw_command = {
		.opcode = NULL,
		.flags = NULL,
		.command_id = NULL,
		.nsid = NULL,
		.rsvd2 = NULL,
		.metadata = NULL,
		.prp1 = NULL,
		.prp2 = NULL,
		.slba = NULL,
		.length = NULL,
		.control = NULL,
		.dsmgmt = NULL,
		.reftag = NULL,
		.apptag = NULL,
		.appmask = NULL
	};

	struct nvme_command cmd = {
			.rw = rw_command
	};

	return chario_nvme_submit_cmd(queue, cmd);
}
*/

static int submit_io_test(void) {
	struct nvme_dev *dev = chario_nvme_get_current_dev();
	struct nvme_ns *ns = list_first_entry(&dev->namespaces, struct nvme_ns, list);
	int result;

	char *data = kmalloc(8192, GFP_KERNEL);

	struct nvme_user_io uio = {
		.opcode = nvme_cmd_read,	// (__u8)  Read command
		.flags = 0,			// (__u8)  No flags?
		.control = 0,			// (__u16) ?
		.nblocks = 1,			// (__u16) 1 block (4096 bytes)
		.rsvd = 0,			// (__u16) ?
		.metadata = 0,			// (__u64) ?
		.addr = (__u64)(uintptr_t)data,	// (__u64) Buffer address
		.slba = 0,			// (__u64) Block 0
		.dsmgmt = 0,			// (__u32) ?
		.reftag = 0,			// (__u32) ?
		.apptag = 0,			// (__u16) ?
		.appmask = 0			// (__u16) ?
	};

	pr_debug("CharIO: submit_io_test()");

	result = chario_nvme_submit_io_kernel(ns, &uio);

	pr_debug("CharIO: submit_io_test() result: %d", result);

	if (strlen(data) < 50) {
		pr_debug("CharIO: submit_io_test()   data: '%s'", data);
	}
	else {
		data[50] = 0;
		pr_debug("CharIO: submit_io_test()   data: '%s' (truncated)", data);
	}

	kfree(data);

	return result;
}

static int submit_user_io(char *buffer, size_t len, loff_t offset, __u8 command) {
	struct nvme_dev *dev = chario_nvme_get_current_dev();
	struct nvme_ns *ns = list_first_entry(&dev->namespaces, struct nvme_ns, list);
	__u16 nblocks = (__u16)((len - 1) >> ns->lba_shift); // Should give number of blocks - 1
	__u64 slba = (__u64)(offset >> ns->lba_shift);
	int result;

	struct nvme_user_io uio = {
		.opcode = command,			// (__u8)  Read command
		.flags = 0,				// (__u8)  No flags?
		.control = 0,				// (__u16) ?
		.nblocks = nblocks,			// (__u16) Number of blocks: floor((length - 1) / lba size)
		.rsvd = 0,				// (__u16) ?
		.metadata = 0,				// (__u64) ?
		.addr = (__u64)(uintptr_t)buffer,	// (__u64) Buffer address
		.slba = slba,				// (__u64) Block address: floor(offset / lba size)
		.dsmgmt = 0,				// (__u32) ?
		.reftag = 0,				// (__u32) ?
		.apptag = 0,				// (__u16) ?
		.appmask = 0				// (__u16) ?
	};

	if (len == 0) {
		return 0;
	}

	pr_debug("CharIO: submit_user_io() nblocks: %hu, slba: %llu, addr: 0x%08x, opcode: %hhu", nblocks, slba, (unsigned)buffer, command);

	result = chario_nvme_submit_io_user(ns, &uio);

	pr_debug("CharIO: submit_user_io() result: %d", result);

	return result;
}


static int __submit_phys_io(dma_addr_t address, size_t len, loff_t offset, __u8 command) {
	struct nvme_dev *dev = chario_nvme_get_current_dev();
	struct nvme_ns *ns = list_first_entry(&dev->namespaces, struct nvme_ns, list);
	__u16 nblocks = (__u16)((len - 1) >> ns->lba_shift); // Should give number of blocks - 1
	__u64 slba = (__u64)(offset >> ns->lba_shift);
	int result;

	struct nvme_user_io uio = {
		.opcode = command,			// (__u8)  Read command
		.flags = 0,				// (__u8)  No flags?
		.control = 0,				// (__u16) ?
		.nblocks = nblocks,			// (__u16) Number of blocks: floor((length - 1) / lba size)
		.rsvd = 0,				// (__u16) ?
		.metadata = 0,				// (__u64) ?
		.addr = (__u64)address,			// (__u64) Buffer address
		.slba = slba,				// (__u64) Block address: floor(offset / lba size)
		.dsmgmt = 0,				// (__u32) ?
		.reftag = 0,				// (__u32) ?
		.apptag = 0,				// (__u16) ?
		.appmask = 0				// (__u16) ?
	};

	if (len == 0) {
		return 0;
	}

	pr_debug("CharIO: __submit_phys_io() nblocks: %hu, slba: %llu, addr: 0x%08x, opcode: %hhu", nblocks, slba, address, command);

	result = chario_nvme_submit_io_phys(ns, &uio);

	pr_debug("CharIO: __submit_phys_io() result: %d", result);

	return result;
}


static ssize_t submit_phys_io(struct file *filp, struct chario_phys_io * io, __u8 command) {
	int result;
	size_t remaining = io->length;
	dma_addr_t address = (dma_addr_t)io->address;
	loff_t off = filp->f_pos;

	while (remaining > MAX_BYTES) {
		result = __submit_phys_io(address, MAX_BYTES, off, command);

		if (likely(result == 0)) {
			remaining -= MAX_BYTES;
			address += MAX_BYTES;
			off += MAX_BYTES;
		}
		else if (result > 0)
			return -EIO;
		else
			return result;
	}

	result = __submit_phys_io(address, remaining, off, command);

	if (likely(result == 0)) {
		filp->f_pos += io->length;
		return io->length;
	}
	else if (result > 0)
		return -EIO;
	else
		return result;
}


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init chario_init(void){
	int result = 0;

	pr_info("CharIO: Initializing CharIO device LKM\n");

	// create the kobject sysfs entry at /sys/chario -- probably not an ideal location!
	chario_kobj = kobject_create_and_add("chario", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
	if(!chario_kobj){
		pr_alert("CharIO: failed to create kobject mapping\n");
		return -ENOMEM;
	}
	// add the attributes to /sys/chario/*
	result = sysfs_create_group(chario_kobj, &attr_group);
	if(result) {
		pr_alert("CharIO: failed to create sysfs group\n");
		kobject_put(chario_kobj); // clean up -- remove the kobject sysfs entry
		return result;
	}

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		pr_alert("CharIO failed to register a major number\n");
		return majorNumber;
	}
	pr_info("CharIO: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	charioClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charioClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Failed to register device class\n");
		return PTR_ERR(charioClass);          // Correct way to return an error on a pointer
	}
	pr_info("CharIO: device class registered correctly\n");

	// Register the device driver
	charioDevice = device_create(charioClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charioDevice)){               // Clean up if there is an error
		class_destroy(charioClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Failed to create the device\n");
		return PTR_ERR(charioDevice);
	}
	pr_info("CharIO: device class created correctly\n"); // Made it! device was initialized

	pr_info("CharIO: Initialising NVMe driver...\n");
	result = chario_nvme_init();
	pr_info("CharIO: NVMe init returned %d\n", result);

	if (result != 0) {
		device_destroy(charioClass, MKDEV(majorNumber, 0));     // remove the device
		class_unregister(charioClass);                          // unregister the device class
		class_destroy(charioClass);                             // remove the device class
		unregister_chrdev(majorNumber, DEVICE_NAME);            // unregister the major number
		kobject_put(chario_kobj);                               // clean up -- remove the kobject sysfs entry
		return result;
	}

	return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit chario_exit(void){

	pr_info("CharIO: Exiting NVMe driver...\n");
	chario_nvme_exit();

	device_destroy(charioClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(charioClass);                          // unregister the device class
	class_destroy(charioClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);            // unregister the major number
	kobject_put(chario_kobj);                               // clean up -- remove the kobject sysfs entry
	pr_info("CharIO: LKM exited\n");
}


static int dev_open(struct inode *inode, struct file *filp){
	pr_info("CharIO: Device opened\n");
	return 0;
}

static int dev_release(struct inode *inode, struct file *filp) {
	pr_info("CharIO: Device successfully closed\n");
	return 0;
}

static ssize_t dev_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	int result;
	size_t remaining = count;
	char *buffer = buf;
	loff_t off = *f_pos;

	while (remaining > MAX_BYTES) {
		result = submit_user_io(buffer, MAX_BYTES, off, nvme_cmd_read);

		if (likely(result == 0)) {
			remaining -= MAX_BYTES;
			buffer += MAX_BYTES;
			off += MAX_BYTES;
		}
		else if (result > 0)
			return -EIO;
		else
			return result;
	}

	result = submit_user_io(buffer, remaining, off, nvme_cmd_read);

	if (likely(result == 0)) {
		*f_pos += count;
		return count;
	}
	else if (result > 0)
		return -EIO;
	else
		return result;
}

static ssize_t dev_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	int result;
	size_t remaining = count;
	char *buffer = (char *)buf;
	loff_t off = *f_pos;

	while (remaining > MAX_BYTES) {
		result = submit_user_io(buffer, MAX_BYTES, off, nvme_cmd_write);

		if (likely(result == 0)) {
			remaining -= MAX_BYTES;
			buffer += MAX_BYTES;
			off += MAX_BYTES;
		}
		else if (result > 0)
			return -EIO;
		else
			return result;
	}

	result = submit_user_io(buffer, remaining, off, nvme_cmd_write);

	if (likely(result == 0)) {
		*f_pos += count;
		return count;
	}
	else if (result > 0)
		return -EIO;
	else
		return result;
}

static loff_t dev_llseek(struct file *filp, loff_t off, int whence) {
	loff_t newpos;

	switch (whence) {
		case SEEK_SET:
			newpos = off;
			break;
		case SEEK_CUR:
			newpos = filp->f_pos + off;
			break;
		case SEEK_END:
			pr_warn("CharIO: llseek() past end of device\n");
			return -ESPIPE;
		default:
			pr_warn("CharIO: llseek() invalid whence %d\n", whence);
			return -EINVAL;
	}

	if (newpos < 0) {
		pr_warn("CharIO: llseek() before start of device\n");
		return -EINVAL;
	}

	pr_debug("CharIO: llseek() by %lld to %lld\n", off, filp->f_pos);

	filp->f_pos = newpos;
	return newpos;
}

static long dev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	switch (cmd) {
		case CHARIO_IOCTL_READ_PHYS:
			return submit_phys_io(filp, (struct chario_phys_io *)arg, nvme_cmd_read);
		case CHARIO_IOCTL_WRITE_PHYS:
			return submit_phys_io(filp, (struct chario_phys_io *)arg, nvme_cmd_write);
		default:
			return -EINVAL;
	}
}


static ssize_t test_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
	int result = submit_io_test();
	return sprintf(buf, "submit_io_test() result: %d\n", result);
}

static ssize_t test_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
	return count;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(chario_init);
module_exit(chario_exit);

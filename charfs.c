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

#define MAX_BYTES 131072 // 128k - Maximum transfer size in one NVMe request (from testing)

#define  DEVICE_NAME "charfs"     ///< The device will appear at /dev/charfs using this value
#define  CLASS_NAME  "charfs"     ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Russell Joyce");   ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("CharFS test"); ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static struct class*  charfsClass  = NULL;  ///< The device-driver class struct pointer
static struct device* charfsDevice = NULL;  ///< The device-driver device struct pointer
static char   attrsName[] = "attrs";

static loff_t current_offset;

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static loff_t  dev_llseek(struct file *filp, loff_t off, int whence);

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
	.llseek = dev_llseek
};

static struct kobj_attribute test_attr = __ATTR(test, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP, test_show, test_store);

static struct attribute *charfs_attrs[] = {
	&test_attr.attr,
	NULL,
};

static struct attribute_group attr_group = {
	.name  = attrsName,
	.attrs = charfs_attrs,
};

static struct kobject *charfs_kobj;

/*
static int rq_test(void) {

	struct nvme_dev *dev = charfs_nvme_get_current_dev();
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
//		pr_err("CharFS: No memory to allocate request");
//		return 1;
//	}

	struct nvme_cmd_info *cmd = blk_mq_rq_to_pdu(req);





	const struct blk_mq_queue_data bd = {
		.rq = req,
		.last = true,
		.list = NULL
	};


	return charfs_nvme_queue_rq(&hctx, &bd);
}
*/
/*
static int submit_cmd_test(void) {
	struct nvme_dev *dev = charfs_nvme_get_current_dev();
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

	return charfs_nvme_submit_cmd(queue, cmd);
}
*/

static int submit_io_test(void) {
	struct nvme_dev *dev = charfs_nvme_get_current_dev();
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

	pr_debug("CharFS: submit_io_test()");

	result = charfs_nvme_submit_io_kernel(ns, &uio);

	pr_debug("CharFS: submit_io_test() result: %d", result);

	if (strlen(data) < 50) {
		pr_debug("CharFS: submit_io_test()   data: '%s'", data);
	}
	else {
		data[50] = 0;
		pr_debug("CharFS: submit_io_test()   data: '%s' (truncated)", data);
	}

	kfree(data);

	return result;
}

static int submit_user_io(char *buffer, size_t len, loff_t offset, __u8 command) {
	struct nvme_dev *dev = charfs_nvme_get_current_dev();
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

	pr_debug("CharFS: submit_user_io() nblocks: %hu, slba: %llu, addr: 0x%08x, opcode: %hhu", nblocks, slba, (unsigned)buffer, command);

	result = charfs_nvme_submit_io_user(ns, &uio);

	pr_debug("CharFS: submit_user_io() result: %d", result);

	return result;
}


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init charfs_init(void){
	int result = 0;

	pr_info("CharFS: Initializing CharFS device LKM\n");

	// create the kobject sysfs entry at /sys/charfs -- probably not an ideal location!
	charfs_kobj = kobject_create_and_add("charfs", kernel_kobj->parent); // kernel_kobj points to /sys/kernel
	if(!charfs_kobj){
		pr_alert("CharFS: failed to create kobject mapping\n");
		return -ENOMEM;
	}
	// add the attributes to /sys/charfs/*
	result = sysfs_create_group(charfs_kobj, &attr_group);
	if(result) {
		pr_alert("CharFS: failed to create sysfs group\n");
		kobject_put(charfs_kobj); // clean up -- remove the kobject sysfs entry
		return result;
	}

	// Try to dynamically allocate a major number for the device -- more difficult but worth it
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
	if (majorNumber<0){
		pr_alert("CharFS failed to register a major number\n");
		return majorNumber;
	}
	pr_info("CharFS: registered correctly with major number %d\n", majorNumber);

	// Register the device class
	charfsClass = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(charfsClass)){                // Check for error and clean up if there is
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Failed to register device class\n");
		return PTR_ERR(charfsClass);          // Correct way to return an error on a pointer
	}
	pr_info("CharFS: device class registered correctly\n");

	// Register the device driver
	charfsDevice = device_create(charfsClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
	if (IS_ERR(charfsDevice)){               // Clean up if there is an error
		class_destroy(charfsClass);           // Repeated code but the alternative is goto statements
		unregister_chrdev(majorNumber, DEVICE_NAME);
		pr_alert("Failed to create the device\n");
		return PTR_ERR(charfsDevice);
	}
	pr_info("CharFS: device class created correctly\n"); // Made it! device was initialized

	pr_info("CharFS: Initialising NVMe driver...\n");
	result = charfs_nvme_init();
	pr_info("CharFS: NVMe init returned %d\n", result);

	if (result != 0) {
		device_destroy(charfsClass, MKDEV(majorNumber, 0));     // remove the device
		class_unregister(charfsClass);                          // unregister the device class
		class_destroy(charfsClass);                             // remove the device class
		unregister_chrdev(majorNumber, DEVICE_NAME);            // unregister the major number
		kobject_put(charfs_kobj);                               // clean up -- remove the kobject sysfs entry
		return result;
	}

	return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit charfs_exit(void){

	pr_info("CharFS: Exiting NVMe driver...\n");
	charfs_nvme_exit();

	device_destroy(charfsClass, MKDEV(majorNumber, 0));     // remove the device
	class_unregister(charfsClass);                          // unregister the device class
	class_destroy(charfsClass);                             // remove the device class
	unregister_chrdev(majorNumber, DEVICE_NAME);            // unregister the major number
	kobject_put(charfs_kobj);                               // clean up -- remove the kobject sysfs entry
	pr_info("CharFS: LKM exited\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
	current_offset = 0;
	pr_info("CharFS: Device opened\n");
	return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
	int result;
	size_t remaining = len;
	char *buff = buffer;
	loff_t off = current_offset;

	while (remaining > MAX_BYTES) {
		result = submit_user_io(buff, MAX_BYTES, off, nvme_cmd_read);

		if (likely(result == 0)) {
			remaining -= MAX_BYTES;
			buff += MAX_BYTES;
			off += MAX_BYTES;
		}
		else if (result > 0)
			return -EIO;
		else
			return result;
	}

	result = submit_user_io(buff, remaining, off, nvme_cmd_read);

	if (likely(result == 0)) {
		current_offset += len;
		return len;
	}
	else if (result > 0)
		return -EIO;
	else
		return result;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
	int result;
	size_t remaining = len;
	char *buff = (char *)buffer;
	loff_t off = current_offset;

	while (remaining > MAX_BYTES) {
		result = submit_user_io(buff, MAX_BYTES, off, nvme_cmd_write);

		if (likely(result == 0)) {
			remaining -= MAX_BYTES;
			buff += MAX_BYTES;
			off += MAX_BYTES;
		}
		else if (result > 0)
			return -EIO;
		else
			return result;
	}

	result = submit_user_io(buff, remaining, off, nvme_cmd_write);

	if (likely(result == 0)) {
		current_offset += len;
		return len;
	}
	else if (result > 0)
		return -EIO;
	else
		return result;
}

static loff_t dev_llseek(struct file *filp, loff_t off, int whence) {
	switch (whence) {
		case SEEK_SET:
			current_offset = off;
			break;
		case SEEK_CUR:
			current_offset += off;
			break;
		case SEEK_END:
			pr_warn("CharFS: llseek() past end of device\n");
			return -ESPIPE;
		default:
			pr_warn("CharFS: llseek() invalid whence %d\n", whence);
			return -EINVAL;
	}

	pr_debug("CharFS: llseek() by %lld to %lld\n", off, current_offset);

	return current_offset;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
	pr_info("CharFS: Device successfully closed\n");
	return 0;
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
module_init(charfs_init);
module_exit(charfs_exit);

#ifndef CHARIO_MODULE_NVME_CORE_H
#define CHARIO_MODULE_NVME_CORE_H

//#include <linux/nvme.h>
#include "nvme.h"

#define REQ_MAX_BYTES 131072 // 128k - Maximum transfer size in one NVMe request (from testing)
#define REQ_MAX_BLOCKS 32 // 32 x 4096k blocks - Maximum transfer size in one NVMe request (from testing)

int chario_nvme_init(void);
void chario_nvme_exit(void);

int chario_nvme_submit_iod(struct nvme_queue *nvmeq, struct nvme_iod *iod, struct nvme_ns *ns);
int chario_nvme_queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd);

int chario_nvme_submit_cmd(struct nvme_queue *nvmeq, struct nvme_command *cmd);

int chario_nvme_submit_io_kernel(struct nvme_ns *ns, struct nvme_user_io *uio);
int chario_nvme_submit_io_user(struct nvme_ns *ns, struct nvme_user_io *uio);
int chario_nvme_submit_io_phys(struct nvme_ns *ns, struct nvme_user_io *uio);

struct nvme_dev *chario_nvme_get_current_dev(void);

struct nvme_phys_iod {
	int npages;		/* In the PRP list. 0 means small pool in use */
	int offset;		/* Of PRP list */
	int length;		/* Of data, in bytes */
	dma_addr_t prp1;
	dma_addr_t prp2;
	struct scatterlist meta_sg[1]; /* metadata requires single contiguous buffer */
};

#endif //CHARIO_MODULE_NVME_CORE_H

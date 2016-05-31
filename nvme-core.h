#ifndef CHARFS_MODULE_NVME_CORE_H
#define CHARFS_MODULE_NVME_CORE_H

#include <linux/nvme.h>

int charfs_nvme_init(void);
void charfs_nvme_exit(void);

int charfs_nvme_submit_iod(struct nvme_queue *nvmeq, struct nvme_iod *iod, struct nvme_ns *ns);
int charfs_nvme_queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd);

struct nvme_dev *charfs_nvme_get_current_dev(void);


#endif //CHARFS_MODULE_NVME_CORE_H

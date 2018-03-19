/*
 * Copyright Altera Corporation (C) 2013,2015. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/platform_device.h>

#include "common.h"
#include "altera_rpdma.h"
#include "altera_epdma.h"

#define MSGDMA_SW_RESET_COUNT		10000
#define MSGDMA_POLL_TIMEOUT		50000

static int msgdma_bit_is_clear(void __iomem *ioaddr, const u32 reg,
			       u32 bit_mask)
{
	u32 value = csr_readl(ioaddr, reg);

	return (value & bit_mask) ? 0 : 1;
}

static void msgdma_xfer(struct dma_device *dmadev, const u32 src_paddr,
			const u32 dest_paddr, const u32 size)
{
	csr_writel(dmadev->dmades, src_paddr, MSGDMA_READ_ADDR);
	csr_writel(dmadev->dmades, dest_paddr, MSGDMA_WRITE_ADDR);
	csr_writel(dmadev->dmades, size, MSGDMA_DES_LEN);

	/* reset and start */
	csr_writel(dmadev->perf_base, PERFCTR_GBL_CLK_CTR_LO_RST,
		   PERFCTR_GBL_CLK_CTR_LO);
	csr_writel(dmadev->perf_base, PERFCTR_GBL_CLK_CTR_HI_START,
		   PERFCTR_GBL_CLK_CTR_HI);

	/* start transfer */
	csr_writel(dmadev->dmades, MSGDMA_DES_CTL_TX, MSGDMA_DES_CTL);
}

static int msgdma_reset(void __iomem *dmacsr)
{
	int counter;

	csr_writel(dmacsr, MSGDMA_CTL_RESET, MSGDMA_CTL);

	counter = 0;
	while (counter++ < MSGDMA_SW_RESET_COUNT) {
		if (msgdma_bit_is_clear(dmacsr, MSGDMA_STS, MSGDMA_STS_RESET))
			break;
		udelay(1);
	}

	if (counter >= MSGDMA_SW_RESET_COUNT) {
		pr_err("msgdma resetting bit never cleared!\n");
		return -ETIMEDOUT;
	}

	/* clear all status bits */
	csr_writel(dmacsr, MSGDMA_STS_MASK, MSGDMA_STS);
	return 0;
}

irqreturn_t msgdma_isr(int irq, void *arg)
{
	u32 *vsrc, *vdest;
	u32 cnt = 0;
	struct dma_device *dmadev = (struct dma_device *)arg;

	/* clear interrupt */
	csr_writel(dmadev->dmacsr, MSGDMA_STS_IRQ,  MSGDMA_STS);

	vsrc = dmadev->vsrc;
	vdest = dmadev->vdest;
	/* last 4 bytes location */
	vsrc += ((dmadev->datlen / 4) - 1);
	vdest += ((dmadev->datlen / 4) - 1);

	/* wait until last 4 bytes posted write complete */
	while (*vsrc != *vdest) {
		if(cnt < MSGDMA_POLL_TIMEOUT)
			cnt++;
		else break;
	}

	/* Stop counter */
	csr_writel(dmadev->perf_base, PERFCTR_GBL_CLK_CTR_LO_STOP,
		   PERFCTR_GBL_CLK_CTR_LO);

	if (cnt < MSGDMA_POLL_TIMEOUT)
		dmadev->perf_tick = csr_readl(dmadev->perf_base,
					      PERFCTR_GBL_CLK_CTR_LO);
	else
		dmadev->perf_tick = 0;

	complete(&dmadev->dma_done);

	return IRQ_HANDLED;
}

u32 msgdma_transfer(struct dma_device *dmadev, const u32 src_paddr,
		    const u32 dest_paddr, const u32 size)
{
	reinit_completion(&dmadev->dma_done);

	csr_writel(dmadev->dmacsr, MSGDMA_STS_MASK, MSGDMA_STS);

	/* Init SGDMA transfer */
	msgdma_xfer(dmadev, src_paddr, dest_paddr, size);

	/* wait DMA transfer complete */
	if (!wait_for_completion_timeout(&dmadev->dma_done,
					 DMA_TIMEOUT)) {
		return 0;
	}

	/* in usec */
	return dmadev->perf_tick / dmadev->perf_mhz;
}

int msgdma_init(void __iomem *dmacsr)
{
	int ret;
	ret = msgdma_reset(dmacsr);
	if (ret)
		return ret;

	csr_writel(dmacsr, MSGDMA_CTL_IE_GLOBAL, MSGDMA_CTL);
	return 0;
}

static int block_ioctl(struct block_device *bdev, fmode_t mode,
		       unsigned int cmd, unsigned long arg)
{
	struct dma_device *dmadev = bdev->bd_disk->private_data;
	u32 timediff;

	switch (cmd) {
	case GET_SIZE_IOCTL:
		if (copy_to_user((void *)arg, (void *)&dmadev->datlen,
				 sizeof(u32)))
			return -EFAULT;
		return 0;
	case SET_CMD_IOCTL:
		if (copy_from_user((void *)&dmadev->dmaxfer_cmd,
				   (void __user *)arg, sizeof(u32)))
			return -EFAULT;
		block_ioctl_setup(dmadev);
		return 0;
	case OCM_TX_IOCTL:
	case OCM_RX_IOCTL:
	case SYS_TX_IOCTL:
	case SYS_RX_IOCTL:

		memset(dmadev->vdest, 0, dmadev->datlen);
		timediff = msgdma_transfer(dmadev, dmadev->osrc,
					   dmadev->odest, dmadev->datlen);
		if (copy_to_user((void *)arg, (void *)&timediff,
				 sizeof(timediff)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY;
}

static void blk_request(struct request_queue *rq)
{
	int err;
	struct request *req;
	u32 offset, nbytes;
	struct dma_device *dmadev;

	req = blk_fetch_request(rq);

	while (req) {
		if (req->cmd_type != REQ_TYPE_FS)
			continue;

		dmadev = req->rq_disk->private_data;
		offset = blk_rq_pos(req) * SECTOR_SIZE;
		nbytes = blk_rq_cur_sectors(req) * SECTOR_SIZE;
		err = -EIO;
		if ((offset + nbytes) <= dmadev->datlen) {
			err = 0;
			switch (rq_data_dir(req)) {
			case READ:
				if (dmadev->dmaxfer_cmd)
					memcpy(bio_data(req->bio),
					       dmadev->vdest + offset, nbytes);
				break;
			case WRITE:
				if (dmadev->dmaxfer_cmd)
					memcpy(dmadev->vsrc + offset,
					       bio_data(req->bio), nbytes);
				break;
			default:
				err = -EIO;
				pr_err("Unknown request %u\n",
				       rq_data_dir(req));
				break;
			}
		}

		if (!__blk_end_request_cur(req, err))
			req = blk_fetch_request(rq);
	}
}

static const struct block_device_operations block_fops = {
	.owner = THIS_MODULE,
	.ioctl = block_ioctl,
};

int block_device_init(struct dma_device *dmadev, const char *dev_name)
{
	int err;
	int major;

	/* register as block device */
	major = register_blkdev(0, dev_name);
	if (major < 0) {
		dev_err(dmadev->dev, "failed to register blkdev %s\n",
			dev_name);
		return -EBUSY;
	}
	dmadev->major = major;
	spin_lock_init(&dmadev->lock);

	dmadev->block_queue = blk_init_queue(blk_request, &dmadev->lock);
	if (!dmadev->block_queue) {
		dev_err(dmadev->dev, "error in blk_init_queue\n");
		err = -ENOMEM;
		goto err_unregdev;
	}

	/* whole disk, no partition */
	dmadev->gd = alloc_disk(1);
	if (!dmadev->gd) {
		dev_err(dmadev->dev, "error in alloc_disk\n");
		err = -ENOMEM;
		goto err_deinit_queue;
	}

	dmadev->gd->major = major;
	dmadev->gd->first_minor = 0;
	dmadev->gd->fops = &block_fops;
	dmadev->gd->private_data = dmadev;
	sprintf(dmadev->gd->disk_name, "%s%d", dev_name, 0);
	set_capacity(dmadev->gd, dmadev->datlen / SECTOR_SIZE);

	dmadev->gd->queue = dmadev->block_queue;

	add_disk(dmadev->gd);
	return 0;
err_deinit_queue:
	blk_cleanup_queue(dmadev->block_queue);
err_unregdev:
	unregister_blkdev(major, dev_name);
	return err;
}

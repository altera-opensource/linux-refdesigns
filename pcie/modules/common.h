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
#ifndef _COMMON_H_
#define _COMMON_H_
#include "altera_rpdma.h"
#include "exports.h"

#define PCI_VENDOR_ID_ALTERA	0x1172

#define DMA_TIMEOUT		(msecs_to_jiffies(100)) /* msec */

/* MSGDMA CSR Registers */
#define MSGDMA_STS		0x0
#define MSGDMA_STS_BUSY		BIT(0)
#define MSGDMA_STS_RESET	BIT(6)
#define MSGDMA_STS_IRQ		BIT(9)
#define MSGDMA_STS_MASK		0x3ff
#define MSGDMA_CTL		0x04
#define MSGDMA_CTL_RESET	BIT(1)
#define MSGDMA_CTL_IE_GLOBAL	BIT(4)

/* MSGDMA Descriptor */
#define MSGDMA_READ_ADDR	0x00
#define MSGDMA_WRITE_ADDR	0x04
#define MSGDMA_DES_LEN		0x08
#define MSGDMA_DES_CTL		0x0C
#define MSGDMA_DES_CTL_TC_IRQ	BIT(14)
#define MSGDMA_DES_CTL_GO	BIT(31)
#define MSGDMA_DES_CTL_TX	(MSGDMA_DES_CTL_TC_IRQ | MSGDMA_DES_CTL_GO)

/* Performance Counter */
#define PERFCTR_GBL_CLK_CTR_LO		0x00
#define PERFCTR_GBL_CLK_CTR_LO_STOP	0x00000000
#define PERFCTR_GBL_CLK_CTR_LO_RST	0x00000001
#define PERFCTR_GBL_CLK_CTR_HI		0x04
#define PERFCTR_GBL_CLK_CTR_HI_START	0x00000000

struct dma_device {
	struct device	*dev;
	spinlock_t	lock; /* lock for block queue */
	void __iomem	*rpocm;
	void __iomem	*epocm;
	void __iomem	*epcra; /* for endpoint only */
	void __iomem	*dmacsr;
	void __iomem	*dmades;
	void __iomem	*perf_base;
	u32		epocm_dmaoff;
	u32		rpocm_dmaoff;
	u8		*sys_buf;
	u32		sys_buf_phys;
	u32		sys_buff_dmaoff;
	u32		translation_mask; /* for endpoint only */
	u32		datlen;
	u32		dmaxfer_cmd;
	u32		perf_mhz;
	u32		perf_tick;
	u32		osrc;
	u32		odest;
	void		*vsrc;
	void		*vdest;
	u32		major;
	struct request_queue *block_queue;
	struct completion dma_done;
	struct gendisk	*gd;
};

static inline void csr_writel(void __iomem *ioaddr, const u32 value,
			      const u32 reg)
{
	writel_relaxed(value, ioaddr + reg);
}

static inline u32 csr_readl(void __iomem *ioaddr, const u32 reg)
{
	return readl_relaxed(ioaddr + reg);
}

u32 msgdma_transfer(struct dma_device *dmadev, const u32 src_paddr,
		    const u32 dest_paddr, const u32 size);
int msgdma_init(void __iomem *dmacsr);
irqreturn_t msgdma_isr(int irq, void *arg);
int block_device_init(struct dma_device *dmadev, const char *dev_name);

void block_ioctl_setup(struct dma_device *dmadev);

#endif

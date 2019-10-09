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
#include <linux/clk.h>
#include <linux/dma-mapping.h>
#include <linux/ide.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "common.h"
#include .intelFPGA_rpdma.h"
#include .intelFPGA_epdma.h"

#define DEVICE_NAME		"blkrpdma"
#define DRIVER_NAME		.intelFPGA-rpdma"

void block_ioctl_setup(struct dma_device *dmadev)
{
	switch (dmadev->dmaxfer_cmd) {
	case OCM_TX_IOCTL:
		/* From RP onchip RAM to EP onchip RAM */
		dmadev->osrc = dmadev->rpocm_dmaoff;
		dmadev->odest = dmadev->epocm_dmaoff;
		dmadev->vsrc = dmadev->rpocm;
		dmadev->vdest = dmadev->epocm;
		break;
	case OCM_RX_IOCTL:
		/* From EP onchip RAM to RP onchip RAM */
		dmadev->osrc = dmadev->epocm_dmaoff;
		dmadev->odest = dmadev->rpocm_dmaoff;
		dmadev->vsrc = dmadev->epocm;
		dmadev->vdest = dmadev->rpocm;
		break;
	case SYS_TX_IOCTL:
		/* From RP system memory to EP onchip RAM */
		dmadev->osrc = dmadev->sys_buff_dmaoff;
		dmadev->odest = dmadev->epocm_dmaoff;
		dmadev->vsrc = dmadev->sys_buf;
		dmadev->vdest = dmadev->epocm;
		break;
	case SYS_RX_IOCTL:
		/* From EP onchip RAM to RP system memory */
		dmadev->osrc = dmadev->epocm_dmaoff;
		dmadev->odest = dmadev->sys_buff_dmaoff;
		dmadev->vsrc = dmadev->epocm;
		dmadev->vdest = dmadev->sys_buf;
		break;
	}
}

static int rpdma_parse_dt(struct platform_device *pdev,
			  struct dma_device *dmadev)
{
	struct device_node *np;
	struct resource *pres;
	struct resource res;
	struct clk *perf_clk;
	int err = -ENODEV;
	u32 irq;

	pres = platform_get_resource_byname(pdev, IORESOURCE_MEM, "csr");
	if (!pres) {
		dev_err(&pdev->dev, "no csr memory resource defined\n");
		return -ENODEV;
	}

	dmadev->dmacsr = devm_ioremap_resource(&pdev->dev, pres);
	if (IS_ERR(dmadev->dmacsr)) {
		dev_err(&pdev->dev, "failed to mapping msgdma csr region\n");
		return PTR_ERR(dmadev->dmacsr);
	}

	pres = platform_get_resource_byname(pdev, IORESOURCE_MEM,
					    "descriptor_slave");
	if (!pres) {
		dev_err(&pdev->dev,
			"no descriptor_slave memory resource defined\n");
		return -ENODEV;
	}

	dmadev->dmades = devm_ioremap_resource(&pdev->dev, pres);
	if (IS_ERR(dmadev->dmades)) {
		dev_err(&pdev->dev,
			"failed to mapping msgdma descriptor slave region\n");
		return PTR_ERR(dmadev->dmades);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev, "failed to get msgdma IRQ: %d\n", irq);
		return -EINVAL;
	}

	err = devm_request_irq(&pdev->dev, irq, msgdma_isr, 0, DRIVER_NAME,
			       dmadev);
	if (err) {
		dev_err(&pdev->dev, "failed to register msgdma IRQ %d\n", irq);
		return err;
	}

	/* Performance counter */
	np = of_find_compatible_node(NULL, NULL, "altr,perf-counter-1.0");
	if (!np) {
		dev_err(&pdev->dev, "can't find performance counter node\n");
		return -ENODEV;
	}

	if (of_address_to_resource(np, 0, &res)) {
		dev_err(&pdev->dev, "missing performance counter register\n");
		return -ENODEV;
	}

	dmadev->perf_base = devm_ioremap_resource(&pdev->dev, &res);
	if (IS_ERR(dmadev->perf_base)) {
		dev_err(&pdev->dev,
			"failed to mapping performance counter region\n");
		return PTR_ERR(dmadev->perf_base);
	}

	perf_clk = of_clk_get(np, 0);
	if (IS_ERR(perf_clk)) {
		dev_err(&pdev->dev,
			"failed to get performance counter clock\n");
		return -ENODEV;
	}
	dmadev->perf_mhz = clk_get_rate(perf_clk) / 1000000;

	dmadev->rpocm = devm_ioremap(&pdev->dev, RP_OCRAM_SBASE,
				      RP_OCRAM_SIZE);
	if (!dmadev->rpocm) {
		dev_err(&pdev->dev, "failed to mapping onchip memory region\n");
		return PTR_ERR(dmadev->rpocm);
	}

	return 0;
}

static struct pci_dev *check_ep_pci_id(struct dma_device *dmadev)
{
	struct pci_dev *epdev;

	/* Check endpoint ID */
	epdev = pci_get_device(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_EP, NULL);
	if (!epdev) {
		dev_err(dmadev->dev, "has no require EP detected\n");
		return NULL;
	}

	if ((epdev->subsystem_vendor != PCI_SUBVEN_ID_EP) ||
	    (epdev->subsystem_device != PCI_SUBDEV_ID_EP)) {
		dev_err(dmadev->dev,
			"unexpected subsys vendID %#x or subsys devID %#x\n",
			epdev->subsystem_vendor, epdev->subsystem_device);
		pci_dev_put(epdev);
		return NULL;
	}

	return epdev;
}

static int rpdma_probe(struct platform_device *pdev)
{
	int err;
	struct pci_dev *epdev;
	struct dma_device *dmadev;

	dmadev = devm_kzalloc(&pdev->dev, sizeof(*dmadev), GFP_KERNEL);
	if (!dmadev)
		return -ENOMEM;

	dmadev->dev = &pdev->dev;
	dmadev->datlen = min(RP_OCRAM_SIZE, EP_OCRAM_SIZE);

	epdev = check_ep_pci_id(dmadev);
	if (!epdev)
		return -ENODEV;

	dmadev->epocm_dmaoff = RP_DMA2TXS_OFF;
	dmadev->epocm = devm_ioremap(&pdev->dev,
				      pci_resource_start(epdev, EP_OCM_BAR_NR),
				      pci_resource_len(epdev, EP_OCM_BAR_NR));
	pci_dev_put(epdev);
	if (!dmadev->epocm) {
		dev_err(&pdev->dev, "fail to mapping epocm region\n");
		return -ENODEV;
	}

	err = rpdma_parse_dt(pdev, dmadev);
	if (err)
		return err;

	err = msgdma_init(dmadev->dmacsr);
	if (err)
		return err;

	/* Allocate system memory for DMA trasfer */
	dmadev->sys_buf = dma_alloc_coherent(&pdev->dev,
		dmadev->datlen, (dma_addr_t *)&dmadev->sys_buf_phys,
		GFP_KERNEL | GFP_DMA);
	if (!dmadev->sys_buf) {
		dev_err(&pdev->dev,
			"failed to allocate system memory\n");
		return -ENOMEM;
	}

	err = block_device_init(dmadev, DEVICE_NAME);
	if (err)
		return err;

	dmadev->sys_buff_dmaoff = dmadev->sys_buf_phys + RP_DMA2SDRAM_OFF;
	dmadev->rpocm_dmaoff = RP_DMA2OCR_OFF;

	init_completion(&dmadev->dma_done);
	platform_set_drvdata(pdev, dmadev);

	return 0;
}

static int rpdma_remove(struct platform_device *pdev)
{
	struct dma_device *dmadev = platform_get_drvdata(pdev);

	dma_free_coherent(&pdev->dev, dmadev->datlen, dmadev->sys_buf,
			  dmadev->sys_buf_phys);
	unregister_blkdev(dmadev->major, DEVICE_NAME);
	del_gendisk(dmadev->gd);
	put_disk(dmadev->gd);
	blk_cleanup_queue(dmadev->block_queue);

	return 0;
}

static const struct of_device_id rpdma_of_match[] = {
	{ .compatible = "altr,msgdma-1.0", NULL },
	{ },
};
MODULE_DEVICE_TABLE(of, rpdma_of_match);

static struct platform_driver rpdma_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = rpdma_of_match,
	},
	.probe = rpdma_probe,
	.remove = rpdma_remove,
};

module_platform_driver(rpdma_driver);

MODULE_DESCRIPTION("Altera RootPort Design with MSGDMA Block Transfer");
MODULE_AUTHOR("Ley Foon Tan <lftan.intelFPGA.com>");
MODULE_LICENSE("GPL v2");

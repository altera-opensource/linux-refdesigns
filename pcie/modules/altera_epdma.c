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
#include <linux/dma-mapping.h>
#include <linux/ide.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/pci.h>
#include <linux/platform_device.h>

#include "common.h"
#include .intelFPGA_rpdma.h"
#include .intelFPGA_epdma.h"

#define DEVICE_NAME	"blkepdma"
#define DRIVER_NAME	.intelFPGA-epdma"

void block_ioctl_setup(struct dma_device *dmadev)
{
	switch (dmadev->dmaxfer_cmd) {
	case OCM_TX_IOCTL:
		/* From EP onchip RAM to RP onchip RAM */
		dmadev->osrc = dmadev->epocm_dmaoff;
		dmadev->odest = dmadev->rpocm_dmaoff;
		dmadev->vsrc = dmadev->epocm;
		dmadev->vdest = dmadev->rpocm;
		csr_writel(dmadev->epcra,
			   RP_DMA2OCR_OFF & dmadev->translation_mask,
			   A2P_ADDR_MAP_LO0);
		break;
	case OCM_RX_IOCTL:
		/* From RP onchip RAM to EP onchip RAM */
		dmadev->osrc = dmadev->rpocm_dmaoff;
		dmadev->odest = dmadev->epocm_dmaoff;
		dmadev->vsrc = dmadev->rpocm;
		dmadev->vdest = dmadev->epocm;
		csr_writel(dmadev->epcra,
			   RP_DMA2OCR_OFF & dmadev->translation_mask,
			   A2P_ADDR_MAP_LO0);
		break;
	case SYS_TX_IOCTL:
		/* From EP onchip RAM to RP system memory */
		dmadev->osrc = dmadev->epocm_dmaoff;
		dmadev->odest = dmadev->sys_buff_dmaoff;
		dmadev->vsrc = dmadev->epocm;
		dmadev->vdest = dmadev->sys_buf;
		csr_writel(dmadev->epcra,
			   dmadev->sys_buf_phys & dmadev->translation_mask,
			   A2P_ADDR_MAP_LO0);
		break;
	case SYS_RX_IOCTL:
		/* From RP system memory to EP onchip RAM */
		dmadev->osrc = dmadev->sys_buff_dmaoff;
		dmadev->odest = dmadev->epocm_dmaoff;
		dmadev->vsrc = dmadev->sys_buf;
		dmadev->vdest = dmadev->epocm;
		csr_writel(dmadev->epcra,
			   dmadev->sys_buf_phys & dmadev->translation_mask,
			   A2P_ADDR_MAP_LO0);
		break;
	}
}

static int setup_msi(struct pci_dev *pdev, struct dma_device *dmadev)
{
	int ret;

	ret = pci_enable_msi(pdev);
	if (ret) {
		dev_err(&pdev->dev, "failed to enable MSI interrupt.\n");
		return ret;
	}

	ret = devm_request_irq(&pdev->dev, pdev->irq, msgdma_isr, 0,
			       "epdma-msi", dmadev);
	if (ret) {
		pci_disable_msi(pdev);
		dev_err(&pdev->dev, "failed to allocate MSI interrupt\n");
		return ret;
	}

	pci_intx(pdev, 0);
	dev_info(&pdev->dev, "using MSI irq #%d\n", pdev->irq);
	return ret;
}

static int setup_intx(struct pci_dev *pdev, struct dma_device *dmadev)
{
	int ret;

	ret = devm_request_irq(&pdev->dev, pdev->irq, msgdma_isr, IRQF_SHARED,
			       "epdma-intx", dmadev);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate legacy interrupt\n");
		return ret;
	}

	pci_intx(pdev, 1);
	dev_info(&pdev->dev, "using legacy irq #%d\n", pdev->irq);
	return ret;
}

static int setup_interrupt(struct pci_dev *pdev, struct dma_device *dmadev)
{
	int ret;

	ret = setup_msi(pdev, dmadev);
	if (!ret)
		return ret;

	return setup_intx(pdev, dmadev);
}

static int epdma_probe(struct pci_dev *pdev,
		       const struct pci_device_id *pci_id)
{
	int err;
	struct pci_dev *rpdev;
	void __iomem   *csrbar_base;
	struct dma_device *dmadev;

	dmadev = devm_kzalloc(&pdev->dev, sizeof(*dmadev), GFP_KERNEL);
	if (!dmadev)
		return -ENOMEM;

	dmadev->dev = &pdev->dev;
	dmadev->datlen = min(RP_OCRAM_SIZE, EP_OCRAM_SIZE);

	/* Check rootport ID */
	rpdev = pci_get_device(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_RP, NULL);
	if (!rpdev) {
		dev_err(&pdev->dev, "has no require RP detected\n");
		return -ENODEV;
	}
	pci_dev_put(rpdev);

	dmadev->rpocm = devm_ioremap(&pdev->dev, RP_OCRAM_SBASE,
					      dmadev->datlen);
	if (!dmadev->rpocm) {
		dev_err(&pdev->dev, "failed to mapping rootport OCM\n");
		return -ENODEV;
	}

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(&pdev->dev, "failed to enable pcie device\n");
		return err;
	}
	pci_set_master(pdev);

	err = pcim_iomap_regions(pdev, BIT(EP_OCM_BAR_NR) | BIT(EP_CSR_BAR_NR),
				 DRIVER_NAME);
	if (err) {
		dev_err(&pdev->dev, "failed to iomap pci device regions\n");
		return err;
	}

	dmadev->epocm_dmaoff = EP_DMA2OCRAM_ADDR;
	dmadev->epocm = pcim_iomap_table(pdev)[EP_OCM_BAR_NR];
	if (!dmadev->epocm) {
		dev_err(&pdev->dev, "failed to map EP OCM region\n");
		return err;
	}

	csrbar_base = pcim_iomap_table(pdev)[EP_CSR_BAR_NR];
	if (!csrbar_base) {
		dev_err(&pdev->dev, "failed to map EP CSR region\n");
		return err;
	}

	dmadev->epcra = csrbar_base + EP_HIP_CRA_AVOFF;
	dmadev->dmacsr = csrbar_base + EP_DMA_CSR_AVOFF;
	dmadev->dmades = csrbar_base + EP_DMA_DES_AVOFF;
	dmadev->perf_base = csrbar_base + EP_PERFCOUNTER_AVOFF;

	err = msgdma_init(dmadev->dmacsr);
	if (err)
		return err;

	err = setup_interrupt(pdev, dmadev);
	if (err)
		return err;

	/* Allocate system memory for DMA trasfer */
	dmadev->sys_buf = pci_alloc_consistent(pdev,
		dmadev->datlen, (dma_addr_t *)&dmadev->sys_buf_phys);
	if (!dmadev->sys_buf) {
		dev_err(&pdev->dev, "failed to allocate system memory for DMA\n");
		return -ENOMEM;
	}

	err = block_device_init(dmadev, DEVICE_NAME);
	if (err)
		return err;

	/* get address translation table mask */
	csr_writel(dmadev->epcra, ~0UL, A2P_ADDR_MAP_LO0);
	dmadev->translation_mask = csr_readl(dmadev->epcra, A2P_ADDR_MAP_LO0) &
					     A2P_ADDR_MAP_MASK;
	csr_writel(dmadev->epcra, 0, A2P_ADDR_MAP_HI0);
	csr_writel(dmadev->epcra, EP_DMA_IRQ_A2P, A2P_INT_ENA_REG);

	dmadev->sys_buff_dmaoff = (dmadev->sys_buf_phys + RP_DMA2SDRAM_OFF) &
				~dmadev->translation_mask;
	dmadev->rpocm_dmaoff = RP_DMA2OCR_OFF & ~dmadev->translation_mask;
	dmadev->perf_mhz = EP_PERFCOUNTER_MFREQ;

	init_completion(&dmadev->dma_done);
	pci_set_drvdata(pdev, dmadev);

	return 0;
}

static void epdma_remove(struct pci_dev *pdev)
{
	struct dma_device *dmadev = pci_get_drvdata(pdev);

	pci_free_consistent(pdev, dmadev->datlen, dmadev->sys_buf,
			    dmadev->sys_buf_phys);
	blk_cleanup_queue(dmadev->block_queue);
	unregister_blkdev(dmadev->major, DEVICE_NAME);
	del_gendisk(dmadev->gd);
	put_disk(dmadev->gd);
}

static const struct pci_device_id epdma_pci_id_tbl[] = {
	{PCI_DEVICE(PCI_VENDOR_ID_ALTERA, PCI_DEVICE_ID_EP)},
	{}
};
MODULE_DEVICE_TABLE(pci, epdma_pci_id_tbl);

static struct pci_driver epde_driver = {
	.name = DRIVER_NAME,
	.probe = epdma_probe,
	.remove = epdma_remove,
	.id_table = epdma_pci_id_tbl,
};

module_pci_driver(epde_driver);

MODULE_DESCRIPTION("Altera Endpoint Design with MSGDMA Block Transfer");
MODULE_AUTHOR("Ley Foon Tan <lftan.intelFPGA.com>");
MODULE_LICENSE("GPL v2");

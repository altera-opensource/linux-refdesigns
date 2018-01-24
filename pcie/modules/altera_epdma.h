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

#ifndef _ALTERA_EPDMA_H
#define _ALTERA_EPDMA_H

#define PCI_DEVICE_ID_EP	0xE001
#define PCI_SUBVEN_ID_EP	0x1172
#define PCI_SUBDEV_ID_EP	0x1484

#define EP_OCM_BAR_NR		0
#define EP_OCRAM_SIZE		0x40000
#define EP_CSR_BAR_NR		2

#define EP_HIP_CRA_AVOFF	0x00000000	/* PCIE CRA slave offset */
#define EP_HIP_CRA_SIZE		0x4000

#define EP_DMA_CSR_AVOFF	0x00004000	/* DMA CSR slave offset */
#define EP_DMA_CSR_SIZE		0x20

/* DMA descriptor slave offset */
#define EP_DMA_DES_AVOFF	0x00004020
#define EP_DMA_DES_SIZE		0x10

/* Performance counter slave offset */
#define EP_PERFCOUNTER_AVOFF	0x00004040
#define EP_PERFCOUNTER_SIZE	0x20
#define EP_PERFCOUNTER_MFREQ	125

#define EP_DMA_IRQ_A2P		BIT(0)
#define EP_DMA2OCRAM_BASE	0x80000000	/* DMA to onchip RAM offset */
#define EP_DMA2OCRAM_ADDR	EP_DMA2OCRAM_BASE
#define EP_DMA2OCR_OFF		0x40000000

/* EP registers offset */
#define A2P_INT_STS_REG		0x0040
#define A2P_INT_ENA_REG		0x0050
#define A2P_ADDR_MAP_LO0	0x1000
#define A2P_ADDR_MAP_HI0	0x1004
#define A2P_ADDR_MAP_MASK	0xFFFFFFFC

#endif


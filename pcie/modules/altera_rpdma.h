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

#ifndef _ALTERA_RPDMA_H
#define _ALTERA_RPDMA_H

#define PCI_DEVICE_ID_RP	0xE000

#define RP_OCRAM_SBASE		0xC0000000	/* Rootport onchip RAM base */
#define RP_OCRAM_SIZE		0x40000		/* Rootport onchip RAM size */
#define RP_DMA2OCR_OFF		0x40000000	/* DMA to onchip RAM offset */
#define RP_DMA2SDRAM_OFF	0x00000000	/* DMA to SDRAM offset */
#define RP_DMA2TXS_OFF		0xD0000000	/* DMA to TXS slave offset */

#endif


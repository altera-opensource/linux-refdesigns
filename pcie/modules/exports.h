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
#ifndef _EXPORTS_H_
#define _EXPORTS_H_

#define IOCTL_TYPE	'Z'
#define GET_SIZE_IOCTL	_IOR(IOCTL_TYPE, 1, int)
#define SET_CMD_IOCTL	_IOW(IOCTL_TYPE, 2, int)
#define OCM_RX_IOCTL	_IOR(IOCTL_TYPE, 3, int)
#define OCM_TX_IOCTL	_IOR(IOCTL_TYPE, 4, int)
#define SYS_RX_IOCTL	_IOR(IOCTL_TYPE, 5, int)
#define SYS_TX_IOCTL	_IOR(IOCTL_TYPE, 6, int)

#endif

Files
=====
- altera_epdma.c
  - Endpoint DMA transfer module source file
- altera_epdma.h
  - Endpoint DMA transfer header file
- altera_rpdma.c
  - Rootport DMA transfer module source file
- altera_rpdma.h
  - Rootport DMA transfer module header file
- common.c
  - Provides common functions shared by rootport and endpoint DMA modules.
- common.h
  - Defines device structure and functions prototype shared by rootport and
    endpoint DMA modules.
- exports.h
  - Defines the IOCTL macros and the userspace application can include this
    header file for IOCTL usage.
- Makefile
  - Makefile to build altera-rpdma and altera-epdma modules
  - To build:
    make CROSS_COMPILE=<toolchain-prefix> ARCH=<arch> KERNEL_SRC=<kernel-path>
  - To clean:
    make CROSS_COMPILE=<toolchain-prefix> ARCH=<arch>      \
      KERNEL_SRC=<kernel-path> clean

Note: altera-rpdma and altera-epdma modules are built and tested with
      kernel 4.1.


Load/Unload Modules
===================
To load modules:
insmod altera-rpdma.ko
insmod altera-epdma.ko

To unload modules:
rmmod altera-rpdma.ko
rmmod altera-epdma.ko

You may install modules into directory /lib/modules/'uname -r', then you can
use modprobe command instead.


DMA transfer ioctl calls
========================

This section attempts to describe the ioctl calls supported by
the rootport and endpoint DMA modules.

ioctl values are listed in "exports.h".

GET_SIZE_IOCTL			Get maximum transfer size

	usage:

	  ioctl(fd, GET_SIZE_IOCTL, &size);

	inputs:		none

	outputs:	size of maximum transfer size

	return:
	  0 on success, -EFAULT on failure


SET_CMD_IOCTL			Set transfer command

	usage:

	  ioctl(fd, SET_CMD_IOCTL, &cmd);

	inputs:
			OCM_RX_IOCTL	DMA receive with on-chip memory
			OCM_TX_IOCTL	DMA transmit with on-chip memory
			SYS_RX_IOCTL	DMA receive with system memory
			SYS_TX_IOCTL	DMA transmit with system memory

	outputs:	none

	return:
	  0 on success, -EFAULT on failure


OCM_RX_IOCTL			DMA receive with on-chip memory

	usage:

	  ioctl(fd, OCM_RX_IOCTL, &time);

	inputs:
			none

	outputs:	Time taken to complete the transfer (in usec)

	return:
	  0 on success, -EFAULT on failure


OCM_TX_IOCTL			DMA transmit with on-chip memory

	usage:

	  ioctl(fd, OCM_TX_IOCTL, &time);

	inputs:
			none

	outputs:	Time taken to complete the transfer (in usec)

	return:
	  0 on success, -EFAULT on failure

SYS_RX_IOCTL			DMA receive with system memory

	usage:

	  ioctl(fd, SYS_RX_IOCTL, &time);

	inputs:
			none

	outputs:	Time taken to complete the transfer (in usec)

	return:
	  0 on success, -EFAULT on failure


SYS_TX_IOCTL			DMA transmit with system memory

	usage:

	  ioctl(fd, SYS_TX_IOCTL, &time);

	inputs:
			none

	outputs:	Time taken to complete the transfer (in usec)

	return:
	  0 on success, -EFAULT on failure

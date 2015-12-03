obj-m += altera-rpdma.o
obj-m += altera-epdma.o
altera-rpdma-objs := common.o altera_rpdma.o
altera-epdma-objs := common.o altera_epdma.o

SRC := $(shell pwd)

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC)

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(SRC) clean

// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2023 Intel Corporation <www.intel.com>
 * Copyright (C) 2025 Altera Corporation
 *
 */

#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

#define DRV_NAME		"intel_pio_ilc"
#define DATA_REG		(0 << 2)
#define DIRECTION_REG		BIT(2)
#define INTERRUPTMASK_REG	(2 << 2)
#define EDGECAPTURE_REG		(3 << 2)

struct intel_pio_ilc {
	struct platform_device *pdev;
	void __iomem			*regs;
	unsigned int			irq_count;
	unsigned int			interrupt;
	struct device_attribute		dev_attr;
	char				sysfs[10];
};

static ssize_t pio_ilc_show_counter(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct intel_pio_ilc *pio_ilc = dev_get_drvdata(dev);

	sprintf(buf, "%u\n", pio_ilc->irq_count);
	strcat(buf, "\0");

	return strlen(buf);
}

static struct attribute *intel_pio_ilc_attrs[2];

struct attribute_group intel_pio_ilc_attr_group = {
	.name = "pio_ilc_data",
	.attrs = intel_pio_ilc_attrs,
};

static irqreturn_t pio_ilc_interrupt_handler(int irq, void *p)
{
	struct intel_pio_ilc *pio_ilc = (struct intel_pio_ilc *)p;

	/* Increment IRQ counter */
	pio_ilc->irq_count++;

	/* Clear all pending IRQs in gpio */
	writel(0xFFFFFFFF, pio_ilc->regs + EDGECAPTURE_REG);

	return IRQ_RETVAL(IRQ_HANDLED);
}

static int intel_pio_ilc_probe(struct platform_device *pdev)
{
	struct	intel_pio_ilc	*pio_ilc;
	struct	resource	*regs;
	int			ret;

	pio_ilc = devm_kzalloc(&pdev->dev, sizeof(struct intel_pio_ilc),
			       GFP_KERNEL);
	if (!pio_ilc)
		return -ENOMEM;

	pio_ilc->pdev = pdev;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!regs)
		return -ENXIO;

	pio_ilc->regs = devm_ioremap_resource(&pdev->dev, regs);
	if (!pio_ilc->regs)
		return -EADDRNOTAVAIL;

	/* Register IRQ */
	pio_ilc->interrupt = platform_get_irq(pdev, 0);

	ret = devm_request_irq(&pdev->dev, (pio_ilc->interrupt),
			       pio_ilc_interrupt_handler, IRQF_SHARED,
			       "pio_ilc_0", (void *)(pio_ilc));

	if (ret < 0)
		dev_warn(&pdev->dev, "Failed to register interrupt handler");

	/* Setup sysfs interface */
	sprintf(pio_ilc->sysfs, "%d", (pio_ilc->interrupt));
	pio_ilc->dev_attr.attr.name = pio_ilc->sysfs;
	pio_ilc->dev_attr.attr.mode = 0444;
	pio_ilc->dev_attr.show = pio_ilc_show_counter;
	intel_pio_ilc_attrs[0] = &pio_ilc->dev_attr.attr;
	intel_pio_ilc_attrs[1] = NULL;

	ret = sysfs_create_group(&pdev->dev.kobj, &intel_pio_ilc_attr_group);

	/* Clear any pending IRQs and Enable IRQ-0 in gpio */
	writel(0xFFFFFFFF, pio_ilc->regs + EDGECAPTURE_REG);
	writel(0x00000001, pio_ilc->regs + INTERRUPTMASK_REG);

	platform_set_drvdata(pdev, pio_ilc);

	dev_info(&pdev->dev, "Driver successfully loaded\n");

	return 0;
}

static void intel_pio_ilc_remove(struct platform_device *pdev)
{
	struct intel_pio_ilc *pio_ilc = platform_get_drvdata(pdev);

	/* Disable IRQs in gpio */
	writel(0x00000000, pio_ilc->regs + INTERRUPTMASK_REG);

	/* Remove sysfs interface */
	sysfs_remove_group(&pdev->dev.kobj, &intel_pio_ilc_attr_group);

	platform_set_drvdata(pdev, NULL);
}

static const struct of_device_id intel_pio_ilc_match[] = {
	{ .compatible = "intel,pio-ilc" },
	{ /* Sentinel */ }
};

MODULE_DEVICE_TABLE(of, intel_pio_ilc_match);

static struct platform_driver intel_pio_ilc_platform_driver = {
	.driver = {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= of_match_ptr(intel_pio_ilc_match),
	},
	.remove			= intel_pio_ilc_remove,
};

static int __init intel_pio_ilc_init(void)
{
	return platform_driver_probe(&intel_pio_ilc_platform_driver,
		intel_pio_ilc_probe);
}

static void __exit intel_pio_ilc_exit(void)
{
	platform_driver_unregister(&intel_pio_ilc_platform_driver);
}

module_init(intel_pio_ilc_init);
module_exit(intel_pio_ilc_exit);

MODULE_AUTHOR("Rod Frazer<rod.frazer@intel.com>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Altera Avalon GPIO driver for Interrupt Latency Counter use");
MODULE_ALIAS("platform:" DRV_NAME);

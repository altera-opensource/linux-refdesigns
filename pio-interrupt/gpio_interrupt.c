/*
 * GPIO interrupt handling example
 *
 * This module serves as kernel module example that can be added via
 * Angstrom recipe.
 *
 * Copyright Altera Corporation (C) 2013-2014.  All rights reserved
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

#include <linux/module.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/gpio.h>

static int gpio_number = -1;

irqreturn_t gpio_isr(int this_irq, void *dev_id)
{
	pr_info("Interrupt happened at gpio:%d\n", gpio_number);
	return IRQ_HANDLED;
}

static void __exit gpio_interrupt_exit(void)
{
	free_irq(gpio_to_irq(gpio_number), 0);
	gpio_free(gpio_number);
	return;
}

static int __init gpio_interrupt_init(void)
{
	int r;
	int req;
	int irq_number;

	if (gpio_number < 0) {
		pr_err("Please specify a valid gpio_number\n");
		return -1;
	}
	req = gpio_request(gpio_number, "pio interrupt");

	if (req != 0) {
		pr_err("Invalid gpio_number specified\n");
		return -1;
	}

	irq_number = gpio_to_irq(gpio_number);

	r = request_irq(irq_number, gpio_isr, IRQF_TRIGGER_FALLING, 0, 0);
	if (r) {
		pr_err("Failure requesting irq %i\n", irq_number);
		return r;
	}
	return 0;
}

MODULE_LICENSE("GPL");

module_param(gpio_number, int, 0);

module_init(gpio_interrupt_init);
module_exit(gpio_interrupt_exit);

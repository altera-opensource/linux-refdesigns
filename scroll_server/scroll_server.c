/*
 * Copyright (c) 2013-2014, Altera Corporation <www.intelFPGA.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - Neither the name of the Altera Corporation nor the
 *   names of its contributors may be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "../include/snprintf_s.h"
#include "led_control.h"

#define STRSIZE 256


int scroll_interval = 500;
int clear = 1;
int led_top = 3;
int led_btm = 0;

void* check_scroll_frequency_thread(void*foo)
{
	int fd;
	int dirfd = 0;
	char dir[STRSIZE];
	char readbuf[STRSIZE];

	while (1) {
		int scroll_interval_tmp;
		snprintf(dir, STRSIZE, "/home/root/.intelFPGA/frequency_fifo_scroll");
		fd = openat(dirfd, dir, O_RDONLY);
		if (fd == -1) {
			printf("Failed opening fifo frequency_fifo_scroll\n");
			return NULL;
		}
		if (read(fd, readbuf, STRSIZE) == -1)
		{
			printf("Failed opening fifo read\n");
			return NULL;
		}
		scroll_interval_tmp = atoi(readbuf);
		close(fd);

		int dirfd0 = 0;
		int dirfd1 = 0;
		int dirfd2 = 0;
		int dirfd3 = 0;
		char dir0[STRSIZE];
		char dir1[STRSIZE];
		char dir2[STRSIZE];
		char dir3[STRSIZE];
		snprintf(dir0, STRSIZE, "/sys/class/leds/fpga_led0/brightness");
		snprintf(dir1, STRSIZE, "/sys/class/leds/fpga_led1/brightness");
		snprintf(dir2, STRSIZE, "/sys/class/leds/fpga_led2/brightness");
		snprintf(dir3, STRSIZE, "/sys/class/leds/fpga_led3/brightness");
		if (openat(dirfd0, dir0, O_RDWR) == -1
		|| openat(dirfd1, dir1, O_RDWR) == -1
		|| openat(dirfd2, dir2, O_RDWR) == -1
		|| openat(dirfd3, dir3, O_RDWR) == -1) {
			printf("Missing FPGA LEDs\n");
		}

		/* if interval is 0, client is doing a read */
		if (scroll_interval_tmp == 0) {
			int fd;
			int dirfd = 0;
			char dir[STRSIZE];
			char write_scroll[STRSIZE];
			snprintf_s_i(write_scroll, STRSIZE-1,"%d", scroll_interval);
			snprintf(dir, STRSIZE, "/home/root/.intelFPGA/get_scroll_fifo");
			fd = openat(dirfd, dir, O_WRONLY);
			if (fd == -1) {
				printf("Failed opening fifo get_scroll_fifo\n");
				return NULL;
			}
			if (write(fd, write_scroll, STRSIZE-1) == -1) {
				printf("Failed opening fifo get_scroll_fifo\n");
				return NULL;
			}
			close(fd);
		}
		else {
			scroll_interval = scroll_interval_tmp;
			clear = 1;
		}
	}

	return 0;
}

void clear_leds()
{
	if (clear) {
		char trigger[] = "none";
		int lednumber = led_btm;

		for (; lednumber <= led_top; lednumber++) {
			setLEDtrigger(lednumber, trigger, sizeof(trigger));
			setLEDBrightness(lednumber, 0);
		}
		clear = 0;
	}
}

int main(int argc, char** argv)
{
	int scroll_top = led_top;
	int scroll_btm = led_btm;
	int scroll = led_btm + 1;
	int isScrollingUp = 0;

	printf("Starting blinking LED server\n");

	umask(0);
	mkdir("/home/root/.intelFPGA", 0777);
	mknod("/home/root/.intelFPGA/frequency_fifo_scroll", S_IFIFO|0666, 0);
	mknod("/home/root/.intelFPGA/get_scroll_fifo", S_IFIFO|0666, 0);

	pthread_t tid;
	pthread_create(&tid, NULL, check_scroll_frequency_thread, NULL);

	/*
	* Toggle the GPIO pin
	*/
	while (1) {
		/* Scrolling LED */
		char scroll_char[STRSIZE];
		/* Set the LED to not blinking */
		clear_leds();
		snprintf_s_i(scroll_char, STRSIZE-1,"%d", scroll);
		setLEDBrightness(scroll, 0);

		if (isScrollingUp)
			snprintf_s_i(scroll_char, STRSIZE-1,"%d", ++scroll);
		else
			snprintf_s_i(scroll_char, STRSIZE-1,"%d", --scroll);

		if (scroll <= scroll_btm) isScrollingUp = 1;
		if (scroll >= scroll_top) isScrollingUp = 0;

		setLEDBrightness(scroll, 1);
		while (scroll_interval < 0)
			usleep(500000);
		if (scroll_interval > 0)
			usleep(scroll_interval * 1000);
	}

	return 0;
}

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

#include "led_control.h"

#define STRSIZE 256

int main(int argc, char** argv)
{
	int led, on_off, i;
	int fd;
	int dirfd = 0;
	char dir[STRSIZE];
	char get_fifo_scroll[STRSIZE], seconds_bet_toggle[10];

	for (i = 1; i < argc; i++) {
		if (strcmp("--help", argv[i]) == 0) {
			printf("Usage %s: <LED number> <0|1> ; 0:off, 1:on\n"
				, argv[0]);
			return -1;
		}
	}

	if (argc != 3) {
		printf("Usage %s: <LED number> <0|1> ; 0:off, 1:on\n"
			, argv[0]);
		return -1;
	}
	else {
		led = atoi(argv[1]);
		on_off = atoi(argv[2]);
		if (led < 0 || led > 3) {
			printf("Invalid LED number. Valid LED range is 0-3\n");
			return -1;
		}
		if (on_off != 0 && on_off != 1) {
			printf("Invalid toggling. Valid input is 0 or 1\n");
			return -1;
		}
	}

	/* Checking if scrolling is still happening
	 * if it is, we will inform user, and bail
	 */
	snprintf(get_fifo_scroll, STRSIZE, "%d", 0);
	snprintf(dir, STRSIZE, "/home/root/.intelFPGA/frequency_fifo_scroll");
	fd = openat(dirfd, dir, O_WRONLY);
	if (fd == -1) {
		printf("Failed opening fifo frequency_fifo_scroll\n");
		return -1;
	}
	if (write(fd, get_fifo_scroll, STRSIZE-1) == -1) {
		printf("Failed opening fifo frequency_fifo_scroll\n");
		return -1;
	}
	close(fd);

	snprintf(dir, STRSIZE, "/home/root/.intelFPGA/get_scroll_fifo");
	fd = openat(dirfd, dir, O_RDONLY);
	if (fd == -1) {
		printf("Failed opening fifo get_scroll_fifo\n");
		return -1;
	}

	if (read(fd, seconds_bet_toggle, 10) == -1)
	{
		printf("Failed opening toggle\n");
                return -1;
	}

	close(fd);
	if (atoi(seconds_bet_toggle) > 0) {
		printf("LED is scrolling.\n");
		printf("Disable scroll_client before changing blink delay\n");
		return -1;
	}

	/*
	* Toggle the GPIO pin
	*/
	clear_led(led);
	char trigger[] = "none";

	/* Setting the trigger to not blinking */
	setLEDtrigger(led, trigger, sizeof(trigger));
	setLEDBrightness(led, on_off);

	return 0;
}

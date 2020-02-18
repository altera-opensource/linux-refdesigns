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
 * ANY EXPRESS OR IMPLIED STRSIZEWARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
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

#include "led_control.h"

#define STRSIZE 256

int main(int argc, char** argv)
{
	FILE *fp;
	int led, delay_ms, i;
	char trigger[] = "timer";
	char dir[STRSIZE];
	char delay[STRSIZE];
	int blink_interval_ms;

	for (i = 1; i < argc; i++) {
		if (strcmp("--help", argv[i]) == 0) {
			printf("Usage %s: <LED number> <Blink delay (in ms)>\n"
				, argv[0]);
			return -1;
		}
	}

	if (argc != 3) {
		printf("Usage %s: <LED number> <Blink delay (in ms)>\n"
			, argv[0]);
		return -1;
	}
	else {
		led = atoi(argv[1]);
		delay_ms = atoi(argv[2]);
		if (led < 0 || led > 3) {
			printf("Invalid LED number. Valid LED range is 0-3\n");
			return -1;
		}
		if (delay_ms <= 0) {
			printf("Invalid blink delay.\n");
			return -1;
		}
	}

	/* Checking if scrolling is still happening
	   if it is, we will inform user, and bail
	*/
	char get_fifo_scroll[STRSIZE], seconds_bet_toggle[10];
	snprintf(get_fifo_scroll, STRSIZE-1, "0");
	fp = fopen("/home/root/.intelFPGA/frequency_fifo_scroll", "w");
	if (fp == NULL) {
		printf("Failed opening fifo frequency_fifo_scroll\n");
		return -1;
	}
	fputs(get_fifo_scroll, fp);
	fclose(fp);
	fp = fopen("/home/root/.intelFPGA/get_scroll_fifo", "r");
	if (fp == NULL) {
		printf("Failed opening fifo get_scroll_fifo\n");
		return -1;
	}
	
	if (fgets(seconds_bet_toggle, 10, fp) == NULL)
	{
		printf("Failed opening fifo frequency_fifo_scroll\n");
                return -1;
	}
	
	fclose(fp);
	if (atoi(seconds_bet_toggle) > 0) {
		printf("LED is scrolling.\n");
		printf("Disable scroll_client before changing blink delay\n");
		return -1;
	}

	/* Blinking LED */
	clear_led(led);
	blink_interval_ms = delay_ms;

	/* Setting the trigger to blinking */
	setLEDtrigger(led, trigger, sizeof(trigger));
	/* Setting the delay */
	snprintf(dir, STRSIZE-1, "/sys/class/leds/fpga_led%d/delay_on", led);
	if ((fp = fopen(dir, "w")) == NULL) {
		printf("Failed to open the file %s\n" , dir);
	}
	else {
		snprintf(delay, STRSIZE-1,"%d", blink_interval_ms);
		if (fwrite(delay, 1, sizeof(delay), fp) == 0)
		{
			fclose(fp);
			return -1;
		}
		fclose(fp);
	}
	snprintf(dir, STRSIZE-1,"/sys/class/leds/fpga_led%d/delay_off", led);
	if ((fp = fopen(dir, "w")) == NULL) {
		printf("Failed to open the file %s\n" , dir);
	}
	else {
		snprintf(delay, STRSIZE-1,"%d", blink_interval_ms);
                if (fwrite(delay, 1, sizeof(delay), fp) == 0)
                {
                        fclose(fp);
                        return -1;
                }
		fclose(fp);
	}

	return 0;
}

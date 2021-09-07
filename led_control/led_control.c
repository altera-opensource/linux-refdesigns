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

#define STRSIZE 256

void setLEDtrigger(int ledno, char* trigger, int size)
{
	FILE *fp;
	char dir[STRSIZE];

	snprintf(dir, STRSIZE-1, "/sys/class/leds/fpga_led%d/trigger", ledno);

	if ((fp = fopen(dir, "w")) == NULL) {
		printf("Failed to open the file %s\n", dir);
	}
	else {
		fwrite(trigger, 1, size, fp);
		fclose(fp);
	}
}

void setLEDBrightness(int ledno, int brightness)
{
	FILE *fp;
	char dir[STRSIZE];
	char brightness_char[STRSIZE];

	snprintf(dir, STRSIZE-1, "/sys/class/leds/fpga_led%d/brightness", ledno);
	snprintf(brightness_char, STRSIZE-1, "%d", brightness);

	if ((fp = fopen(dir, "w")) == NULL) {
		printf("Failed to open the file %s\n", dir);
	}
	else {
		fwrite(brightness_char, 1, sizeof(brightness_char), fp);
		fclose(fp);
	}
}

void clear_led(int led)
{
	char trigger[] = "none";
	setLEDtrigger(led, trigger, sizeof(trigger));
	setLEDBrightness(led, 0);
}


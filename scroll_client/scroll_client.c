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

#include <linux/stat.h>

int main(int argc, char** argv)
{
	char ms_bet_toggle[100];

	if (argc != 2) {
		printf("Usage: %s <ms between LED toggle>\n", argv[0]);
		printf("  If <ms between toggle> is set to 0,\n");
		printf("  application will return the current toggling delay.\n");
		printf("  If <ms between toggle> is set to negative value,\n");
		printf("  application will stop the LED scrolling.\n");
		return -1;
	}

	if (EOF == sscanf(argv[1], "%s", ms_bet_toggle)) {
		printf("Failed reading <ms between LED toggle>\n.");
		return -1;
	}
	else {
		FILE *fp;
		fp = fopen("/home/root/.intelFPGA/frequency_fifo_scroll", "w");
		if (fp == NULL) {
			printf("Failed opening fifo frequency_fifo_scroll\n");
			return -1;
		}
		fputs(ms_bet_toggle, fp);
		fclose(fp);
		if(atoi(ms_bet_toggle) == 0) {
			fp = fopen("/home/root/.intelFPGA/get_scroll_fifo", "r");
			if (fp == NULL) {
				printf("Failed opening fifo get_scroll_fifo\n");
				return -1;
			}
			
			if (fgets(ms_bet_toggle, 10, fp) == NULL)
			{
				printf("Failed opening read\n");
				return -1;
			}
			fclose(fp);
			printf("%d", atoi(ms_bet_toggle));
			return 0;
		}
	}
	return 0;
}

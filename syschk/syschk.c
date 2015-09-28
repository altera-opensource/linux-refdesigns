/*
 * Copyright (c) 2015, Altera Corporation <www.altera.com>
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

#include <arpa/inet.h>
#include <dirent.h>
#include <malloc.h>
#include <net/if.h>
#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define MIN_WINDOW_SIZE_X 20
#define MIN_WINDOW_SIZE_Y 80

/* This defines how far the indentation of key to value */
#define INDENT 22
#define MAX_TEXT_OUTPUT 30

/* The current row to be written to. row2 stores current row for 2nd column */
int row, row2;
/* Maximum row and column of the screen size */
int maxrow, maxcol;

/* Reads the output from a specified filesystem location*/
char* read_fs(const char *node) {
	FILE* fd;
	int i=0;
	char *buff = malloc(sizeof(char)*MAX_TEXT_OUTPUT);

	fd = fopen(node, "r");
	if (!fd) {
		strcpy(buff, "N/A");
		return buff;
	}
	fgets(buff, MAX_TEXT_OUTPUT, fd);

	for(i=0; i<MAX_TEXT_OUTPUT; i++) {
		if(buff[i] == '\n')
			buff[i] = '\0';
	}
	buff[254] = 0;


	fclose(fd);

	return buff;
}

char* get_i2c_hwmon(char *i2c_address, char *pmbusname, char *type) {
	char *label, *print, *filename;
	label = strtok(pmbusname, "_");
	filename = malloc((30 +
		strlen(i2c_address))*sizeof(char) + strlen(label));
	strcpy(filename, "/sys/bus/i2c/devices/");
	strcat(filename, i2c_address);
	strcat(filename, "/");
	strcat(filename, label);
	strcat(filename, type);
	print = read_fs(filename);
	free(filename);

	return print;
}

void print_hwmon(char *device, char *i2c_address) {
	DIR *pmbusdir;
	struct dirent *pmbusent;

	if ((pmbusdir = opendir(device)) != NULL) {
		while ((pmbusent = readdir(pmbusdir)) != NULL) {
			if(strstr(pmbusent->d_name, "_label") != 0) {
				char *print = get_i2c_hwmon(i2c_address,
					pmbusent->d_name, "_label");
				mvprintw(row, 0, "%s", print);
				free(print);

				get_i2c_hwmon(i2c_address,
					pmbusent->d_name, "_input");
				mvprintw(row++, INDENT, ": %s", print);
				free(print);
			}
		}
		row++;
		closedir(pmbusdir);
	}
}

void print_all_hwmon() {
	DIR *dir;
	struct dirent *ent;
	char *filename;

	if ((dir = opendir("/sys/bus/i2c/devices")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0)
				continue;
			filename = malloc(27 + strlen(ent->d_name));
			strcpy(filename, "/sys/bus/i2c/devices/");
			strcat(filename, ent->d_name);
			strcat(filename, "/name");
			char *value = read_fs(filename);
			if (strcmp(value, "pmbus") == 0) {
				strcpy(filename, "/sys/bus/i2c/devices/");
				strcat(filename, ent->d_name);
				print_hwmon(filename, ent->d_name);
			}
			free(filename);
			free(value);
		}
		closedir(dir);
	}
	return;
}

/* Check if the text will overflow to the next line and trim as necessary */
void trim_overflow_characters(char *text) {
	int char_overflow = (strlen(text) + maxcol / 2 + INDENT) - maxcol + 2;

	if (char_overflow > 0)
		text[strlen(text) - char_overflow] = '\0';
}

void print_usb() {
	DIR *dir;
	struct dirent *ent;
	char *filename;

	if ((dir = opendir("/sys/bus/usb/devices")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			char *value;
			if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0 ||
					strstr(ent->d_name, "usb") == 0)
				continue;
			filename = malloc(30 + strlen(ent->d_name));
			strcpy(filename, "/sys/bus/usb/devices/");
			strcat(filename, ent->d_name);
			strcat(filename, "/product");
			value = read_fs(filename);
			trim_overflow_characters(value);
			mvprintw(row2, maxcol/2, "%s", ent->d_name);
			mvprintw(row2++, maxcol/2+INDENT, ": %s", value);
			free(filename);
			free(value);
		}
		row2++;
		closedir(dir);
	}
}

void print_sysid() {
	char *value, *filename;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir("/sys/bus/platform/devices")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0 ||
					strstr(ent->d_name, "sysid") == 0)
				continue;
			filename = malloc(38 + strlen(ent->d_name));
			strcpy(filename, "/sys/bus/platform/devices/");
			strcat(filename, ent->d_name);
			strcat(filename, "/sysid/id");
			value = read_fs(filename);
			mvprintw(row2, maxcol/2, "%s", ent->d_name);
			mvprintw(row2++, maxcol/2+INDENT, ": %s", value);
			free(filename);
			free(value);
		}
		row2 += 2;
		closedir(dir);
	}
}

void print_uart() {
	char *value, *filename;
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir("/proc/device-tree/soc")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0 ||
					strstr(ent->d_name, "serial") == 0)
				continue;
			filename = malloc(32 + strlen(ent->d_name));
			strcpy(filename, "/proc/device-tree/soc/");
			strcat(filename, ent->d_name);
			strcat(filename, "/status");
			value = read_fs(filename);
			mvprintw(row2, maxcol/2, "%s", ent->d_name);
			mvprintw(row2++, maxcol/2+INDENT, ": %s", value);
			free(filename);
			free(value);
		}
		row2++;
		closedir(dir);
	}
}

int check_screen() {

	if(maxrow < MIN_WINDOW_SIZE_X || maxcol < MIN_WINDOW_SIZE_Y) {
		return false;
	}

	return true;
}

void print_leds() {
	DIR *dir;
	struct dirent *ent;
	char *filename;
	if ((dir = opendir("/sys/class/leds")) != NULL) {
		while ((ent = readdir (dir)) != NULL) {
			if (strcmp(ent->d_name, ".") == 0 ||
					strcmp(ent->d_name, "..") == 0)
				continue;
			filename = malloc(30 + strlen(ent->d_name));
			strcpy(filename, "/sys/class/leds/");
			strcat(filename, ent->d_name);
			strcat(filename, "/brightness");
			char *value = read_fs(filename);
			mvprintw(row, 0, "%s", ent->d_name);
			if(strcmp(value, "1") == 0)
				mvprintw(row++, INDENT, ": ON");
			else
				mvprintw(row++, INDENT, ": OFF");
			free(value);
		}
		closedir(dir);
	}
}

void print_ip() {
	int fd;
	struct ifreq ifr;

	if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		mvprintw(row, 0, "IPv4 Address");
		mvprintw(row++, INDENT, ": N/A");
	}
	else {
		ifr.ifr_addr.sa_family = AF_INET;
		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
		ioctl(fd, SIOCGIFADDR, &ifr);
		close(fd);
		mvprintw(row, 0, "IPv4 Address");
		mvprintw(row++, INDENT, ": %s", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
	}

	row++;
}

void print_header() {
	const char *header = "ALTERA SYSTEM CHECK";

	mvprintw(0, maxcol/2-strlen(header)/2, "%s", header);
}

int main(int argc, char *argv[])
{
	initscr(); /*Initialize ncurses */
	do {
		getmaxyx(stdscr, maxrow, maxcol); /* get the max row and col */
		if (!check_screen()) {
			endwin();
			printf("Your display is too small to run this\n");
			printf("It must be at least 20 lines and 80 columns.\n");
			return -1;
		}
		/* Reinitialize the row to start from top */
		row = 2;
		row2 = 2;
		clear();
		print_header();
		print_all_hwmon();
		print_usb();
		print_ip();
		print_uart();
		print_sysid();
		print_leds();
		refresh();
		timeout (500);
	} while(getch() != 'q');
	endwin();

	return 0;
}


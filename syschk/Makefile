C_SRC := syschk.c strnlen_s.c safe_str_constraint.c ignore_handler_s.c strcat_s.c strcpy_s.c
CFLAGS := -g -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Werror -Wall -fstack-protector-all -O -I../include -I../safeclib

RM := rm -rf

ELF ?= $(basename $(firstword $(C_SRC)))
OBJ := $(patsubst %.c,%.o,$(C_SRC))

.PHONY: all
all: $(ELF)

.PHONY:
clean:
	$(RM) $(ELF) $(OBJ)

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $@ -l curses


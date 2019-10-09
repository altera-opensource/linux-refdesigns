C_SRC := scroll_client.c
CFLAGS := -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security -Werror=format-security -Werror -Wall -fstack-protector-all -O -I../include -I../safeclib

ifeq ($(or $(COMSPEC),$(ComSpec)),)
RM := rm -rf
else
RM := cs-rm -rf
endif

ELF ?= $(basename $(firstword $(C_SRC)))
OBJ := $(patsubst %.c,%.o,$(C_SRC))

.PHONY: all
all: $(ELF) $(OBJ)

.PHONY:
clean:
	$(RM) $(ELF) *.o

$(OBJ): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ELF): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) ../led_control/led_control.a -o $@ -lpthread


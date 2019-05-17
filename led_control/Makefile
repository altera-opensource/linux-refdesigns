C_SRC := led_control.c
CFLAGS := -Werror -Wall -D_FORTIFY_SOURCE=2 -fstack-protector-all -I../led_control

ifeq ($(or $(COMSPEC),$(ComSpec)),)
RM := rm -rf
else
RM := cs-rm -rf
endif

LIB ?= $(basename $(firstword $(C_SRC))).a
OBJ := $(patsubst %.c,%.o,$(C_SRC))

.PHONY: all
all: $(LIB) $(OBJ)

.PHONY:
clean:
	$(RM) $(LIB) *.o

$(OBJ): $(C_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB): $(OBJ)
	$(AR) cr $(LIB) $(OBJ)


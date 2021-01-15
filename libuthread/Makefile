# Target library
lib := libuthread.a
objs := preempt.o uthread.o context.o queue.o
CC := gcc
CFLAGS := -Wall -Wextra -Werror
CFLAGS += -g

ifneq ($(V),1)
Q = @
endif

all: $(lib)

deps := $(patsubst %.o,%.d,$(objs))
-include $(deps)
DEPFLAGS = -MMD -MF $(@:.o=.d)

## TODO: Phase 1.1

%.o: %.c
	@echo "CC $@"
	$(Q)$(CC) $(CFLAGS) -c -o $@ $< $(DEPFLAGS)

$(lib): $(objs)
	ar rcs $(lib) $(objs)

clean:
	@echo "clean"
	$(Q)rm -f $(targets) $(objs) $(deps)
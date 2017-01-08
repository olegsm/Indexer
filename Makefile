CC = gcc

APP = solution
SRCDIR += ./
INCLUDES  += $(addprefix -I, $(SRCDIR))
CFLAGS += -Wall -std=c99

HDRS=$(wildcard $(addsuffix *.h, $(SRCDIR)))
SRCS=$(wildcard $(addsuffix *.c, $(SRCDIR)))
OBJS=$(SRCS:.c=.o)

default all:: $(APP)

clean:
	for obj in $(OBJS); do \
		rm -f $$obj; \
	done
	rm -f $(APP)

test:
	find . -name '*.txt' -exec ./$(APP) {} --test \;

$(OBJS): $(SRCS) $(HDRS)
	$(CC) $(INCLUDES) $(CFLAGS) -c $(@:.o=.c) -o $@

$(APP): $(OBJS) $(HDRS)
	$(CC) -o $@ $(OBJS)

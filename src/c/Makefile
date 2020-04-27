CC = gcc
CFLAGS = -Wall -std=gnu99
LD = gcc
LDFLAGS = -g -std=gnu99
LDLIBS = 

DEBUGFLAG = 0

SRCS = veml6030-cli.c veml6030.c
OBJS = veml6030-cli.o veml6030.o
TARGETS = veml6030-cli debug

debug: CFLAGS += -DDEBUG -g

all: ${TARGETS}

veml6030-cli: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

debug: $(OBJS)
	$(LD) -o $@ $^ $(LDLIBS) $(LDFLAGS)

%.o: %.c 
	$(CC) $(CFLAGS) -c $< 

%.d: %.c
	gcc -MM -MF $@ $<

-include $(SRCS:.c=.d)

.PHONY: clean
clean:
	rm -f *~ *.d *.o $(TARGETS) 

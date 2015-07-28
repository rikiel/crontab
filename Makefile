#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard.elias@matfyz.cz>
#

SHELL		= /bin/bash
CC		= gcc
CFLAGS		= -Wall -g -c
LDFLAGS		= -lpthread

TARGET		= mycrontab
SOURCES		= conf.c crontab.c main.c utils.c logger.c
OBJECTS		= $(SOURCES:.c=.o)

MAKEDEPENDENCY	= $(CC) -MM -E
GETDEPENDENCY	= sed 's/^.*://'

ARGS		= --debug --log-to=$(TARGET).log crontab
run: $(TARGET)
	./$(TARGET) $(ARGS)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

debug: build
	gdb $(TARGET)

# prerekvizity
conf.o: conf.c conf.h utils.h logger.h
crontab.o: crontab.c crontab.h conf.h utils.h logger.h
logger.o: logger.c logger.h utils.h conf.h
main.o: main.c crontab.h utils.h logger.h
utils.o: utils.c utils.h logger.h

## NEFUNGUJE na solarise...
#conf.o:	$(shell $(MAKEDEPENDENCY) conf.c	| $(GETDEPENDENCY))
#crontab.o:	$(shell $(MAKEDEPENDENCY) crontab.c	| $(GETDEPENDENCY))
#logger.o:	$(shell $(MAKEDEPENDENCY) logger.c	| $(GETDEPENDENCY))
#main.o:	$(shell $(MAKEDEPENDENCY) main.c	| $(GETDEPENDENCY))
#utils.o:	$(shell $(MAKEDEPENDENCY) utils.c	| $(GETDEPENDENCY))

# spolocna kompilacia pre .c subory
%.o:
	$(CC) $(CFLAGS) $(@:.o=.c) -o $@

cstyle:
	./cstyle.pl *.[ch]

clean:
	rm -f *.o *.log $(TARGET)

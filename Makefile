#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard.elias@matfyz.cz>
#

TARGET		= mycrontab
SOURCES		= conf.c crontab.c logger.c main.c utils.c
OBJECTS		= $(SOURCES:.c=.o)

MAKEDEPENDENCY	= $(CC) -MM -E
GETDEPENDENCY	= sed 's/^.*://'

ARGS		= --debug --log-to=$(TARGET).log crontab
run: $(TARGET)
	./$(TARGET) $(ARGS)
tests: run

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

debug: build
	gdb $(TARGET)

conf.o: conf.c conf.h utils.h logger.h
crontab.o: crontab.c crontab.h conf.h utils.h logger.h
logger.o: logger.c logger.h utils.h conf.h
main.o: main.c crontab.h utils.h logger.h
utils.o: utils.c utils.h logger.h

# compilation:
%.o:
	$(CC) -c $(CFLAGS) $(@:.o=.c) -o $@

cstyle:
	./cstyle.pl *.[ch]

clean:
	rm -f *.o *.log $(TARGET)

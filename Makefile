#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard.elias@matfyz.cz>
#

TARGET		= mycrontab
SOURCES		= conf.c crontab.c logger.c main.c utils.c
OBJECTS		= $(SOURCES:.c=.o)

ARGS		= --debug --log-to=$(TARGET).log crontab
run: $(TARGET)
	./$(TARGET) $(ARGS)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

build: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

debug: build
	gdb $(TARGET)

# prerequisites:
conf.o: conf.c conf.h logger.h utils.h
crontab.o: crontab.c crontab.h conf.h logger.h utils.h
logger.o: logger.c logger.h conf.h utils.h
main.o: main.c crontab.h utils.h logger.h
utils.o: utils.c utils.h logger.h

# compilation:
%.o:
	$(CC) -c $(CFLAGS) $(@:.o=.c) -o $@

cstyle:
	./cstyle.pl *.[ch]

clean:
	rm -f *.o *.log *.lock $(TARGET)

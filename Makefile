#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard.elias@matfyz.cz>
#

SHELL		= /bin/bash
CC		= gcc
CXX		= g++
CFLAGS		= -Wall -g -c
LDFLAGS		= $($(SHELL) pkg-config --libs log4cpp)

TARGET		= mycrontab
SOURCES		= conf.c crontab.c main.c utils.c logger.cpp
OBJECTS		= $(shell echo $(SOURCES) | sed 's/\.\(c\|cpp\)/.o/g')

ARGS		= --debug --log-to=$(TARGET).log crontab
run: $(TARGET)
	./$(TARGET) $(ARGS)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

build: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) -o $(TARGET)

# prerekvizity
crontab.o: crontab.c crontab.h conf.h utils.h logger.hpp
conf.o: conf.c conf.h utils.h logger.hpp
utils.o: utils.c utils.h conf.h logger.hpp
main.o: main.c utils.h crontab.h

# kompilacia len pre logger
logger.o: logger.cpp logger.hpp
	$(CXX) $(CFLAGS) $< -o $@

# spolocna kompilacia pre .c subory
%.o:
	$(CC) $(CFLAGS) $(@:.o=.c) -o $@

cstyle:
	./cstyle.pl *.[ch] *.[ch]pp

clean:
	rm -f *.o *.log $(TARGET)

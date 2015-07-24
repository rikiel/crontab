#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard@ba30.eu>
#


SHELL			= bash
CC				= gcc
CXX				= g++
CFLAGS			= -std=c11 -c -Wall -g
CPPFLAGS		= -std=c++11 -c -Wall -g
LFLAGS			= $(shell pkg-config --libs log4cpp)

BUILDDIR		= build
TARGET			= mycrontab
SOURCES			= $(shell find ./ -name "*.c" | sed 's@\./@@g')
HEADERS			= $(shell find ./ -name "*.h" | sed 's@\./@@g')
OBJECTS			= $(SOURCES:%.c=%.o) logger.o
MAKEFILES		= $(OBJECTS:%.o=%.mk)

MAKEDEPENDENCY	= $(CC) -MM -E 

KILLTIME = 1

run: build
	@echo "**********************************************************************"
	@echo "********************* RUNNING  PROGRAM *******************************"
	@echo "**********************************************************************"
	@echo
	sleep $(KILLTIME) && killall $(TARGET) 2> /dev/null &
	$(BUILDDIR)/$(TARGET)


build: $(BUILDDIR)/$(TARGET)

debug: build
	gdb $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(BUILDDIR)/Makefile

$(BUILDDIR)/Makefile: $(SOURCES) $(HEADERS) $(addprefix $(BUILDDIR)/,$(MAKEFILES))
	@rm -f $@
	@echo -e\
			\\nbuild : $(TARGET) \\n \
			\\n \
			\\n$(TARGET) : $(OBJECTS) \\n\\t \
			@echo "****************LINKING:****************" \\n\\t \
			@echo \\n\\t \
			$(CXX) $(LFLAGS) $(OBJECTS) -o $(TARGET) \\n\\t \
			@echo \\n\\t \
			@echo "**************END-LINKING:**************" \\n\\t \
			\\n \
			\\ninclude $(MAKEFILES) \
					>> $@
	@make -j4 --directory=$(BUILDDIR) -f Makefile

$(BUILDDIR)/%.mk: %.c
	@mkdir -p $(BUILDDIR)
	@rm -f $@
	@$(MAKEDEPENDENCY) $(CFLAGS) $^ | sed 's@\([^. :]*.\.\([hc]pp\|[hc]\)\)@../\1@g' \
					>> $@
	@echo '	$(CC) $(CFLAGS) $$< -o $$@ ' \
					>> $@

$(BUILDDIR)/%.mk: %.cpp
	@mkdir -p $(BUILDDIR)
	@rm -f $@
	@$(MAKEDEPENDENCY) $(CPPFLAGS) $^ | sed 's@\([^. :]*.\.\([hc]pp\|[hc]\)\)@../\1@g' \
					>> $@
	@echo '	$(CXX) $(CPPFLAGS) $$< -o $$@ ' \
					>> $@


clean:
	rm -rf $(BUILDDIR)

cleanMK:
	rm -f $(BUILDDIR)/*.mk Makefile

.PHONY: $(BUILDDIR)/Makefile clean cleanMK



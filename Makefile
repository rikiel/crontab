#
# File: Makefile
# Created: 2015-02-15
# By: Richard Eliáš <richard@ba30.eu>
#


CC				= g++
CFLAGS			= -std=c++11 -c -Wall
LFLAGS			= -std=c++11 -Wall $(shell pkg-config --libs log4cpp)
SHELL			= /bin/bash

BUILDDIR		= build

TARGET			= program
SOURCES			= $(shell find ./ -name "*.c" | sed 's@\./@@g')
HEADERS			= $(shell find ./ -name "*.h" -or -name "*.hpp" | sed 's@\./@@g')
OBJECTS			= $(SOURCES:%.c=%.o) logger.o
MAKEFILES		= $(OBJECTS:%.o=%.mk)

MAKEDEPENDENCY	= $(CC) -MM -E 


run: build
	@echo "**********************************************************************"
	@echo "********************* RUNNING  PROGRAM *******************************"
	@echo "**********************************************************************"
	@echo
	sleep 1 && killall $(TARGET) &
	$(BUILDDIR)/$(TARGET)


build: $(BUILDDIR)/$(TARGET)


$(BUILDDIR)/$(TARGET): $(BUILDDIR)/Makefile

# je to .PHONY target, vykona sa vzdy:
$(BUILDDIR)/Makefile: $(SOURCES) $(HEADERS) $(addprefix $(BUILDDIR)/,$(MAKEFILES))
	@rm -f $@
	@echo -e\
			\\nbuild : $(TARGET) \\n \
			\\n \
			\\n$(TARGET) : $(OBJECTS) \\n\\t \
			@echo "****************LINKING:****************" \\n\\t \
			@echo \\n\\t \
			$(CC) $(LFLAGS) $(OBJECTS) -o $(TARGET) \\n\\t \
			@echo \\n\\t \
			@echo "**************END-LINKING:**************" \\n\\t \
			\\n \
			\\ninclude $(MAKEFILES) \
					>> $@
	@make --directory=$(BUILDDIR) -f Makefile

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
	@$(MAKEDEPENDENCY) $(CFLAGS) $^ | sed 's@\([^. :]*.\.\([hc]pp\|[hc]\)\)@../\1@g' \
					>> $@
	@echo '	$(CC) $(CFLAGS) $$< -o $$@ ' \
					>> $@


clean:
	rm -rf $(BUILDDIR)

cleanMK:
	rm -f $(BUILDDIR)/*.mk Makefile

.PHONY: $(BUILDDIR)/Makefile clean cleanMK



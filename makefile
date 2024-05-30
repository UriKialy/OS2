# This Makefile is used to build and clean multiple subdirectories.
# It provides targets for building all subdirectories, cleaning all subdirectories,
# and running programs in the "q1" subdirectory.

SUBDIRS := $(wildcard */)

.PHONY: all $(SUBDIRS)

# Target to build all subdirectories
all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

.PHONY: clean

# Target to clean all subdirectories
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# Target to run programs in the "q1" subdirectory
run-q1:
	@echo "Running programs in directory q1"
	@$(MAKE) -C q1/

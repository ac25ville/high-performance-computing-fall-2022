SUBDIRS = src

all: subdirs
	@echo "Build Complete"

subdirs:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -r ./.vscode core.* *.csv
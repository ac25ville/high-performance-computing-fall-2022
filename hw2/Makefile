all: src
	@echo "Still a work in progress, please use the make file in src"

src:
	@cd src && make

clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
	rm -r ./.vscode
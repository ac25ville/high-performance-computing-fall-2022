SUBDIRS = src
SHELL:=/bin/bash

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

max_parallel = 8
c = 10000
analysis: all
	@cd src/ ;
	@rm analysis*
	@echo "Analysis Beginning" ;
	@n=50 ; seed_file="../coordCSVs/"$$n"_Coord.csv"; while [[ $$n -le 200 ]] ; do \
		# ((seed_file = "" + )) ; \
		echo $$n ; \
		p=2 ; while [[ $$p -le ${max_parallel} ]] ; do \
			mpirun -np $$p "src/homework4" $$seed_file $$n ${c} $$p >> analysis.csv ; \
			((p = p+1)) ; \
			pid=$! ; \
			wait $$pid ; \
		done ; \
		((n = n+50)) ; \
	done
	@cd ../ ;
	@echo "Analysis Complete" ;
info:
	@echo "Main targets (cvode, cvodes, ida, idas, kinsol):"
	@echo "  all               compiles all tests without running them"
	@echo "  tests.byte.log    run standard tests (byte code)"
	@echo "  tests.opt.log     run standard tests (native code)"
	@echo "  perf.byte.log     time standard tests (byte code)"
	@echo "  perf.opt.log      time standard tests (native code)"
	@echo "  ocaml             compile the other ocaml examples without running them"

include ../config

C_EXAMPLES=$(if $(EXAMPLESROOT),,c_examples_unavailable)
c_examples_unavailable:
	@echo "C version of examples not found.  Try running ../configure again"
	@echo "with EXAMPLES=/path/to/sundials/examples.  You can give the examples"
	@echo "directory in the sundials C source tree or (if you installed the C library's"
	@echo "examples) the one in the installation."
	@false


# Every conditional in this function should be prefixed with $(or
# $(1), ...), so that if $(1) is nonempty we get the list of
# absolutely all subdirectories including disabled ones, and if $(1)
# is empty we get the list of enabled subdirectories.  Per-example
# filtering is handled in examples.mk.
GEN_SUBDIRS=$(foreach m,cvode cvodes ida idas kinsol,			\
	       $(foreach v,						\
	            serial						\
	            $(if $(or $(1),$(MPI_ENABLED)),parallel),		\
	          $m/$v))						\
	    $(if $(or $(1),$(ARKODE_ENABLED)),				\
	       $(foreach v,						\
	             C_serial						\
	             $(if $(or $(1),$(MPI_ENABLED)),C_parallel)		\
	             $(if $(or $(1),$(OPENMP_ENABLED)),C_openmp),	\
	           arkode/$v))						\
	    $(if $(or $(1),$(AT_LEAST_2_6)),				\
	       $(foreach v,						\
	             serial						\
	             $(if $(or $(1),$(MPI_ENABLED)),parallel)		\
	             $(if $(or $(1),$(OPENMP_ENABLED)),openmp)		\
	             $(if $(or $(1),$(PTHREADS_ENABLED)),pthreads),	\
	           nvector/$v))

ALL_SUBDIRS=$(call GEN_SUBDIRS,all)  # All subdirectories
SUBDIRS=$(call GEN_SUBDIRS)          # Just the enabled ones.

# Nvector examples are unsuitable for performance comparisons, so
# filter them out unless PERF_FORCE_NVECTORS is nonempty.
PERF_SUBDIRS=$(if $(PERF_FORCE_NVECTORS),$(SUBDIRS),\
		  $(filter-out nvector/%,$(SUBDIRS)))


TOP_TARGETS = tests.byte.log tests.opt.log perf.byte.log perf.opt.log reps
SUBDIR_TARGETS = $(foreach d,$(SUBDIRS),$(foreach t,$(TOP_TARGETS),$d/$t))

.PHONY: default tests.byte.log tests.opt.log tests.byte tests.opt ocaml \
	perf.byte.log perf.opt.log $(SUBDIR_TARGETS)

all: ocaml tests.byte tests.opt

$(SUBDIR_TARGETS):
	@$(MAKE) -C `echo $@ | sed -e 's#/[^/]*$$##'` \
		    `echo $@ | sed -e 's#.*/##'`

# For each subdirectory foo (for example, foo=cvode/serial), don't
# give up immediately if foo/tests.opt.log fails, provided
# foo/tests.opt.log is not one of the targets specified on the command
# line.
define ADD_IGNORE_RULE
    ifeq (,$(findstring $1,$(MAKECMDGOALS)))
    .IGNORE: $1
    endif
endef
$(foreach t,$(SUBDIR_TARGETS),$(eval $(call ADD_IGNORE_RULE,$t)))

tests.opt tests.byte: tests.%: $(foreach s,$(SUBDIRS),$s/tests.%)

tests.opt.log tests.byte.log: tests.%.log: $(C_EXAMPLES) \
					$(foreach s,$(SUBDIRS),$s/tests.%.log)
	@echo ======================================================================
	@echo 'Summary (each should be 0):'
	@for s in ${SUBDIRS}; do		\
	    grep "^ *[0-9]* $$s" $$s/$@;	\
	done
	@! for s in ${SUBDIRS}; do		\
	    grep "^ *[0-9]* $$s" $$s/$@;	\
	done | grep -vq '^ *0'

UTILS=utils

$(UTILS)/perf: $(UTILS)/perf.ml
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -o $@ unix.cmxa $<

$(UTILS)/crunchperf: $(UTILS)/crunchperf.ml
	$(OCAMLOPT) $(OCAMLOPTFLAGS) -o $@ str.cmxa unix.cmxa $<

perf.opt.log perf.byte.log: perf.%.log: $(C_EXAMPLES)			   \
				$(UTILS)/perf $(UTILS)/crunchperf	   \
				$(foreach s,$(PERF_SUBDIRS),$s/perf.%.log)
	@echo ======================================================================
	@$(UTILS)/crunchperf -m $(foreach s,$(PERF_SUBDIRS),$s/$@) > $@
	$(UTILS)/crunchperf -s $@
	@$(if $(findstring $@,$(MAKECMDGOALS)), \
	   echo "Note: \"make $(@:.log=.plot)\" can plot this for you.")

reps: $(foreach s,$(SUBDIRS),$s/reps)

C_TITLE=C ($(CC) $(filter-out -I% -DNDEBUG=1,$(CFLAGS)))
NATIVE_TITLE='OCaml ($(OCAML_VERSION)) native code performance over $(C_TITLE)'
BYTE_TITLE  ='OCaml ($(OCAML_VERSION)) byte code performance over $(C_TITLE)'

PLOTTYPES=jpg png pdf eps

perf.opt.plot: perf.opt.log
	TITLE=$(NATIVE_TITLE) utils/plot.sh $<
	@$(UTILS)/plot.sh --explain-vars

perf.byte.plot: perf.byte.log
	TITLE=$(BYTE_TITLE) utils/plot.sh $<
	@$(UTILS)/plot.sh --explain-vars

$(foreach t,$(PLOTTYPES),perf.opt.$t): perf.opt.log
	TITLE=$(NATIVE_TITLE)				 \
	    TERMINAL="$(subst perf.opt.,,$@) noenhanced" \
	    OUTPUT=$@ $(UTILS)/plot.sh $<
	@printf "\nPlot saved in %s.\n" "$@"
	@$(UTILS)/plot.sh --explain-vars

$(foreach t,$(PLOTTYPES),perf.byte.$t): perf.byte.log
	TITLE=$(BYTE_TITLE) \
	    TERMINAL="$(subst perf.byte.,,$@) noenhanced"	  \
	    OUTPUT=$@ $(UTILS)/plot.sh $<
	@printf "\nPlot saved in %s.\n" "$@"
	@$(UTILS)/plot.sh --explain-vars

ocaml:
	${MAKE} -C ocaml

distclean: clean-utils
	@for s in ${ALL_SUBDIRS}; do		\
		${MAKE} -C $$s distclean;	\
	done
	@${MAKE} -C ocaml distclean

clean: clean-utils
	@for s in ${ALL_SUBDIRS}; do		\
		${MAKE} -C $$s clean;		\
	done
	@${MAKE} -C ocaml clean

clean-utils:
	-@rm -f perf.byte.* perf.opt.*
	-@rm -f $(foreach f,utils/perf utils/crunchperf,\
		    $f $f.cmi $f.cmx $f.cmo $f.o)

# Invoke `make' with USELIB=sundials to run the tests with the
# sensitivity-agnostic subset of CVODES.  Note that memory usage
# statistics will differ from the versions in sundials/C, unless those
# are recompiled to link against CVODES.
USELIB ?= sundials_no_sens
SRCROOT=../../..
SUBDIR=arkode/C_serial

EXAMPLES = ark_KrylovDemo_prec.ml \
	   ark_analytic.ml \
	   ark_analytic_nonlin.ml \
	   ark_brusselator.ml \
	   ark_brusselator_fp.ml \
	   ark_brusselator1D.ml \
	   ark_heat1D.ml \
	   ark_heat1D_adapt.ml \
	   ark_robertson.ml \
	   ark_robertson_root.ml

LAPACK_EXAMPLES =

KLU_EXAMPLES =
SUPERLUMT_EXAMPLES =

include ../../examples.mk

#TODO:
#$(eval $(call EXECUTION_RULE,cvRoberts_dns_uw_alt,$$<,cvRoberts_dns_uw))
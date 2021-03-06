/***********************************************************************
 *                                                                     *
 *                   OCaml interface to Sundials                       *
 *                                                                     *
 *             Timothy Bourke, Jun Inoue, and Marc Pouzet              *
 *             (Inria/ENS)     (Inria/ENS)    (UPMC/ENS/Inria)         *
 *                                                                     *
 *  Copyright 2015 Institut National de Recherche en Informatique et   *
 *  en Automatique.  All rights reserved.  This file is distributed    *
 *  under a New BSD License, refer to the file LICENSE.                *
 *                                                                     *
 ***********************************************************************/

#include <caml/mlvalues.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/fail.h>

#include "../sundials/sundials_ml.h"

#ifndef SUNDIALS_ML_KLU
CAMLprim value c_arkode_klu_init (value varkode_mem, value vneqs, value vnnz)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_klu_set_ordering (value varkode_mem, value vordering)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_klu_reinit (value varkode_mem, value vn, value vnnz,
				    value vrealloc)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_klu_get_num_jac_evals(value varkode_mem)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_mass_klu_init (value varkode_mem, value vneqs,
				       value vnnz)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_mass_klu_set_ordering (value varkode_mem,
					       value vordering)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_mass_klu_reinit (value varkode_mem,
					 value vn, value vnnz, value vrealloc)
{ CAMLparam0(); CAMLreturn (Val_unit); }

CAMLprim value c_arkode_klu_get_num_mass_evals(value varkode_mem)
{ CAMLparam0(); CAMLreturn (Val_unit); }
#else
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#include "arkode_ml.h"
#include "../lsolvers/sls_ml.h"

#include <arkode/arkode.h>
#include <arkode/arkode_sparse.h>
#include <arkode/arkode_klu.h>

enum arkode_klu_ordering_tag {
  VARIANT_ARKODE_KLU_AMD     = 0,
  VARIANT_ARKODE_KLU_COLAMD  = 1,
  VARIANT_ARKODE_KLU_NATURAL = 2,
};

static int jacfn(realtype t,
		 N_Vector y,
		 N_Vector fy,	     
		 SlsMat Jac,
		 void *user_data,
		 N_Vector tmp1,
		 N_Vector tmp2,
		 N_Vector tmp3)
{
    CAMLparam0();
    CAMLlocalN (args, 2);
    CAMLlocal3(session, cb, smat);

    WEAK_DEREF (session, *(value*)user_data);
    args[0] = arkode_make_jac_arg (t, y, fy,
				  arkode_make_triple_tmp (tmp1, tmp2, tmp3));

    cb = ARKODE_LS_CALLBACKS_FROM_ML(session);
    cb = Field (cb, 0);
    smat = Field(cb, 1);
    if (smat == Val_none) {
	Store_some(smat, c_sls_sparse_wrap(Jac, 0));
	Store_field(cb, 1, smat);
	args[1] = Some_val(smat);
    } else {
	args[1] = Some_val(smat);
	c_sparsematrix_realloc(args[1], 0);
    }

    /* NB: Don't trigger GC while processing this return value!  */
    value r = caml_callback2_exn (Field(cb, 0), args[0], args[1]);

    CAMLreturnT(int, CHECK_EXCEPTION(session, r, RECOVERABLE));
}

CAMLprim value c_arkode_klu_init (value varkode_mem, value vneqs, value vnnz)
{
    CAMLparam3(varkode_mem, vneqs, vnnz);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);
    int flag;

    flag = ARKKLU (arkode_mem, Int_val(vneqs), Int_val(vnnz));
    CHECK_FLAG ("ARKKLU", flag);
    flag = ARKSlsSetSparseJacFn(arkode_mem, jacfn);
    CHECK_FLAG("ARKSlsSetSparseJacFn", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_klu_set_ordering (value varkode_mem, value vordering)
{
    CAMLparam2(varkode_mem, vordering);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    int flag = ARKKLUSetOrdering (arkode_mem, Int_val(vordering));
    CHECK_FLAG ("ARKKLUSetOrdering", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_klu_reinit (value varkode_mem, value vn, value vnnz,
				   value vrealloc)
{
    CAMLparam4(varkode_mem, vn, vnnz, vrealloc);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    int flag = ARKKLUReInit (arkode_mem,
			     Int_val(vn),
			     Int_val(vnnz),
			     Bool_val(vrealloc) ? 1 : 2);
    CHECK_FLAG ("ARKKLUReInit", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_klu_get_num_jac_evals(value varkode_mem)
{
    CAMLparam1(varkode_mem);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    long int r;
    int flag = ARKSlsGetNumJacEvals(arkode_mem, &r);
    CHECK_FLAG("ARKSlsGetNumJacEvals", flag);

    CAMLreturn(Val_long(r));
}

/* Mass matrix rouintes */

static int massfn(realtype t,
		  SlsMat M,
		  void *user_data,
		  N_Vector tmp1,
		  N_Vector tmp2,
		  N_Vector tmp3)
{
    CAMLparam0();
    CAMLlocalN (args, 3);
    CAMLlocal3(session, cb, smat);

    WEAK_DEREF (session, *(value*)user_data);
    args[0] = caml_copy_double(t);
    args[1] = arkode_make_triple_tmp (tmp1, tmp2, tmp3);

    cb = ARKODE_MASS_CALLBACKS_FROM_ML(session);
    cb = Field (cb, 0);
    smat = Field(cb, 1);
    if (smat == Val_none) {
	Store_some(smat, c_sls_sparse_wrap(M, 0));
	Store_field(cb, 1, smat);
	args[2] = Some_val(smat);
    } else {
	args[2] = Some_val(smat);
	c_sparsematrix_realloc(args[2], 0);
    }

    /* NB: Don't trigger GC while processing this return value!  */
    value r = caml_callback3_exn (Field(cb, 0), args[0], args[1], args[2]);

    CAMLreturnT(int, CHECK_EXCEPTION(session, r, RECOVERABLE));
}

CAMLprim value c_arkode_mass_klu_init (value varkode_mem, value vneqs, value vnnz)
{
    CAMLparam3(varkode_mem, vneqs, vnnz);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);
    int flag;

    flag = ARKMassKLU (arkode_mem, Int_val(vneqs), Int_val(vnnz), massfn);
    CHECK_FLAG ("ARKMassKLU", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_mass_klu_set_ordering (value varkode_mem,
					       value vordering)
{
    CAMLparam2(varkode_mem, vordering);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    int flag = ARKMassKLUSetOrdering (arkode_mem, Int_val(vordering));
    CHECK_FLAG ("ARKMassKLUSetOrdering", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_mass_klu_reinit (value varkode_mem,
					 value vn, value vnnz, value vrealloc)
{
    CAMLparam4(varkode_mem, vn, vnnz, vrealloc);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    int flag = ARKMassKLUReInit (arkode_mem,
				 Int_val(vn),
				 Int_val(vnnz),
				 Bool_val(vrealloc) ? 1 : 2);
    CHECK_FLAG ("ARKMassKLUReInit", flag);

    CAMLreturn (Val_unit);
}

CAMLprim value c_arkode_klu_get_num_mass_evals(value varkode_mem)
{
    CAMLparam1(varkode_mem);
    void *arkode_mem = ARKODE_MEM_FROM_ML (varkode_mem);

    long int r;
    int flag = ARKSlsGetNumMassEvals(arkode_mem, &r);
    CHECK_FLAG("ARKSlsGetNumMassEvals", flag);

    CAMLreturn(Val_long(r));
}

#endif

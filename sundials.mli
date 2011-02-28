(***********************************************************************)
(*                                                                     *)
(*              Ocaml interface to Sundials CVODE solver               *)
(*                                                                     *)
(*       Timothy Bourke (INRIA Rennes) and Marc Pouzet (LIENS)         *)
(*                                                                     *)
(*  Copyright 2011 Institut National de Recherche en Informatique et   *)
(*  en Automatique.  All rights reserved.  This file is distributed    *)
(*  under the terms of the GNU Library General Public License, with    *)
(*  the special exception on linking described in file LICENSE.        *)
(*                                                                     *)
(***********************************************************************)

(** Generic definitions for the Sundials suite

 @version VERSION()
 @author Timothy Bourke (INRIA)
 @author Marc Pouzet (LIENS)
 *)

(** {2 Constants} *)

(** The BIG_REAL constant.
    @cvode <node5#s:types> Data Types
 *)
val big_real : float

(** The UNIT_ROUNDOFF constant.
    @cvode <node5#s:types> Data Types
 *)
val unit_roundoff : float

(** {2 Arrays of floats} *)

(** A {{:OCAML_DOC_ROOT(Bigarray.Array1)} (Bigarray)} vector of floats. *)
type real_array =
  (float, Bigarray.float64_elt, Bigarray.c_layout) Bigarray.Array1.t

(** [make_real_array n] returns a {!real_array} with [n] elements. *)
val make_real_array : int -> real_array

(** A {{:OCAML_DOC_ROOT(Bigarray.Array2)} (Bigarray)} two-dimensional array of
   floats. *)
type real_array2 =
  (float, Bigarray.float64_elt, Bigarray.c_layout) Bigarray.Array2.t

(** [make_real_array2 m n] returns a {!real_array2} with [m] x [n]  elements. *)
val make_real_array2 : int -> int -> real_array2

(** Utility functions for serial nvectors as used in {!Cvode_serial}. *)
module Carray :
  sig
    type t = real_array

    (** An array with 0 elements. *)
    val empty : t

    (** [create n] returns an array with [n] elements. *)
    val create : int -> t

    (** Copies the contents of an {{:OCAML_DOC_ROOT(Array)} Array} into a
       {!real_array} *)
    val of_array : float array -> t

    (** [fill a c] sets all elements of the array [a] to the constant [c]. *)
    val fill : t -> float -> unit

    (** Returns the length of an array *)
    val length : t -> int

    (** [print_with_time t a] prints a line containing the current time (see
        {!print_time}) followed by a tab-delimited list of the values of [a],
        using the format [% e], and then a newline. *)
    val print_with_time : float -> t -> unit

    (** [print_with_time' t a] prints a line containing the current time (see
        {!print_time}) followed by a tab-delimited list of the values of [a],
        using the format [% .8f], and then a newline. *)
    val print_with_time' : float -> t -> unit

    (** [app f a] applies [f] to the values of each element in [a]. *)
    val app : (float -> unit) -> t -> unit

    (** [app f a] applies [f] to the indexes and values of each element
        in [a]. *)
    val appi : (int -> float -> unit) -> t -> unit

    (** [map f a] applies [f] to the value of each element in [a] and
        stores the result back into the same element. *)
    val map : (float -> float) -> t -> unit

    (** [map f a] applies [f] to the index and value of each element
        in [a] and stores the result back into the same element. *)
    val mapi : (int -> float -> float) -> t -> unit
  end

(** {2 Arrays of ints} *)

(** A {{:OCAML_DOC_ROOT(Bigarray.Array1)} (Bigarray)} vector of integers. *)
type int_array =
  (int32, Bigarray.int32_elt, Bigarray.c_layout) Bigarray.Array1.t

(** [make_int_array n] returns an {!int_array} with [n] elements. *)
val make_int_array  : int -> int_array

(** {2 Arrays of roots (zero-crossings)} *)

(** Utility functions for arrays of roots (zero-crossings). *)
module Roots :
  sig
    type t = int_array
    type val_array = Carray.t

    val empty : t
    val create : int -> t
    val length : t -> int

    val print : t -> unit
    val print' : t -> unit

    val get : t -> int -> bool
    val get' : t -> int -> int

    val set : t -> int -> bool -> unit

    val reset : t -> unit
    val exists : t -> bool

    val app : (bool -> unit) -> t -> unit
    val appi : (int -> bool -> unit) -> t -> unit
  end

(** {2 Miscellaneous utility functions} *)

(** [print_time (s1, s2) t] prints [t] with [s1] on the left and [s2] on the
    right.  *)
val print_time : string * string -> float -> unit

(** Controls the precision of {!print_time}.
 
    If [true] the format [%.15e] is used, otherwise [%e]
    (the default) is used. *)
val extra_time_precision : bool ref

(** [format_float fmt f] formats [f] according to the format string [fmt],
    using the low-level [caml_format_float] function. *)
val format_float : string -> float -> string

(** Equivalent to [format_float "%a"].
  
    [floata f] returns the bit-level representation of [f] in
    hexadecimal as a string. *)
val floata : float -> string

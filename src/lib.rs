// This software may be modified and distributed under the terms
// of the Apache-2.0 license. See the LICENSE file for details.

// Originally developed (as a fork) in https://github.com/paritytech/reed-solomon-novelpoly
// which is implementation of Novel Polynomial Basis and Its Application to Reed-Solomon Erasure Codes
// https://arxiv.org/abs/1404.3458

#![warn(missing_docs)] // refuse to compile if documentation is missing
#![warn(rust_2018_compatibility)]
#![warn(rust_2018_idioms)]
// for enum variants
#![allow(unused_variables)]
#![allow(non_snake_case)]
#![warn(future_incompatible)]
#![feature(proc_macro_is_available)]

//!
//! Glue code to generate C headers for Novel Polynomial Basis and Its Application to Reed-Solomon Erasure Codes
//!

/// Glue code for erasure_coding
pub mod erasure_coding;

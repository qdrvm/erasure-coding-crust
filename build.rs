use cbindgen::Config;
use std::env;

fn main() {
    let crate_dir = env::var("CARGO_MANIFEST_DIR").unwrap();

    cbindgen::Builder::new()
        .with_crate(crate_dir)
        .with_config(Config::from_file("./cbindgen.toml").expect("Parsing config failed"))
        .generate()
        .expect("Unable to generate bindings")
        .write_to_file("./include/erasure_coding/erasure_coding.h");
}

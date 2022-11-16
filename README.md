# Erasure-coding-crust

C bindings over RUST implementations of [reed-solomon-novelpoly](https://github.com/paritytech/reed-solomon-novelpoly) that implements [Novel Polynomial Basis and Its Application to
Reed-Solomon Erasure Codes](https://www.citi.sinica.edu.tw/papers/whc/4454-F.pdf).

## Build

1. Install rust compiler (nightly), `cargo`:
    ```bash
    curl https://sh.rustup.rs -sSf | sh -s -- -y --default-toolchain nightly
    source $HOME/.cargo/env
    rustup install nightly
    rustup default nightly
    ```
2. `mkdir build && cd build`
3. Options;
   - `-DTESTING=[ON|OFF]` - enable or disable build of tests.
   - `-DCMAKE_BUILD_TYPE=[Release|Debug]` - select build type.
   - `-DBUILD_SHARED_LIBS=[TRUE|FALSE]` - build shared/static library. 
   
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```
4. Build and install library: 
   ```
   sudo make install
   ```

## Docs

Header with comments will be generated in `build/include/erasure_coding/erasure_coding.h`.

## Examples

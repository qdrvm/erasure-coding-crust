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
   - `-DBENCHMARK=[ON|OFF]` - build benchmark/benchmark project
   
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```
4. Build and install library: 
   ```
   sudo make install
   ```

## Docs

Header with comments will be generated in `build/include/erasure_coding/erasure_coding.h`.

## Performance

AMD Ryzen 5950x, 32Gb RAM, under VM
```
~~~ [ Benchmark case: 15 bytes ] ~~~
Encode RUST (100 cycles): 4 us
Decode RUST (100 cycles): 78.775 ms
Encode C++ (100 cycles): 4 us
Decode C++ (100 cycles): 41.968 ms

~~~ [ Benchmark case: 300 bytes ] ~~~
Encode RUST (100 cycles): 735 us
Decode RUST (100 cycles): 80.181 ms
Encode C++ (100 cycles): 277 us
Decode C++ (100 cycles): 42.517 ms

~~~ [ Benchmark case: 5000 bytes ] ~~~
Encode RUST (100 cycles): 12.815 ms
Decode RUST (100 cycles): 97.648 ms
Encode C++ (100 cycles): 4782 us
Decode C++ (100 cycles): 51.095 ms

~~~ [ Benchmark case: 100000 bytes ] ~~~
Encode RUST (100 cycles): 251.146 ms
Decode RUST (100 cycles): 422.756 ms
Encode C++ (100 cycles): 100.051 ms
Decode C++ (100 cycles): 220.24 ms

~~~ [ Benchmark case: 1000000 bytes ] ~~~
Encode RUST (100 cycles): 2526.03 ms
Decode RUST (100 cycles): 3501.28 ms
Encode C++ (100 cycles): 987.493 ms
Decode C++ (100 cycles): 1770.36 ms

~~~ [ Benchmark case: 10000000 bytes ] ~~~
Encode RUST (100 cycles): 25.1685 s
Decode RUST (100 cycles): 34.4632 s
Encode C++ (100 cycles): 9.71128 s
Decode C++ (100 cycles): 17.5619 s
```

## Examples

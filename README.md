# YAML to JSON with Wasm

This project helps convert YAML strings to JSON strings using WebAssembly. This speeds up data format conversion in environments with limited access to external libraries (e.g. Snowflake UDF).

Under the hood, this project relies on [Rapid YAML](https://github.com/biojppm/rapidyaml) (or `ryml` for short) for parsing YAML and generating JSON. After including `ryml` headers in C++ source files, the project is compiled with [emscripten](https://emscripten.org/) into a single-file JavaScript source encapsulating [WebAssembly](https://webassembly.org/) (Wasm). The core functionality is executed in Wasm, the JavaScript wrapper marshals types (e.g. JavaScript strings to C strings), and caches initialization for environments in which the code may be re-entered (e.g. Snowflake JavaScript UDF). Snowflake UDF templates are provided in `src/template/`.

## Comparison

To assess the performance of cross-compiling a high-speed YAML parser to Wasm, and embedding in a Snowflake user-defined function (UDF), we compare the following approaches:

* Python implementation. Wraps [PyYAML](https://github.com/yaml/pyyaml), which is a full-featured YAML processing framework for Python. The parser is imported as a package, and invoked in a handler function. The handler takes a YAML string and returns a JSON object. We validate whether we can serialize the JSON object to a JSON string with built-in `json.dumps`.
* Pure JavaScript implementation. Wraps [js-yaml](nodeca.github.io/js-yaml/), which is a fast YAML parser and dumper in JavaScript. The parser code is embedded inline in a Snowflake JavaScript UDF, which takes and returns a `VARCHAR`.
* Wasm implementation with `VARCHAR` input and output. Wraps [Rapid YAML](https://github.com/biojppm/rapidyaml). The parser code is cross-compiled to Wasm with emscripten with single-file output, and the JavaScript code is embedded inline in a Snowflake JavaScript UDF. The UDF takes a `VARCHAR` parameter as input, and returns a `VARCHAR` result as output.
* Wasm implementation with `BINARY` input and output. Identical to the previous approach but takes a `BINARY` parameter as input (string encoded in UTF-8), and returns `BINARY` as output. `BINARY` is converted to `VARCHAR` in Snowflake with the function `TO_VARCHAR` and parsed into a `VARIANT` with `PARSE_JSON`.

The following table shows execution times of converting 100,000 records of YAML strings (stored as `VARCHAR` in Snowflake) into JSON stored as `VARIANT`, measured on a Snowflake *x-small* warehouse.

| Approach          | Time (s) |
| ----------------- | -------- |
| Python            |       64 |
| pure JavaScript   |       53 |
| Wasm with VARCHAR |       45 |
| Wasm with BINARY  |       18 |

## Contributing

Compiling this project requires [emscripten](https://emscripten.org/), the LLVM-based compiler to produce Wasm. Detailed instructions on how to install `emcc`/`em++` are available in the [Getting started](https://emscripten.org/docs/getting_started/downloads.html) guide. We use *Installation instructions using the emsdk (recommended)*.

The project also depends on [Rapid YAML](https://github.com/biojppm/rapidyaml) but the amalgamated header file `src/ryml_all.hpp` has been pushed to the repository. This can be re-generated by running a Python script in the cloned `rapidyaml` repository:

```
python3 ./tools/amalgamate.py
```

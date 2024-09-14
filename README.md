# YAML to JSON with Wasm

This project helps convert YAML strings to JSON strings using WebAssembly. This speeds up data format conversion in environments with limited access to external libraries (e.g. Snowflake UDF).

Under the hood, this project relies on [Rapid YAML](https://github.com/biojppm/rapidyaml) (or `ryml` for short) for parsing YAML and generating JSON. After including `ryml` headers in C++ source files, the project is compiled with [emscripten](https://emscripten.org/) into a single-file JavaScript source encapsulating [WebAssembly](https://webassembly.org/) (Wasm). The core functionality is executed in Wasm, the JavaScript wrapper marshals types (e.g. JavaScript strings to C strings), and caches initialization for environments in which the code may be re-entered (e.g. Snowflake JavaScript UDF). A Snowflake UDF template is provided in `template.sql`.

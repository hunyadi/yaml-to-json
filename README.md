# YAML to JSON with Wasm

This project helps convert YAML strings to JSON strings using WebAssembly. This speeds up data format conversion in environments with limited access to external libraries (e.g. Snowflake UDF).

Under the hood, this project relies on [Rapid YAML](https://github.com/biojppm/rapidyaml) (or `ryml` for short) for parsing YAML and generating JSON. After including `ryml` headers in C++ source files, the project is compiled with [Emscripten](https://emscripten.org/) into a single-file JavaScript source encapsulating [WebAssembly](https://webassembly.org/) (Wasm). The core functionality is executed in Wasm, the JavaScript wrapper marshals types (e.g. JavaScript strings to C strings), and caches initialization for environments in which the code may be re-entered (e.g. Snowflake JavaScript UDF). Snowflake UDF templates are provided in `src/template/`.

## Comparison

To assess the performance of cross-compiling a high-speed YAML parser to Wasm, and embedding in a Snowflake user-defined function (UDF), we compare the following approaches:

* Python implementation. Wraps [PyYAML](https://github.com/yaml/pyyaml), which is a full-featured YAML processing framework for Python. The parser is imported as a package, and invoked in a handler function. The handler takes a YAML string and returns a JSON object. We validate whether we can serialize the JSON object to a JSON string with built-in `json.dumps`.
* Pure JavaScript implementation. Wraps [js-yaml](nodeca.github.io/js-yaml/), which is a fast YAML parser and dumper in JavaScript. The parser code is embedded inline in a Snowflake JavaScript UDF, which takes and returns a `VARCHAR`.
* Wasm implementation with `VARCHAR` input and output. Wraps [Rapid YAML](https://github.com/biojppm/rapidyaml). The parser code is cross-compiled to Wasm with Emscripten with single-file output, and the JavaScript code is embedded inline in a Snowflake JavaScript UDF. The UDF takes a `VARCHAR` parameter as input, and returns a `VARCHAR` result as output.
* Wasm implementation with `BINARY` input and output. Identical to the previous approach but takes a `BINARY` parameter as input (string encoded in UTF-8), and returns `BINARY` as output. `BINARY` is converted to `VARCHAR` in Snowflake with the function `TO_VARCHAR` and parsed into a `VARIANT` with `PARSE_JSON`.

The following table shows execution times of converting 100,000 records of YAML strings (stored as `VARCHAR` in Snowflake) into JSON stored as `VARIANT`, measured on a Snowflake *x-small* warehouse.

| Approach          | Time (s) |
| ----------------- | -------- |
| Python            |       64 |
| pure JavaScript   |       53 |
| Wasm with VARCHAR |       45 |
| Wasm with BINARY  |       20 |

## Design considerations

Snowflake JavaScript UDF doesn't support loading code from an external stage, and is restricted to inline functions. We compile with the `emcc` option `SINGLE_FILE` to overcome this limitation. This produces a single JavaScript file rather than separate `*.js` and `*.wasm` files, where the former would load the latter. Likewise, we turn off asynchronous compilation because the Snowflake environment is synchronous.

The restricted JavaScript environment in Snowflake UDF lacks classes and functions like `TextEncoder`, `TextDecoder` and `atob`. However, Emscripten embeds Wasm code in JavaScript encoded with Base64, and attempts to invoke the above classes and functions, which ultimately leads to an exception. We provide an implementation of `atob`, which takes a Base64-encoded string, and produces a string of raw bytes, thus lifting the restriction in Snowflake.

As shown by performance measurements, Wasm with `BINARY` as input and output is more efficient than `VARCHAR`. We receive a `Uint8Array` from Snowflake, which we can directly set in `Module.HEAPU8`. (`Module.HEAPU8` represents heap memory in Wasm with byte-aligned access.) Similarly, we return a `Uint8Array` to Snowflake, which we have obtained by slicing `Module.HEAPU8`. With `VARCHAR`, we would have to do our own char-to-byte and byte-to-char conversion in high-level JavaScript, involving Emscripten utility library functions `lengthBytesUTF8`, `stringToUTF8` and `UTF8ToString`.

Unfortunately, we typically receive `VARCHAR` as input and output. Thus, we use the conversion function `TO_BINARY` to encode YAML input strings to UTF-8 on input prior to invoking `yaml_to_json_array`. Likewise, we use `TO_VARCHAR` to decode UTF-8 on output to get a JSON string. Occasionally, the YAML input string may contain escaped characters like `\x97`. `\x97` is the en-dash character as per the character set *windows-1250* but it is not a correctly encoded UTF-8 sequence. (Instead, the YAML string should use (verbatim) `—` or (escaped) `\u2014` to represent this character.) Rapid YAML interprets `\x97` at face value, which in turn leads to an invalid UTF-8 string on output. `TO_VARCHAR` in Snowflake is sensitive to errors, the entire batch fails as opposed to the returning `NULL` on encoding errors. As a work-around, we implement [UTF-8 validation](https://bjoern.hoehrmann.de/utf-8/decoder/dfa/) in Wasm, and make the UDF return `NULL` when it would produce an invalid UTF-8 string.

The YAML-to-JSON conversion function is designed to be resilient to errors. When malformed input is received, Rapid YAML triggers a parser error, which calls the error handler function. Normally, this would terminate the Wasm process with `abort`, or raise an exception. We prefer not to rely on catching `abort` in JavaScript as doing so may mask other types of critical errors. Catching exceptions without Wasm exception support, however, is relatively expensive. As a compromise solution, we use `setjmp` in the main transformation function to save the calling environment, and invoke `longjmp` when a parser error occurs.

The body of JavaScript UDFs is re-entered by Snowflake. To avoid re-parsing Wasm code and re-initializing Wasm state each time the UDF is called, we maintain state in a global variable, and elide initialization if the variable is already set.

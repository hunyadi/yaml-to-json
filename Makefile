all: dist/yaml_to_json_array.sql dist/yaml_to_json_string.sql

EXPORTED_FUNCTIONS = _string_create,_string_delete,_string_data,_string_length,_transform

EXPORTED_RUNTIME_METHODS = stringToUTF8,UTF8ToString,lengthBytesUTF8,HEAPU8

CXX_HEADERS = src/string.hpp src/utf8.hpp src/ryml_all.hpp

CXX_SOURCES = src/yaml_to_json.cpp src/ryaml.cpp

dist/yaml_to_json.js: src/wasm_helpers.js ${CXX_SOURCES} ${CXX_HEADERS}
	em++ -Oz -flto --no-entry \
		-DNDEBUG \
		-s STRICT \
		-s EXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_METHODS} \
		-s FILESYSTEM=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CXX_SOURCES}

dist/yaml_to_json.txt: dist/yaml_to_json.wasm
	base64 -i $< -o $@

dist/yaml_to_json_array.sql: src/template/binary.sql src/base64.js dist/yaml_to_json.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json.js > $@

dist/yaml_to_json_string.sql: src/template/varchar.sql src/base64.js dist/yaml_to_json.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json.js > $@

.PHONY: clean
clean:
	rm -f dist/*.js
	rm -f dist/*.sql
	rm -f dist/*.wasm
	rm -f dist/*.txt

.PHONY: all
all: dist/yaml_to_json_array.sql dist/yaml_to_json_string.sql

EXCEPTION_CATCHING_ALLOWED = transform
EXPORTED_FUNCTIONS = _string_create,_string_delete,_string_data,_string_length,_transform

EXPORTED_RUNTIME_FOR_ARRAY = HEAPU8
EXPORTED_RUNTIME_FOR_STRING = stringToUTF8,UTF8ToString,lengthBytesUTF8

CXX_HEADERS = src/ryml_all.hpp src/string.hpp src/utf8.hpp
CXX_SOURCES = src/ryml_all.cpp src/yaml_to_json.cpp

dist/yaml_to_json_array.js: src/wrapper/array.js ${CXX_SOURCES} ${CXX_HEADERS}
	em++ -Oz -fexceptions -flto --no-entry \
		-D NDEBUG \
		-D RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS \
		-s STRICT \
		-s EXCEPTION_CATCHING_ALLOWED=${EXCEPTION_CATCHING_ALLOWED} \
		-s EXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_FOR_ARRAY} \
		-s FILESYSTEM=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CXX_SOURCES}

dist/yaml_to_json_string.js: src/wrapper/string.js ${CXX_SOURCES} ${CXX_HEADERS}
	em++ -Oz -fexceptions -flto --no-entry \
		-D NDEBUG \
		-D RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS \
		-s STRICT \
		-s EXCEPTION_CATCHING_ALLOWED=${EXCEPTION_CATCHING_ALLOWED} \
		-s EXPORTED_FUNCTIONS=${EXPORTED_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_FOR_STRING} \
		-s FILESYSTEM=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CXX_SOURCES}

dist/yaml_to_json.txt: dist/yaml_to_json.wasm
	base64 -i $< -o $@

dist/yaml_to_json_array.sql: src/template/array.sql src/base64.js dist/yaml_to_json_array.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json_array.js > $@

dist/yaml_to_json_string.sql: src/template/string.sql src/base64.js dist/yaml_to_json_string.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json_string.js > $@

.PHONY: clean
clean:
	rm -f dist/*.js
	rm -f dist/*.sql
	rm -f dist/*.wasm
	rm -f dist/*.txt

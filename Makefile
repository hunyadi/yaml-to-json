.PHONY: all
all: dist/check_yaml.sql dist/yaml_to_json_array.sql dist/yaml_to_json_string.sql

EXPORTED_FUNCTIONS = _main,_string_create,_string_delete,_string_data,_string_length
CHECK_FUNCTIONS = ${EXPORTED_FUNCTIONS},_check_yaml
CONVERT_FUNCTIONS = ${EXPORTED_FUNCTIONS},_transform_yaml

EXPORTED_RUNTIME_FOR_ARRAY = HEAPU8
EXPORTED_RUNTIME_FOR_STRING = stringToUTF8,UTF8ToString,lengthBytesUTF8

CXX_HEADERS = src/ryml_all.hpp src/string.hpp src/utf8.hpp
CXX_SOURCES = src/ryml_all.cpp src/string.cpp
CHECK_SOURCES = ${CXX_SOURCES} src/check_yaml.cpp
CONVERT_SOURCES = ${CXX_SOURCES} src/yaml_to_json.cpp

dist/check_yaml.js: src/wrapper/check_yaml.js ${CHECK_SOURCES} ${CXX_HEADERS}
	em++ -Oz -flto \
		-D NDEBUG \
		-D RYML_NO_DEFAULT_CALLBACKS \
		-s STRICT \
		-s EXPORTED_FUNCTIONS=${CHECK_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_FOR_ARRAY} \
		-s FILESYSTEM=0 -s IGNORE_MISSING_MAIN=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CHECK_SOURCES}

dist/yaml_to_json_array.js: src/wrapper/yaml_to_json_array.js ${CONVERT_SOURCES} ${CXX_HEADERS}
	em++ -Oz -flto \
		-D NDEBUG \
		-D RYML_NO_DEFAULT_CALLBACKS \
		-s STRICT \
		-s EXPORTED_FUNCTIONS=${CONVERT_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_FOR_ARRAY} \
		-s FILESYSTEM=0 -s IGNORE_MISSING_MAIN=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CONVERT_SOURCES}

dist/yaml_to_json_string.js: src/wrapper/yaml_to_json_string.js ${CONVERT_SOURCES} ${CXX_HEADERS}
	em++ -Oz -flto \
		-D NDEBUG \
		-D RYML_NO_DEFAULT_CALLBACKS \
		-s STRICT \
		-s EXPORTED_FUNCTIONS=${CONVERT_FUNCTIONS} \
		-s EXPORTED_RUNTIME_METHODS=${EXPORTED_RUNTIME_FOR_STRING} \
		-s FILESYSTEM=0 -s IGNORE_MISSING_MAIN=0 -s WASM=1 -s WASM_ASYNC_COMPILATION=0 -s SINGLE_FILE=1 \
		-o $@ \
		--post-js $< \
		${CONVERT_SOURCES}

dist/yaml_to_json.txt: dist/yaml_to_json.wasm
	base64 -i $< -o $@

dist/check_yaml.sql: src/template/check_yaml.sql src/base64.js dist/check_yaml.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/check_yaml.js > $@

dist/yaml_to_json_array.sql: src/template/yaml_to_json_array.sql src/base64.js dist/yaml_to_json_array.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json_array.js > $@

dist/yaml_to_json_string.sql: src/template/yaml_to_json_string.sql src/base64.js dist/yaml_to_json_string.js
	python src/replace.py $< "@@BASE64_DECODER@@" src/base64.js "@@EMSCRIPTEN_OUTPUT@@" dist/yaml_to_json_string.js > $@

.PHONY: clean
clean:
	rm -f dist/*.js
	rm -f dist/*.sql
	rm -f dist/*.wasm
	rm -f dist/*.txt

CREATE OR REPLACE FUNCTION
  YAML_TO_JSON_ARRAY(YAML_ARRAY BINARY)
  RETURNS BINARY
  LANGUAGE JAVASCRIPT
  RETURNS NULL ON NULL INPUT
  IMMUTABLE
  COMMENT = 'Converts a YAML binary string encoded in UTF-8 to a JSON binary string also encoded in UTF-8.'
AS
$$
@@BASE64_DECODER@@

function setup(Module) {
@@EMSCRIPTEN_OUTPUT@@
}

if (typeof(Module) === "undefined") {
  Module = {};
  setup(Module);
}

return Module.yaml_to_json_array(YAML_ARRAY);
$$;

CREATE OR REPLACE FUNCTION
  YAML_TO_JSON(YAML_STRING VARCHAR)
  RETURNS VARIANT
  LANGUAGE SQL
  COMMENT = 'Parses a YAML string into a semi-structured value.'
AS
$$
  PARSE_JSON(TO_VARCHAR(YAML_TO_JSON_ARRAY(TO_BINARY(YAML_STRING, 'UTF-8')), 'UTF-8'))
$$

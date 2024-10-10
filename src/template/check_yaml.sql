CREATE OR REPLACE FUNCTION
  CHECK_YAML_ARRAY(YAML_ARRAY BINARY)
  RETURNS BINARY
  LANGUAGE JAVASCRIPT
  RETURNS NULL ON NULL INPUT
  IMMUTABLE
  COMMENT = 'Parses and validates a YAML binary string encoded in UTF-8.'
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

return Module.check_yaml(YAML_ARRAY);
$$;

CREATE OR REPLACE FUNCTION
  CHECK_YAML(YAML_STRING VARCHAR)
  RETURNS VARCHAR
  LANGUAGE SQL
  COMMENT = 'Parses and validates a YAML string.'
AS
$$
  TO_VARCHAR(CHECK_YAML_ARRAY(TO_BINARY(YAML_STRING, 'UTF-8')), 'UTF-8')
$$

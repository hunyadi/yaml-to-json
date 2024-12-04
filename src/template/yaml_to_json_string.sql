--
-- Converts YAML to JSON with Wasm.
--
-- Copyright 2024, Levente Hunyadi
-- https://github.com/hunyadi/yaml-to-json

CREATE OR REPLACE FUNCTION
  YAML_TO_JSON_STRING(YAML_STRING VARCHAR)
  RETURNS VARCHAR
  LANGUAGE JAVASCRIPT
  RETURNS NULL ON NULL INPUT
  IMMUTABLE
  COMMENT = 'Converts a YAML string to a JSON string.'
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

return Module.yaml_to_json_string(YAML_STRING);
$$;

CREATE OR REPLACE FUNCTION
  YAML_TO_JSON(YAML_STRING VARCHAR)
  RETURNS VARIANT
  LANGUAGE SQL
  COMMENT = 'Parses a YAML string into a semi-structured value.'
AS
$$
CASE WHEN YAML_STRING IS NULL THEN NULL ELSE PARSE_JSON(YAML_TO_JSON_STRING(YAML_STRING)) END
$$

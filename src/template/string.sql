CREATE OR REPLACE FUNCTION
  yaml_to_json_string(yaml_string VARCHAR)
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

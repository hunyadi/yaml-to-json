/**
 * Converts YAML to JSON with Wasm.
 *
 * @param {string} yaml The YAML string to parse.
 * @returns {string | null} The JSON string generated.
 */
function yaml_to_json_string(yaml) {
    const yaml_length = lengthBytesUTF8(yaml);
    const yaml_string = _string_create(yaml_length);
    try {
        const yaml_buffer = _string_data(yaml_string);
        stringToUTF8(yaml, yaml_buffer, yaml_length + 1);

        const json_string = _transform_yaml(yaml_string);
        if (!json_string) {
            return null;
        }
        try {
            const json_length = _string_length(json_string);
            const json_buffer = _string_data(json_string);
            return UTF8ToString(json_buffer, json_length);
        } finally {
            _string_delete(json_string);
        }
    } finally {
        _string_delete(yaml_string);
    }
}
Module["yaml_to_json_string"] = yaml_to_json_string;

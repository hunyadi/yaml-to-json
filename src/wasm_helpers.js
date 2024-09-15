/**
 * Converts YAML to JSON with Wasm.
 *
 * @param {Uint8Array} yaml The YAML string to parse.
 * @returns {Uint8Array} The JSON string generated.
 */
function yaml_to_json_array(yaml) {
    const yaml_string = _string_create(yaml.length);
    try {
        const yaml_buffer = _string_data(yaml_string);
        Module.HEAPU8.set(yaml, yaml_buffer);
        const json_string = _transform(yaml_string);
        if (!json_string) {
            return null;
        }
        try {
            const json_length = _string_length(json_string);
            const json_buffer = _string_data(json_string);
            return Module.HEAPU8.slice(json_buffer, json_buffer + json_length);
        } finally {
            _string_delete(json_string);
        }
    } finally {
        _string_delete(yaml_string);
    }
}
Module["yaml_to_json_array"] = yaml_to_json_array;

/**
 * Converts YAML to JSON with Wasm.
 *
 * @param {string} yaml The YAML string to parse.
 * @returns {string} The JSON string generated.
 */
function yaml_to_json_string(yaml) {
    const yaml_length = lengthBytesUTF8(yaml);
    const yaml_string = _string_create(yaml_length);
    try {
        const yaml_buffer = _string_data(yaml_string);
        stringToUTF8(yaml, yaml_buffer, yaml_length + 1);

        const json_string = _transform(yaml_string);
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

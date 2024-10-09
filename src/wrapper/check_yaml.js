/**
 * Parses and validates YAML with Wasm.
 *
 * @param {Uint8Array} yaml The YAML string to parse and validate.
 * @returns {Uint8Array | null} The parse or validation error emitted.
 */
function check_yaml(yaml) {
    const yaml_string = _string_create(yaml.length);
    try {
        const yaml_buffer = _string_data(yaml_string);
        Module.HEAPU8.set(yaml, yaml_buffer);
        const message = _check_yaml(yaml_string);
        if (!message) {
            return null;
        }
        try {
            const message_length = _string_length(message);
            const message_buffer = _string_data(message);
            return Module.HEAPU8.slice(message_buffer, message_buffer + message_length);
        } finally {
            _string_delete(message);
        }
    } finally {
        _string_delete(yaml_string);
    }
}
Module["check_yaml"] = check_yaml;

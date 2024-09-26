const { yaml_to_json_array } = require('./dist/yaml_to_json_array.js');
const { yaml_to_json_string } = require('./dist/yaml_to_json_string.js');
const { atob } = require('./src/base64.js')

// "árvíztűrő türökfúrógép" stored as UTF-8, prints with multi-byte characters
console.log(
  new TextDecoder("utf-8").decode(
    Uint8Array.from(
      atob('w6FydsOtenTFsXLFkSB0w7xrw7ZyZsO6csOzZ8OpcA=='),
      c => c.charCodeAt(0)
    )
  )
);

// a simple YAML string
console.log(yaml_to_json_string('{foo: 1, bar: [2, 3], john: doe}'));

// an invalid YAML string
console.log(yaml_to_json_string('{}0'));

// a complex YAML string
const yaml = String.raw`
en: Planet (Gas)
fr: Planète (Gazeuse)
ru: Планета (Газ)
ja: 惑星（ガス）
zh: 行星（气体）
# UTF8 decoding only happens in double-quoted strings,
# as per the YAML standard
decode this: "\u263A \xE2\x98\xBA"
and this as well: "\u2705 \U0001D11E"
not decoded: '\u263A \xE2\x98\xBA'
neither this: '\u2705 \U0001D11E'
`;
console.log(yaml_to_json_string(yaml));
console.log(new TextDecoder("utf-8").decode(yaml_to_json_array(new TextEncoder("utf-8").encode(yaml))));

// a complex YAML string with the document start marker and non-specific tags (!ruby)
const y = `--- !ruby/hash:ActiveSupport::HashWithIndifferentAccess
id:
regrade_option: ''
points_possible: 1.0
correct_comments: ''
incorrect_comments: ''
neutral_comments: ''
correct_comments_html: ''
incorrect_comments_html: ''
neutral_comments_html: ''
question_type: multiple_choice_question
question_name: Question
name: Question
question_text: "<p>Inca used an innovative farming method to overcome difficulties
  presented by living in the mountains. This technique was called:</p>"
answers:
- !ruby/hash:ActiveSupport::HashWithIndifferentAccess
  id: 1000
  text: terraced agriculture
  html: ''
  comments: ''
  comments_html: ''
  weight: 100.0
- !ruby/hash:ActiveSupport::HashWithIndifferentAccess
  id: 2000
  text: slash & burn agriculture
  html: ''
  comments: ''
  comments_html: ''
  weight: 0.0
- !ruby/hash:ActiveSupport::HashWithIndifferentAccess
  id: 3000
  text: chinampas farming
  html: ''
  comments: ''
  comments_html: ''
  weight: 0.0
- !ruby/hash:ActiveSupport::HashWithIndifferentAccess
  id: 4000
  text: crop rotation
  html: ''
  comments: ''
  comments_html: ''
  weight: 0.0
text_after_answers: ''
assessment_question_id:
`;
console.log(yaml_to_json_string(y));
console.log(new TextDecoder("utf-8").decode(yaml_to_json_array(new TextEncoder("utf-8").encode(y))));

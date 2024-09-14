"""
Substitutes placeholder strings in a file with the contents of other files.
"""

import sys
from pathlib import Path

def replace(template: Path, replacements: dict[str, Path]):
    """
    Substitutes placeholder strings in a file with the contents of other files.

    :param template: The file in which to look for placeholders.
    :param replacements: Maps a string to replace to the file whose contents to substitute.
    """

    with open(template, "r") as f:
        content = f.read()

    for placeholder, replacement in replacements.items():
        with open(replacement, "r") as f:
            content = content.replace(placeholder, f.read())

    return content

if __name__ == "__main__":
    template = Path(sys.argv[1])
    replacements: dict[str, Path] = {}
    for index in range(2, len(sys.argv), 2):
        placeholder = sys.argv[index]
        replacement = Path(sys.argv[index + 1])
        replacements[placeholder] = replacement

    print(replace(template, replacements))

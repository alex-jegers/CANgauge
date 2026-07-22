#!/usr/bin/env python3
"""

THIS FILE WAS AUTOMATICALLY GENERATED

bump_version.py
 
Pre-build script for STM32CubeIDE.
 
Finds the `#define VERSION "..."` line in version.h, increments the
LAST dot-separated number in the version core by 1 (the
"sub-sub-version" component, e.g. the 3rd number in v1.2.3), and
stamps the current date/time onto the end of the string.
 
Result format:  "vX.Y.Z mm/dd/yy hh:mmAM/PM"
    e.g.        "v1.3.5 07/19/26 02:32PM"
 
Any existing date/time stamp on the line is discarded and replaced
with a fresh one each run, so re-running the build doesn't pile up
multiple timestamps.
 
Usage:
    python bump_version.py
    python bump_version.py "C:\\path\\to\\version.h"
 
If no path is given, VERSION_FILE below is used.
 
STM32CubeIDE setup (Project > Properties > C/C++ Build > Settings >
Build Steps > Pre-build steps), example command:
 
    python "C:\\Users\\awjpp\\LocalDocuments\\CANgaugeSrc\\CANgauge\\application\\bump_version.py"
 
(Put the script itself somewhere convenient - it doesn't have to live
next to version.h.)
"""
 
import re
import sys
from datetime import datetime
from pathlib import Path
 
# Default location of version.h - used if no command-line arg is given.
VERSION_FILE = r"C:\Users\awjpp\LocalDocuments\CANgaugeSrc\CANgauge\application\version.h"
 
# Matches lines like:
#   #define VERSION     "v1.3.4"
#   #define VERSION     "v1.3.4 07/18/26 09:15AM"   (already stamped - old stamp is discarded)
#   #define VERSION     "1.3.4"
#
# Captures:
#   prefix   - everything up to and including the opening quote
#   vprefix  - optional letter prefix like 'v'
#   version  - the dotted version number, e.g. 1.3.4
#   after    - everything after the closing quote (usually just the rest of the line)
# Anything between the version number and the closing quote (e.g. an old
# date/time stamp) is matched but discarded/regenerated.
VERSION_LINE_RE = re.compile(
    r'^(?P<prefix>\s*#define\s+VERSION\s+")'
    r'(?P<vprefix>[A-Za-z]*)'
    r'(?P<version>\d+(?:\.\d+)+)'
    r'[^"]*"'
    r'(?P<after>.*)$'
)
 
 
def bump_version_string(version_str: str) -> str:
    """Increment the last dot-separated number in a version string."""
    parts = version_str.split(".")
    parts[-1] = str(int(parts[-1]) + 1)
    return ".".join(parts)
 
 
def main():
    path_arg = sys.argv[1] if len(sys.argv) > 1 else VERSION_FILE
    version_path = Path(path_arg)
 
    if not version_path.is_file():
        print(f"ERROR: version.h not found at: {version_path}")
        sys.exit(1)
 
    lines = version_path.read_text(encoding="utf-8").splitlines(keepends=True)
 
    updated = False
    new_lines = []
    old_version = new_version = None
    vprefix = ""
    timestamp = datetime.now().strftime("%m/%d/%y %I:%M%p")
 
    for line in lines:
        # splitlines(keepends=True) keeps the newline char(s) in `line`,
        # so match against the line stripped of its trailing newline.
        stripped = line.rstrip("\r\n")
        newline = line[len(stripped):]  # preserves \n or \r\n or none (last line)
 
        match = VERSION_LINE_RE.match(stripped)
        if match and not updated:
            old_version = match.group("version")
            vprefix = match.group("vprefix")
            new_version = bump_version_string(old_version)
            new_line = (
                match.group("prefix")
                + vprefix
                + new_version
                + " "
                + timestamp
                + '"'
                + match.group("after")
                + newline
            )
            new_lines.append(new_line)
            updated = True
        else:
            new_lines.append(line)
 
    if not updated:
        print("ERROR: Could not find a line matching '#define VERSION \"X.Y.Z\"' "
              "(or similar) in version.h. No changes made.")
        sys.exit(1)
 
    version_path.write_text("".join(new_lines), encoding="utf-8")
    print(f"Version bumped: {vprefix}{old_version} -> {vprefix}{new_version} {timestamp}")
 
 
if __name__ == "__main__":
    main()
#
#    LibZenit
#    Copyright (C) 2026  Ian Torres
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU Affero General Public License version 3
#    as published by the Free Software Foundation.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Affero General Public License for more details.
#
#    You should have received a copy of the GNU Affero General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

import hashlib
import os
import sys
from pathlib import Path

def main():
    prefix = Path(sys.argv[1]) if len(sys.argv) > 1 else Path.cwd()
    lib_dir = prefix / "lib"
    if not lib_dir.exists():
        lib_dir = prefix

    for f in sorted(lib_dir.iterdir()):
        if f.suffix in (".a", ".lib", ".dylib", ".so", ".dll"):
            sha = hashlib.sha256(f.read_bytes()).hexdigest()
            (prefix / "checksum.txt").write_text(sha + "\n")
            print(f"SHA256 ({f.name}): {sha}")
            return

    print("No library file found")
    sys.exit(1)

if __name__ == "__main__":
    main()

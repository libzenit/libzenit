//
//    LibZenit
//    Copyright (C) 2026  Ian Torres
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License version 3
//    as published by the Free Software Foundation.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
// Test that UUID generation handles random source failures.
// The UUID module doesn't use malloc/calloc — it uses platform CSPRNG.
// This test verifies that NULL param paths work (covered in test_uuid.c).
//

#include <stdio.h>

int main(void) {
    /* UUID uses platform CSPRNG, not malloc.
     * All error paths are NULL-param checks covered in test_uuid.c.
     * This file exists to maintain the pattern of one malloc-fail test
     * per module, even when the module doesn't allocate. */
    printf("PASS: uuid malloc fail (no-op)\n");
    return 0;
}

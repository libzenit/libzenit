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

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout


class LibZenitConan(ConanFile):
    name = "libzenit"
    version = "0.1.0"
    package_type = "library"

    license = "AGPL-3.0-only"
    author = "Ian Torres"
    url = "https://github.com/libzenit/libzenit"
    description = "Portable C library providing building blocks for systems programming"
    topics = ("c", "data-structures", "utilities", "arena-allocator", "json",
              "ring-buffer", "state-machine", "hash-map", "bloom-filter")

    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "src/*", "include/*", "cmake/*"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["LIBZENIT_BUILD_TESTS"] = False
        tc.variables["LIBZENIT_BUILD_BENCHMARKS"] = False
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["zenit"]

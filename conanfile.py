from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain, cmake_layout


class SetUp(ConanFile):
    settings = "os", "arch", "compiler", "build_type"
    options = { "sanitizer": ["asan", "tsan", "msan", "ubsan"] }
    default_options = { "sanitizer": "asan" }

    def configure(self):
        # DEBUG
        if self.settings.build_type == "Debug":
            print("Configuring DEBUG build.")

            # ENABLE ALL WARNINGS
            warning_flags = [
                "-Wall", "-Wextra", "-Wpedantic", "-Wshadow", "-Wnon-virtual-dtor",
                "-Wcast-align", "-Wunused", "-Woverloaded-virtual","-Wdouble-promotion",
                "-Wconversion", "-Wmisleading-indentation",
            ]

            # SELECT SANITIZER
            sanitizer_flags = []
            if self.options.sanitizer == "asan":
                sanitizer_flags = ["-fsanitize=address", "-fsanitize=leak"]
            elif self.options.sanitizer == "tsan":
                sanitizer_flags = ["-fsanitize=thread"]
            elif self.options.sanitizer == "msan":
                sanitizer_flags = ["-fsanitize=memory"]
            elif self.options.sanitizer == "ubsan":
                sanitizer_flags = ["-fsanitize=undefined"]

            debug_flags = warning_flags + sanitizer_flags
            debug_flags.extend(["-O0", "-g3", "-fno-omit-frame-pointer"])
            
            self.conf.define("tools.build:cxxflags", debug_flags)
            self.conf.define("tools.build:cflags", debug_flags)
            self.conf.define("tools.build:sharedlinkflags", sanitizer_flags)
            self.conf.define("tools.build:exelinkflags", sanitizer_flags)

        # RELEASE
        elif self.settings.build_type == "Release":
            print("Configuring RELEASE build.")
            release_flags = ["-O3", "-DNDEBUG", "-march=native"]
            self.conf.define("tools.build:cxxflags", release_flags)
            self.conf.define("tools.build:cflags", release_flags)

    
    def requirements(self):
        self.requires("gtest/1.17.0")
        self.requires("benchmark/1.9.4")
    
    def generate(self):
        cmake = CMakeToolchain(self)
        cmake.generator = "Ninja"
        cmake.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
        cmake_layout(self)
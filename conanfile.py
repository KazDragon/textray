from conans import ConanFile, CMake, tools


class TerminalppConan(ConanFile):
    name = "munin-acceptance"
    version = "1.3.0"
    license = "MIT"
    author = "KazDragon"
    url = "https://github.com/KazDragon/terminalpp"
    description = "A C++ library for interacting with ANSI terminal windows"
    topics = ("terminal-emulators", "ansi-escape-codes")
    settings = "os", "compiler", "build_type", "arch"
    exports = "*"
    options = {"shared": [True, False], "withTests": [True, False]}
    default_options = {"shared": False, "withTests": False}
    requires = ("telnetpp/[>=2.0.0]@kazdragon/conan-public",
                "munin/[>=0.3.1]@kazdragon/conan-public",
                "terminalpp/[>=1.3.0]@kazdragon/conan-public",
                "serverpp/[>=0.0.1]@kazdragon/conan-public",
                "boost_program_options/[>=1.69]@bincrafters/stable")
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.definitions["BUILD_SHARED_LIBS"] = self.options.shared
        cmake.configure()
        cmake.build()

    def package(self):
        self.copy("*.hpp", dst="include", src="include")
        self.copy("*hello.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["terminalpp"]


from conans import ConanFile, CMake, tools


class TextratConan(ConanFile):
    name = "textray"
    license = "MIT"
    author = "KazDragon"
    url = "https://github.com/KazDragon/textray"
    description = "A Telnet server test bed for Munin, implementing a text-based FPS view"
    topics = ("terminal-emulators", "ansi-escape-codes")
    settings = "os", "compiler", "build_type", "arch"
    exports = "*"
    options = {"shared": [True, False], "withTests": [True, False]}
    default_options = {"shared": False, "withTests": False}
    requires = ("serverpp/[>=0.0.7]@kazdragon/conan-public",
                "telnetpp/[>=2.1.2]@kazdragon/conan-public",
                "terminalpp/[>=2.0.1]@kazdragon/conan-public",
                "munin/[>=0.3.10]@kazdragon/conan-public",
                "boost/[>=1.69]")
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
        self.cpp_info.libs = ["textray"]


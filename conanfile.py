from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import get
from conan.tools.scm import Git

class DeskPetRecipe(ConanFile):
    name = "deskpet"
    package_type = "library"

    # Optional metadata
    license = ""
    author = "Hyfloac mail@hyfloac.com"
    url = "https://github.com/hyfloac/DeskPet"
    description = ""

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = { "shared": [True, False] }
    default_options = { "shared": False }

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "SysLib/*", "PetAI/*"

    def set_version(self):
        self.version = self.conan_data["latest"];

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if not self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["deskpet"]
        if self.options.shared:
            self.cpp_info.libs = ["DeskPetShared"]
            self.cpp_info.set_property("cmake_target_name", "deskpet::PetAIShared")
        else:
            self.cpp_info.libs = ["DeskPetStatic"]
            self.cpp_info.set_property("cmake_target_name", "deskpet::PetAIStatic")
    

    

    


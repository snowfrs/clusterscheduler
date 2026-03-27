# cmake ... -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake
#   or
# export CMAKE_PROJECT_TOP_LEVEL_INCLUDES=/path/to/file.cmake
#
# will be evaluated before the project() call!
#
# Shows details about std support for gcc versions
# https://gcc.gnu.org/projects/cxx-status.html

# Get some details about the environment 
execute_process(COMMAND hostname OUTPUT_VARIABLE HOSTNAME_FQDN OUTPUT_STRIP_TRAILING_WHITESPACE)
string(REGEX REPLACE "\\..*$" "" HOSTNAME "${HOSTNAME_FQDN}")
# execute_process(COMMAND hostname -s OUTPUT_VARIABLE HOSTNAME OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND ${CMAKE_SOURCE_DIR}/source/dist/util/arch OUTPUT_VARIABLE SGE_ARCH_RAW)
string(STRIP "${SGE_ARCH_RAW}" SGE_ARCH)
set(SGE_ARCH "${SGE_ARCH}" PARENT_SCOPE)
message(STATUS "Toolchain evaluation started")
message(STATUS "Hostname: ${HOSTNAME}")
message(STATUS "Build ID: ${CMAKE_BUILD_ID}")
message(STATUS "Arch:     ${SGE_ARCH}")

set(LINK_CPP_STATICALLY OFF)

if (("${SGE_ARCH}" STREQUAL "ulx-amd64") AND (${HOSTNAME} STREQUAL "ce7-0-ulx-amd64"))
   # Systems compiler provides c++20 partially only
   set(CMAKE_C_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/gcc")
   set(CMAKE_CXX_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/g++")
   set(CMAKE_C_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   set(CMAKE_CXX_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   message(STATUS "Selected: C++ v15.2 (GCC); binutils v2.45 => c++23 fully supported and stable")

   set(LINK_CPP_STATICALLY ON) # IMPORTANT!
elseif (("${SGE_ARCH}" STREQUAL "lx-amd64") AND (${HOSTNAME} STREQUAL "ce8-0-lx-amd64"))
   # Systems compiler provides c++20 
   set(CMAKE_C_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/gcc")
   set(CMAKE_CXX_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/g++")
   set(CMAKE_C_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   set(CMAKE_CXX_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   message(STATUS "Selected: C++ v15.2 (GCC); binutils v2.45 => c++23 fully supported and stable")
   set(LINK_CPP_STATICALLY ON)
elseif (("${SGE_ARCH}" STREQUAL "xlx-amd64") AND (${HOSTNAME} STREQUAL "ce6-0-xlx-amd64"))
   # Systems compiler provides c++20 partially only
   set(CMAKE_C_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/gcc")
   set(CMAKE_CXX_COMPILER "/tools/PKG/gcc-15.2/${SGE_ARCH}/bin/g++")
   set(CMAKE_C_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   set(CMAKE_CXX_FLAGS_INIT "-B/tools/PKG/binutils-2.45/${SGE_ARCH}/bin")
   message(STATUS "Selected: C++ v15.2 (GCC); binutils v2.45 => c++23 fully supported and stable")

   set(LINK_CPP_STATICALLY ON) # IMPORTANT!
elseif (("${SGE_ARCH}" STREQUAL "lx-riscv64") AND (${HOSTNAME} STREQUAL "su0-0-lx-riscv64"))
   # Default is gcc 15.2.1 => c++23 fully supported and stable
elseif (("${SGE_ARCH}" STREQUAL "lx-arm64") AND (${HOSTNAME} STREQUAL "ce8-3-lx-arm64"))
   # Default is gcc 13.2.1 => c++23 fully supported and stable
elseif (("${SGE_ARCH}" STREQUAL "lx-ppc64le") AND (${HOSTNAME} STREQUAL "al8-1-lx-ppc64le"))
   # Default is gcc 13.2.1 => c++23 fully supported and stable
elseif (("${SGE_ARCH}" STREQUAL "lx-s390x") AND (${HOSTNAME} STREQUAL "al8-2-lx-s390x"))
   # Default is gcc 11.2.1 => c++20 partially 
elseif (("${SGE_ARCH}" STREQUAL "fbsd-amd64") AND (${HOSTNAME} STREQUAL "fr13-0-fbsd-amd64"))
   # Default is clang 17.0.6 => c++23 fully supported and stable 
endif()

if (LINK_CPP_STATICALLY) 
   # IMPORTANT:
   #
   # stdc++ MUST be loaded **statically** otherwise the created libraries/binaries will
   #        ino run on systems with a smaller library version which is the case with
   #        non-os distributed g++.
   # libgcc MUST be loaded **dynamically** because other libraries (like libpthread) are also linked
   #        shared. Handling otherwise would cause different unwinding stacks in an application
   #        that would cause pthread_cancel() or exception handling in C++ to fail.
   #        (see also CS-1894 and CS-1176)
   set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "-static-libstdc++ -shared-libgcc")
   message(STATUS "Linking: static C++ libs but dynamic C lib")
endif()

message(STATUS "Toolchain evaluation finished")


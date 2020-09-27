


include(GNUInstallDirs)

set(KCC_INSTALL_LIBDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR})
set(KCC_INSTALL_BINDIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

set(SYSCONFDIR "/etc" CACHE PATH "Installation directory for configurations")
set(KCC_PLUGIN_DIR ${KCC_INSTALL_LIBDIR}/kiran-cc-daemon)


# Determine the platform.
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  set(OS_MACOSX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  set(OS_LINUX 1)
  set(OS_POSIX 1)
elseif("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
  set(OS_WINDOWS 1)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  # using Clang
  set(COMPILER_CLANG 1)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(COMPILER_GCC 1)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
  set(COMPILER_INTEL 1)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
  set(COMPILER_MSVC 1)
endif()

if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  set(GEN_NINJA 1)
elseif(${CMAKE_GENERATOR} STREQUAL "Unix Makefiles")
  set(GEN_MAKEFILES 1)
endif()

# Determine the build type.
if(NOT CMAKE_BUILD_TYPE AND (GEN_NINJA OR GEN_MAKEFILES))
  # CMAKE_BUILD_TYPE should be specified when using Ninja or Unix Makefiles.
  set(CMAKE_BUILD_TYPE Release)
  message(STATUS "No CMAKE_BUILD_TYPE value selected, using ${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)


# Platform-specific compiler/linker flags.
list(APPEND KSD_COMPILER_FLAGS
  -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
  -fPIC                           # Generate position-independent code for shared libraries
  -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
  -pipe                           # Use pipes rather than temporary files for communication between build stages
  -Wall                           # Enable all warnings
  -Werror                         # Treat warnings as errors
  -Wno-unused-value               # Disable unused value warning
  -Wno-missing-braces             # Disable missing braces warning
  -Wno-deprecated-declarations    # Disable deprecated declarations warning
  )
list(APPEND KSD_COMPILER_FLAGS    # for C++
  #-fno-rtti                       # Disable real-time type information
  -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
  )
if(COMPILER_GCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
  list(APPEND KSD_COMPILER_FLAGS -Wno-format-truncation) # deal with a bug in gcc8
endif()
if(COMPILER_GCC AND STATIC_LINK_CRT)
  list(APPEND KSD_COMPILER_FLAGS
    -static-libstdc++             # Static link
    -static-libgcc
  )
endif()
if(LOW_GLIBC_VERSION)
  list(APPEND KSD_COMPILER_FLAGS
    -Wl,--wrap=memcpy
  )
endif()
list(APPEND KSD_COMPILER_FLAGS_DEBUG
  -O0                             # Disable optimizations
  -g                              # Generate debug information
  )
list(APPEND KSD_COMPILER_FLAGS_RELEASE
  -O3                             # Optimize for maximum speed
  -fdata-sections                 # Enable linker optimizations to improve locality of reference for data sections
  -ffunction-sections             # Enable linker optimizations to improve locality of reference for function sections
  )
if(FORCE_DEBUG_INFO)
  list(APPEND KSD_COMPILER_FLAGS_RELEASE
    -g
  )
endif()
list(APPEND KSD_LINKER_FLAGS
  -fPIC                              # Generate position-independent code for shared libraries
  -Wl,--fatal-warnings               # Treat warnings as errors
  -Wno-deprecated-declarations       # Disable deprecated declarations warning
  #-Wl,-z,noexecstack                # Mark the stack as non-executable (security feature)
  #-Wl,-z,relro                      # Mark relocation sections as read-only (security feature)
  )

list(APPEND KSD_LINKER_FLAGS_RELEASE
  -Wl,-O1                         # Enable linker optimizations
  -Wl,--as-needed                 # Only link libraries that export symbols used by the binary
  -Wl,--gc-sections               # Remove unused code resulting from -fdata-sections and -function-sections
  )
list(APPEND KSD_COMPILER_DEFINES
  _FILE_OFFSET_BITS=64            # Allow the Large File Support (LFS) interface to replace the old interface
  _LINUX                          # IsLinux
  )
list(APPEND KSD_COMPILER_DEFINES_RELEASE
  NDEBUG                          # Not a debug build
  )

include(CheckCXXCompilerFlag)

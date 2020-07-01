


get_property(LIB64 GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS)

if ("${LIB64}" STREQUAL "TRUE")
    set(LIBSUFFIX 64)
else()
    set(LIBSUFFIX "")
endif()

set(LIBDIR  ${CMAKE_INSTALL_PREFIX}/lib${LIBSUFFIX} CACHE PATH "Installation directory for libraries")

set(KSD_PLUGIN_DIR ${LIBDIR}/kiran-session-daemon)
set(KSD_PLUGIN_EXT "ksd-plugin")


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

if(NOT DEFINED PROJECT_ARCH)
  if(CMAKE_SIZEOF_VOID_P MATCHES 8)
    set(PROJECT_ARCH "x86_64")
  else()
    set(PROJECT_ARCH "x86")
  endif()
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

option(STATIC_LINK_CRT "link c++ runtime library statically" OFF)
option(FORCE_DEBUG_INFO "enable debug info in release version" OFF)
option(LOW_GLIBC_VERSION "make binary which can distribute to environments which have lower GLIBC" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)


#
# Linux configuration.
#

if(OS_LINUX)
  set(OS_SPECIFIC_DIR_NAME "linux")
  # Platform-specific compiler/linker flags.
  list(APPEND KSD_COMPILER_FLAGS
    -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
    -fPIC                           # Generate position-independent code for shared libraries
    -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
    -pipe                           # Use pipes rather than temporary files for communication between build stages
    -Wall                           # Enable all warnings
    -Werror                         # Treat warnings as errors
    -Wno-unused-value               # Disable unused value warning
    -Wno-missing-braces                # Disable missing braces warning
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
    #-Wl,--fatal-warnings              # Treat warnings as errors
    #-Wl,-z,noexecstack                # Mark the stack as non-executable (security feature)
    #-Wl,-z,relro                      # Mark relocation sections as read-only (security feature)
    )
  if(USE_TCMALLOC)
    list(APPEND KSD_LINKER_FLAGS -L${PROJECT_SOURCE_DIR}/third_party/tcmalloc)
    list(APPEND KSD_LINKER_FLAGS -ltcmalloc)
    list(APPEND KSD_LINKER_FLAGS -lunwind)
  endif()
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

  if(PROJECT_ARCH STREQUAL "x86_64")
    # 64-bit architecture.
    list(APPEND KSD_COMPILER_FLAGS
      -m64
      -march=x86-64
      )
    list(APPEND KSD_LINKER_FLAGS
      -m64
      )
  elseif(PROJECT_ARCH STREQUAL "x86")
    # 32-bit architecture.
    list(APPEND KSD_COMPILER_FLAGS
      -msse2
      -mfpmath=sse
      -mmmx
      -m32
      )
    list(APPEND KSD_LINKER_FLAGS
      -m32
      )
  endif()

endif()


#
# Windows configuration.
#

if(OS_WINDOWS)
  set(OS_SPECIFIC_DIR_NAME "win")

  if (GEN_NINJA)
    # When using the Ninja generator clear the CMake defaults to avoid excessive
    # console warnings (see issue #2120).
    set(CMAKE_CXX_FLAGS "")
    set(CMAKE_CXX_FLAGS_DEBUG "/MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "")
  endif()

  # Consumers who run into LNK4099 warnings can pass /Z7 instead (see issue #385).
  set(KSD_DEBUG_INFO_FLAG "/Zi" CACHE STRING "Optional flag specifying specific /Z flag to use")

  if(STATIC_LINK_CRT)
    set(KSD_RUNTIME_LIBRARY_FLAG "/MT")
  else()
    set(KSD_RUNTIME_LIBRARY_FLAG "/MD")
  endif()
  # Consumers using different runtime types may want to pass different flags
  list(APPEND KSD_COMPILER_FLAGS_DEBUG ${KSD_RUNTIME_LIBRARY_FLAG}d)
  list(APPEND KSD_COMPILER_FLAGS_RELEASE ${KSD_RUNTIME_LIBRARY_FLAG})

  # Platform-specific compiler/linker flags.
  list(APPEND KSD_COMPILER_FLAGS
    /MP           # Multiprocess compilation
    /Gy           # Enable function-level linking
    /W4           # Warning level 4
    /WX           # Treat warnings as errors
    /EHa          # C++ Exception with SEH
    /source-charset:utf-8 #specify the source file charset
    ${KSD_DEBUG_INFO_FLAG}
    )
  list(APPEND KSD_COMPILER_FLAGS_DEBUG
    /Od           # Disable optimizations
    /RTC1         # Enable basic run-time checks
    )
  list(APPEND KSD_COMPILER_FLAGS_RELEASE
    /O2           # Optimize for maximum speed
    /Ob2          # Inline any suitable function
    /GF           # Enable string pooling
    /GL
    )
  set(KSD_LINKER_FLAGS)
  list(APPEND KSD_LINKER_FLAGS_DEBUG
    /DEBUG        # Generate debug information
    )
  list(APPEND KSD_LINKER_FLAGS_RELEASE
    /LTCG:incremental   # link-time code generation incremental
    )
  if(FORCE_DEBUG_INFO)
    list(APPEND KSD_LINKER_FLAGS_RELEASE
      /DEBUG
   )
  endif()
  list(APPEND KSD_COMPILER_DEFINES
    WIN32 _WIN32 _WINDOWS NOGDI       # Windows platform
    UNICODE _UNICODE                  # Unicode build
    WINVER=0x0601 _WIN32_WINNT=0x601  # Targeting Windows 7
    WIN32_LEAN_AND_MEAN               # Exclude less common API declarations
    _CRT_SECURE_NO_WARNINGS           # No secure warnings
    _CRT_NONSTDC_NO_DEPRECATE         # No ISO warnings
    )
  list(APPEND KSD_COMPILER_DEFINES_RELEASE
    NDEBUG _NDEBUG                    # Not a debug build
    )

endif()



#
# MAC configuration.
#

if(OS_MACOSX)
  set(OS_SPECIFIC_DIR_NAME "macos")
  # Platform-specific compiler/linker flags.
  list(APPEND KSD_COMPILER_FLAGS
          -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
          -fPIC                           # Generate position-independent code for shared libraries
          -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
          -pipe                           # Use pipes rather than temporary files for communication between build stages
          -Wall                           # Enable all warnings
          -Werror                         # Treat warnings as errors
          -Wno-unused                        # Disable unused value warning
          -Wno-missing-braces                # Disable missing braces warning
          #-Wno-inconsistent-missing-override # Disable inconsistent missing override
          -Wno-deprecated-declarations        # Disable deprecated declarations
          -Wno-pessimizing-move               # Disable pessimizing move
          )
  list(APPEND KSD_COMPILER_FLAGS   # for C++
          -fno-rtti                       # Disable real-time type information
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
          #-Wl,--fatal-warnings              # Treat warnings as errors
          -Wl,-rpath,.:./cfg:/data/light/lib # Set rpath so that libraries can be placed next to the executable
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
          _MACOS                         # IsLinux
          )
  list(APPEND KSD_COMPILER_DEFINES_RELEASE
          NDEBUG                          # Not a debug build
          )

  include(CheckCXXCompilerFlag)

  if(PROJECT_ARCH STREQUAL "x86_64")
    # 64-bit architecture.
    list(APPEND KSD_COMPILER_FLAGS
            -m64
            -march=x86-64
            )
    list(APPEND KSD_LINKER_FLAGS
            -m64
            )
  elseif(PROJECT_ARCH STREQUAL "x86")
    # 32-bit architecture.
    list(APPEND KSD_COMPILER_FLAGS
            -msse2
            -mfpmath=sse
            -mmmx
            -m32
            )
    list(APPEND KSD_LINKER_FLAGS
            -m32
            )
  endif()

endif()
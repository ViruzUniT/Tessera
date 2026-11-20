#pragma once

// Build configuration --------------------------------------------------------

#if (defined(TESSERA_DEBUG) + defined(TESSERA_STAGING) + defined(TESSERA_RELEASE)) != 1
#error "Exactly one build configuration must be defined: TESSERA_DEBUG, TESSERA_STAGING, or TESSERA_RELEASE."
#endif

// Platform -------------------------------------------------------------------

#if defined(_WIN32)
#define TESSERA_PLATFORM_WINDOWS 1
#else
#error "Unsupported platform. Tessera currently targets Windows first."
#endif

// Compiler -------------------------------------------------------------------

#if defined(__clang__)
#define TESSERA_COMPILER_CLANG 1
#elif defined(_MSC_VER)
#define TESSERA_COMPILER_MSVC 1
#else
#error "Unsupported compiler. Use Clang or MSVC."
#endif

// Architecture ---------------------------------------------------------------

#if defined(_M_X64) || defined(__x86_64__)
#define TESSERA_ARCH_X64 1
#else
#error "Unsupported architecture. Tessera currently targets x64."
#endif

// C++ standard ---------------------------------------------------------------

#if defined(_MSVC_LANG)
#define TESSERA_CPP_VERSION _MSVC_LANG
#else
#define TESSERA_CPP_VERSION __cplusplus
#endif

#if TESSERA_CPP_VERSION < 202302L
#error "Tessera requires C++23 or newer."
#endif

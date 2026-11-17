#pragma once
#include <tessera/core/config.h>

#include <source_location>
#include <string_view>

namespace Tessera {

void debugBreak();

[[noreturn]] void reportAssertionFailure(std::string_view expression, std::string_view message,
    std::source_location location = std::source_location::current());

}  // namespace Tessera

#if defined(TESSERA_DEBUG) || defined(TESSERA_STAGING)
#define TESSERA_ASSERT(condition)                                                         \
  do {                                                                                 \
    if (!(condition)) {                                                                \
      ::Tessera::reportAssertionFailure(#condition, {}, std::source_location::current()); \
    }                                                                                  \
  } while (false)
#define TESSERA_ASSERT_MSG(condition, message)                                                   \
  do {                                                                                        \
    if (!(condition)) {                                                                       \
      ::Tessera::reportAssertionFailure(#condition, (message), std::source_location::current()); \
    }                                                                                         \
  } while (false)
#define TESSERA_VERIFY(condition) TESSERA_ASSERT(condition)
#else
#define TESSERA_ASSERT(condition) ((void)0)
#define TESSERA_ASSERT_MSG(condition, message) ((void)0)
#define TESSERA_VERIFY(condition) ((void)(condition))
#endif

#define TESSERA_DEBUG_BREAK() ::Tessera::debugBreak()

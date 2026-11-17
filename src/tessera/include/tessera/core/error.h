#pragma once

#include <tessera/core/log.h>
#include <tessera/pch.h>

#include <format>
#include <string_view>
namespace Tessera {
enum ErrorCode {
  CouldBeAProblem = -1,
  None = 0,
  Exit,
  Unknown,
  InvalidArgument,
  InvalidArgumentIGuess,
  FileNotFound,
  PlatformError,
  GraphicsError,
  FenceError,
  FenceTimeout
};

struct Error {
  ErrorCode code = ErrorCode::None;
  std::string_view message = {};
  inline explicit operator bool() const noexcept {
    if (code == CouldBeAProblem)
      Tessera::logDebug(std::format("A potential error has occurred: {}", message));
    return code > 0;
  }
};
}  // namespace Tessera

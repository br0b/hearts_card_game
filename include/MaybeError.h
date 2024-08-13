#ifndef MAYBE_ERROR_H
#define MAYBE_ERROR_H

#include <memory>
#include <optional>

#include "Error.h"

using MaybeError = std::optional<std::unique_ptr<Error>>;

#endif

#pragma once

#include "core/TranslationTypes.h"

#include <QString>

namespace verbuno {

class PromptBuilder final {
public:
    [[nodiscard]] static QString systemPrompt(const TranslationRequest& request);
    [[nodiscard]] static QString styleInstruction(TranslationStyle style);
};

} // namespace verbuno

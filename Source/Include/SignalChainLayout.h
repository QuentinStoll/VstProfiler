#pragma once

#include <array>
#include <cstdint>

namespace SignalChain {
constexpr int rowCount = 2;
constexpr int columnsPerRow = 8;
constexpr int slotCount = rowCount * columnsPerRow;

// One chain that wraps onto a second row when the first row runs out of width.
// Both rows run left to right. The upper row ends on a down-link; the lower
// row starts with the continuation of that signal and ends on the output.
constexpr int inputSlot = 0;
constexpr int downSlot = 7;
constexpr int returnSlot = 8;
constexpr int outputSlot = 15;
constexpr int movableSlotCount = 12;

inline constexpr bool isMovableSlot(int chainSlot) noexcept {
    return (chainSlot >= 1 && chainSlot <= 6) || (chainSlot >= 9 && chainSlot <= 14);
}

inline constexpr int movableIndexForSlot(int chainSlot) noexcept {
    if (chainSlot >= 1 && chainSlot <= 6) {
        return chainSlot - 1;
    }
    if (chainSlot >= 9 && chainSlot <= 14) {
        return chainSlot - 3;
    }
    return -1;
}

inline constexpr int chainSlotForMovableIndex(int index) noexcept {
    if (index < 0 || index >= movableSlotCount) {
        return -1;
    }
    return index < 6 ? index + 1 : index + 3;
}

inline constexpr int visualRow(int chainSlot) noexcept {
    return chainSlot <= downSlot ? 0 : 1;
}

inline constexpr int visualColumn(int chainSlot) noexcept {
    if (chainSlot <= downSlot) {
        return chainSlot;
    }
    return chainSlot - returnSlot;
}

inline constexpr int chainSlotAt(int row, int column) noexcept {
    if (column < 0 || column >= columnsPerRow) {
        return -1;
    }
    if (row == 0) {
        return column;
    }
    if (row == 1) {
        return returnSlot + column;
    }
    return -1;
}

static_assert(chainSlotAt(0, 2) == 2);
static_assert(chainSlotAt(1, 0) == returnSlot);
static_assert(chainSlotAt(1, columnsPerRow - 1) == outputSlot);
static_assert(visualColumn(downSlot) == columnsPerRow - 1);
static_assert(visualColumn(returnSlot) == 0);

enum class Stage : int {
    Empty = 0,
    Amp = 1,
    Cab = 2,
    Eq = 3,
    Pedal = 4
};

constexpr int uniqueStageCount = 4;

inline constexpr bool isEffectStage(Stage stage) noexcept {
    return stage == Stage::Amp || stage == Stage::Cab || stage == Stage::Eq || stage == Stage::Pedal;
}

struct Layout {
    std::array<Stage, movableSlotCount> slots{{Stage::Empty,
                                               Stage::Amp,
                                               Stage::Empty,
                                               Stage::Cab,
                                               Stage::Empty,
                                               Stage::Eq,
                                               Stage::Empty,
                                               Stage::Empty,
                                               Stage::Empty,
                                               Stage::Empty,
                                               Stage::Empty,
                                               Stage::Empty}};

    Stage atSlot(int chainSlot) const noexcept {
        const auto index = movableIndexForSlot(chainSlot);
        if (index < 0) {
            return Stage::Empty;
        }
        return slots[static_cast<size_t>(index)];
    }

    int slotFor(Stage stage) const noexcept {
        if (!isEffectStage(stage)) {
            return -1;
        }
        for (int index = 0; index < movableSlotCount; ++index) {
            if (slots[static_cast<size_t>(index)] == stage) {
                return chainSlotForMovableIndex(index);
            }
        }
        return -1;
    }

    int count(Stage stage) const noexcept {
        int found = 0;
        for (const auto candidate : slots) {
            if (candidate == stage) {
                ++found;
            }
        }
        return found;
    }

    bool contains(Stage stage) const noexcept {
        return slotFor(stage) >= 0;
    }

    bool occupies(int chainSlot) const noexcept {
        if (chainSlot == inputSlot || chainSlot == downSlot || chainSlot == returnSlot || chainSlot == outputSlot) {
            return true;
        }
        return atSlot(chainSlot) != Stage::Empty;
    }

    void place(Stage stage, int chainSlot) noexcept {
        const int target = movableIndexForSlot(chainSlot);
        if (!isEffectStage(stage) || target < 0) {
            return;
        }

        slots[static_cast<size_t>(target)] = stage;
    }

    void clear(int chainSlot) noexcept {
        const int target = movableIndexForSlot(chainSlot);
        if (target < 0) {
            return;
        }

        slots[static_cast<size_t>(target)] = Stage::Empty;
    }

    void moveSlot(int fromSlot, int toSlot) noexcept {
        const int from = movableIndexForSlot(fromSlot);
        const int to = movableIndexForSlot(toSlot);
        if (from < 0 || to < 0 || from == to) {
            return;
        }

        const auto displaced = slots[static_cast<size_t>(to)];
        slots[static_cast<size_t>(to)] = slots[static_cast<size_t>(from)];
        slots[static_cast<size_t>(from)] = displaced;
    }

    bool isValid() const noexcept {
        for (const auto stage : slots) {
            if (stage != Stage::Empty && !isEffectStage(stage)) {
                return false;
            }
        }
        return true;
    }

    std::array<Stage, movableSlotCount> processingOrder() const noexcept {
        std::array<Stage, movableSlotCount> order{};
        int count = 0;
        for (const auto stage : slots) {
            if (stage != Stage::Empty) {
                order[static_cast<size_t>(count++)] = stage;
            }
        }
        return order;
    }

    int occupiedCount() const noexcept {
        int count = 0;
        for (const auto stage : slots) {
            if (stage != Stage::Empty) {
                ++count;
            }
        }
        return count;
    }

    constexpr std::uint64_t packed() const noexcept {
        std::uint64_t value = 0;
        for (int index = 0; index < movableSlotCount; ++index) {
            value |= (static_cast<std::uint64_t>(slots[static_cast<size_t>(index)]) & 0x0F)
                     << (4 * index);
        }
        return value;
    }

    static Layout fromPacked(std::uint64_t packed) noexcept {
        Layout layout;
        for (int index = 0; index < movableSlotCount; ++index) {
            layout.slots[static_cast<size_t>(index)] =
                static_cast<Stage>((packed >> (4 * index)) & 0x0F);
        }
        return layout.isValid() ? layout : Layout{};
    }
};

inline constexpr std::uint64_t defaultPacked = Layout{}.packed();
}  // namespace SignalChain

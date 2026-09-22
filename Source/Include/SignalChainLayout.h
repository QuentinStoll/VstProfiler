#pragma once

#include <array>
#include <cstdint>

namespace SignalChain {
constexpr int slotCount = 8;
constexpr int inputSlot = 0;
constexpr int outputSlot = 7;
constexpr int firstMovableSlot = 1;
constexpr int lastMovableSlot = 6;
constexpr int movableSlotCount = lastMovableSlot - firstMovableSlot + 1;

enum class Stage : int {
    Empty = 0,
    Amp = 1,
    Cab = 2,
    Eq = 3,
    Pedal = 4
};

constexpr int uniqueStageCount = 4;

inline constexpr bool isUniqueStage(Stage stage) noexcept {
    return stage == Stage::Amp || stage == Stage::Cab || stage == Stage::Eq || stage == Stage::Pedal;
}

struct Layout {
    std::array<Stage, movableSlotCount> slots{{Stage::Empty, Stage::Amp, Stage::Empty, Stage::Cab, Stage::Empty, Stage::Eq}};

    Stage atSlot(int chainSlot) const noexcept {
        if (chainSlot < firstMovableSlot || chainSlot > lastMovableSlot) {
            return Stage::Empty;
        }
        return slots[static_cast<size_t>(chainSlot - firstMovableSlot)];
    }

    int slotFor(Stage stage) const noexcept {
        if (!isUniqueStage(stage)) {
            return -1;
        }
        for (int index = 0; index < movableSlotCount; ++index) {
            if (slots[static_cast<size_t>(index)] == stage) {
                return index + firstMovableSlot;
            }
        }
        return -1;
    }

    bool contains(Stage stage) const noexcept {
        return slotFor(stage) >= 0;
    }

    bool occupies(int chainSlot) const noexcept {
        if (chainSlot == inputSlot || chainSlot == outputSlot) {
            return true;
        }
        return atSlot(chainSlot) != Stage::Empty;
    }

    void place(Stage stage, int chainSlot) noexcept {
        if (!isUniqueStage(stage) || chainSlot < firstMovableSlot || chainSlot > lastMovableSlot) {
            return;
        }

        const int target = chainSlot - firstMovableSlot;
        int from = -1;
        for (int index = 0; index < movableSlotCount; ++index) {
            if (slots[static_cast<size_t>(index)] == stage) {
                from = index;
                break;
            }
        }

        if (from == target) {
            return;
        }

        const auto displaced = slots[static_cast<size_t>(target)];
        slots[static_cast<size_t>(target)] = stage;
        if (from >= 0) {
            slots[static_cast<size_t>(from)] = displaced == stage ? Stage::Empty : displaced;
        }
    }

    bool isValid() const noexcept {
        bool seenAmp = false;
        bool seenCab = false;
        bool seenEq = false;
        bool seenPedal = false;
        for (const auto stage : slots) {
            switch (stage) {
                case Stage::Empty:
                    break;
                case Stage::Amp:
                    if (seenAmp) {
                        return false;
                    }
                    seenAmp = true;
                    break;
                case Stage::Cab:
                    if (seenCab) {
                        return false;
                    }
                    seenCab = true;
                    break;
                case Stage::Eq:
                    if (seenEq) {
                        return false;
                    }
                    seenEq = true;
                    break;
                case Stage::Pedal:
                    if (seenPedal) {
                        return false;
                    }
                    seenPedal = true;
                    break;
                default:
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

    constexpr std::uint32_t packed() const noexcept {
        std::uint32_t value = 0;
        for (int index = 0; index < movableSlotCount; ++index) {
            value |= (static_cast<std::uint32_t>(slots[static_cast<size_t>(index)]) & 0x0F)
                     << (4 * index);
        }
        return value;
    }

    static Layout fromPacked(std::uint32_t packed) noexcept {
        Layout layout;
        for (int index = 0; index < movableSlotCount; ++index) {
            layout.slots[static_cast<size_t>(index)] = static_cast<Stage>((packed >> (4 * index)) & 0x0F);
        }
        return layout.isValid() ? layout : Layout{};
    }
};

inline constexpr std::uint32_t defaultPacked = Layout{}.packed();
}  // namespace SignalChain

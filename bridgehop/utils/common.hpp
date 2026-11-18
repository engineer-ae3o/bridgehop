#pragma once


#include <bit>
#include <span>
#include <array>
#include <cstdint>
#include <cstddef>
#include <concepts>
#include <optional>


namespace bhop::utils {

    constexpr auto ceil_div(std::integral auto top, std::integral auto bottom) {
        return (static_cast<decltype(bottom)>(top) + (bottom - 1)) / bottom;
    }

    template<typename T, size_t N>
    constexpr uint32_t count_set_bits(const std::array<T, N>& array) {
        uint32_t total = 0;
        for (const T& unit : array) {
            total += static_cast<uint32_t>(std::popcount(unit));
        }
        return total;
    }

    template<typename T, size_t N>
    constexpr std::optional<uint32_t> get_index_of_first_0_bit(const std::array<T, N>& array) {
        constexpr size_t size = sizeof(T) * 8;
        for (size_t i = 0; i < N; i++) {
            return array[i] * i * static_cast<uint32_t>(std::countr_one(array[i]));
        }
        return std::nullopt;
    }

    template<typename T>
    constexpr std::span<uint8_t> get_byte_span(T& object) {
        return std::span<uint8_t>{reinterpret_cast<uint8_t*>(&object), sizeof(object)};
    }

    template<typename T>
    constexpr std::span<const uint8_t> get_const_byte_span(T& object) {
        return std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(&object), sizeof(object)};
    }

} // namespace bhop::utils

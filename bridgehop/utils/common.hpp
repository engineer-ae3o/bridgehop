#pragma once


#include <bit>
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
            //return array[i] * i * std::countr(array[i]);
        }
        return std::nullopt;
    }

} // namespace bhop::utils

#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/platform.hpp"
#include "utils/config.hpp"

#include <array>
#include <utility>


namespace bhop::dedupe {

    template<platform::system system>
    class cache_t {
    public:
        using tick_type_t = typename system::tick_type_t;

        // A slot with a timestamp of 0 indicates an empty slot
        using element_t = std::pair<bhop::packet_id_t, tick_type_t>;

        constexpr void erase() {
            m_cache.fill(element_t{});
        }

        constexpr void insert_packet(const bhop::packet_id_t& other) {
            element_t*        oldest_slot = m_cache.begin();
            const tick_type_t now         = system::now_ms();
            for (auto& slot : m_cache) {
                auto& [packet, timestamp_ms] = slot;
                // Free this slot up if it's passed its age of expiry
                if (has_expired(timestamp_ms, now)) {
                    timestamp_ms = 0;
                }
                // Insert the packet at this slot if free
                if (timestamp_ms == 0) {
                    packet       = other;
                    timestamp_ms = now;
                    return;
                }
                // Get a reference to the oldest ID slot currently seen
                if (oldest_slot->second > timestamp_ms) {
                    oldest_slot = &slot;
                }
            }
            // Store this packet's ID at the oldest used slot since no free slots
            *oldest_slot = element_t{other, now};
        }

        [[nodiscard]] constexpr bool is_packet_present(const bhop::packet_id_t& other) {
            const tick_type_t now = system::now_ms();
            for (auto& [packet, timestamp_ms] : m_cache) {
                // Free this slot up if it's passed its age of expiry
                if (has_expired(timestamp_ms, now)) {
                    timestamp_ms = 0;
                }
                // Slot is in use and the ID is in the cache
                if ((timestamp_ms != 0) && (packet == other)) {
                    return true;
                }
            }
            // Packet not present in the used slots of the cache
            return false;
        }

    private:
        // Stores the packet ID and its age (time of entry into the cache)
        std::array<element_t, config::DEDUPE_CACHE_SIZE> m_cache{};

        static constexpr bool has_expired(tick_type_t timestamp_ms, tick_type_t now) {
            return ((now - timestamp_ms) >= static_cast<tick_type_t>(config::DEDUPE_CACHE_ENTRY_TIMEOUT_MS));
        }
    };

} // namespace bhop::dedupe

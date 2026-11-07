#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/platform.hpp"
#include "utils/config.hpp"

#include <array>
#include <utility>


namespace bhop::dedupe {

    template<platform::system_t system>
    class cache_t {
    public:
        using tick_type_t = typename system::tick_type_t;

        // A slot with an age of 0 indicates an empty slot
        using element_t = std::pair<bhop::packet_id_t, tick_type_t>;

        void insert_packet(const bhop::packet_id_t& other) {
            // NOLINTNEXTLINE(readability-container-data-pointer)
            element_t* oldest_slot = &m_cache[0];
            for (auto& slot : m_cache) {
                auto& [packet, timestamp_ms] = slot;
                // Free this slot up if it's passed its age of expiry
                if (has_expired(timestamp_ms)) {
                    timestamp_ms = 0;
                }
                // Insert this packet at this slot since free
                if (timestamp_ms == 0) {
                    packet       = other;
                    timestamp_ms = system::now_ms();
                    return;
                }
                // Get a reference to the oldest ID slot currently seen
                if (oldest_slot->second > timestamp_ms) {
                    oldest_slot = &slot;
                }
            }
            // Store this packet's ID at the oldest used slot since no free slots
            *oldest_slot = element_t{other, system::now_ms()};
        }

        bool is_packet_present(const bhop::packet_id_t& other) {
            for (auto& [packet, timestamp_ms] : m_cache) {
                // Free this slot up if it's passed its age of expiry
                if (has_expired(timestamp_ms)) {
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

        void erase() {
            m_cache.fill(0);
        }

    private:
        // Stores the packet ID and its age (time of entry into the cache)
        std::array<element_t, config::DEDUPE_CACHE_SIZE> m_cache{};

        bool has_expired(tick_type_t timestamp_ms) {
            return ((system::now_ms() - timestamp_ms) >=
                    static_cast<tick_type_t>(config::DEDUPE_CACHE_ENTRY_TIMEOUT_MS));
        }
    };

} // namespace bhop::dedupe

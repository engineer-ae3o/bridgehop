#pragma once


#include "utils/types.hpp"
#include "utils/cache.hpp"
#include "utils/config.hpp"
#include "utils/packet.hpp"
#include "utils/platform.hpp"
#include "utils/bhop_constants.hpp"

#include <span>
#include <array>
#include <optional>


namespace bhop::active {

    template<platform::transport_t transport_t, platform::filesystem_t filesystem_t, platform::timers_t timers_t>
    struct config_t {
        bool         is_first_boot{};
        transport_t  transport{};
        filesystem_t filesystem{};
        timers_t     timer{};
    };

    template<platform::transport_t   transport,
             platform::filesystem_t  filesystem,
             platform::timers_t      timers,
             platform::aes_128_ccm_t aes_128_ccm,
             platform::ed25519_t     ed25519,
             platform::x25519_t      x25519,
             platform::rng_t         rng,
             platform::sha256_t      sha256,
             platform::system_t      system>
    class node_t {
    public:
        using config_t = config_t<transport, filesystem, timers>;

        node_t() = default;

        ~node_t() {
            (void)cleanup();
        }

        node_t(const node_t&)            = delete;
        node_t& operator=(const node_t&) = delete;
        node_t(node_t&&)                 = delete;
        node_t& operator=(node_t&&)      = delete;

        // Initialize the library as an active node
        types::error_t init(const config_t& config) {
            if (m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            // TODO: Check config struct

            m_config = config;

            // Read the contacts file header and cache it
            m_config.filesystem.read(
                types::file_t::CONTACTS,
                std::span{reinterpret_cast<uint8_t*>(&m_contacts_header), sizeof(m_contacts_header)},
                0);

            m_is_initialized = true;
            return types::error_t::ERR_OK;
        }

        // This tears down all active sessions when deinitializing the library.
        // It is also called from the destructor
        types::error_t deinit() {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }
            return cleanup();
        }

        // Open and close a session.
        // NOTE: close drops all received packets up to that point.
        // A callback is registered in init(...) and is invoked when the session is closed
        // asynchronously due to whatever reason and that reason is passed to the callback.
        types::error_t open(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            // TODO: Send handshake packet to peer

            return types::error_t::ERR_OK;
        }

        types::error_t close(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            // TODO: Send close packet to peer and free slot

            return types::error_t::ERR_OK;
        }

        // This returns a span to all the IDs of all nodes with which exists a connection.
        [[nodiscard]] std::span<bhop::node_id_t> get_open_sessions() {
            if (!m_is_initialized) {
                return {};
            }

            return {};
        }

        // Add a contact with its ID
        types::error_t add_contact(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            auto index = utils::get_index_of_first_0_bit(m_contacts_header);
            if (index == std::nullopt) {
                // All bits are set. No free slot
                return types::error_t::ERR_CONTACT_LIST_FULL;
            }

            const types::contact_slot_t contact = {.node_id = node_id, .handshake_counter = 0};
            m_config.filesystem.write(types::file_t::CONTACTS,
                                      std::span{reinterpret_cast<const uint8_t*>(&contact), sizeof(contact)},
                                      CONTACTS_OFFSET + (index.value() * sizeof(contact)));

            // Mark slot as used and write back header to the file
            m_contacts_header[index.value() / 8] |= 1UL << (index.value() % 8);
            m_config.filesystem.write(
                types::file_t::CONTACTS,
                std::span{reinterpret_cast<const uint8_t*>(&m_contacts_header), sizeof(m_contacts_header)},
                0);

            return types::error_t::ERR_OK;
        }

        // Remove a contact with its ID
        types::error_t delete_contact(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            // Read in batches
            std::array<types::contact_slot_t, CONTACTS_FILE_READ_BATCH_SIZE> slot;

            // Scan file for free slot and insert
            for (uint32_t i = 0; i < (config::MAX_CONTACTS / slot.size()); i++) {
                m_config.filesystem.read(types::file_t::CONTACTS,
                                         std::span{reinterpret_cast<uint8_t*>(&slot), sizeof(slot)},
                                         CONTACTS_OFFSET + (i * sizeof(slot)));
            }

            // Contact is not on this device
            return types::error_t::ERR_CONTACT_NOT_EXIST;
        }

        [[nodiscard]] bool is_contact_stored(const bhop::node_id_t& node_id) const {
            if (!m_is_initialized) {
                return false;
            }

            // Read in batches
            std::array<types::contact_slot_t, CONTACTS_FILE_READ_BATCH_SIZE> slot;

            // Scan file for free slot and insert
            for (uint32_t i = 0; i < (config::MAX_CONTACTS / slot.size()); i++) {
                m_config.filesystem.read(types::file_t::CONTACTS,
                                         std::span{reinterpret_cast<uint8_t*>(&slot), sizeof(slot)},
                                         CONTACTS_OFFSET + (i * sizeof(slot)));
            }

            // Contact not in any used slot
            return false;
        }

        // Return the number of stored contacts
        [[nodiscard]] std::optional<uint32_t> get_num_contacts() const {
            if (!m_is_initialized) {
                return std::nullopt;
            }
            return utils::count_set_bits(m_contacts_header);
        }

        // Send a message to a peer. It fails if the ID has not been
        // saved before or a session does not exist with that peer.
        types::error_t send(const bhop::node_id_t& node_id, std::span<const uint8_t> msg) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            return types::error_t::ERR_OK;
        }

        // NOTE: For recption of messages, a callback is registered in init(...) and that is called when a message is
        // received. A span to the received message and the node ID of the sender node are passed to the callback.

        // Gets the debug counter. Pretty self explanatory
        [[nodiscard]] std::optional<uint32_t> get_debug_counter() const {
            if (!m_is_initialized) {
                return std::nullopt;
            }
            return m_debug_counter;
        }

    private:
        bool     m_is_initialized{};
        config_t m_config{};
        uint32_t m_debug_counter{};

        dedupe::cache_t<system> m_cache{};

        types::contacts_file_header_t m_contacts_header{};

        // Where the contact slots start in the contacts file
        static constexpr uint32_t CONTACTS_OFFSET               = sizeof(types::contacts_file_header_t);
        static constexpr uint32_t CONTACTS_FILE_READ_BATCH_SIZE = 20;

        types::error_t cleanup() {
            types::error_t error = types::error_t::ERR_OK;

            // Close all open sessions
            auto sessions = get_open_sessions();
            for (const auto& session : sessions) {
                error = close(session);
            }

            // TODO: Erase all internal state
            m_config         = {};
            m_debug_counter  = 0;
            m_is_initialized = false;
            m_cache.erase();

            return error;
        }
    };

} // namespace bhop::active

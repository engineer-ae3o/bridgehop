#pragma once


#include "utils/types.hpp"
#include "utils/cache.hpp"
#include "utils/config.hpp"
#include "utils/packet.hpp"
#include "utils/platform.hpp"
#include "utils/bhop_constants.hpp"

#include <span>
#include <array>


namespace bhop::active {

    struct config_t {};

    template<platform::transport_t   transport,
             platform::aes_128_ccm_t aes_128_ccm,
             platform::ed25519_t     ed25519,
             platform::x25519_t      x25519,
             platform::rng_t         rng,
             platform::sha256_t      sha256,
             platform::system_t      system,
             platform::filesystem_t  filesystem>
    class node_t {
    public:
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

            return types::error_t::ERR_OK;
        }

        types::error_t close(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

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

            return types::error_t::ERR_OK;
        }

        // Remove a contact with its ID
        types::error_t delete_contact(const bhop::node_id_t& node_id) {
            if (!m_is_initialized) {
                return types::error_t::ERR_INVALID_STATE;
            }

            return types::error_t::ERR_OK;
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
        [[nodiscard]] uint32_t get_debug_counter() const {
            return m_debug_counter;
        }

    private:
        bool     m_is_initialized{};
        config_t m_config{};
        uint32_t m_debug_counter{};

        dedupe::cache_t<system> m_cache{};

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

#pragma once


#include "utils/types.hpp"
#include "utils/platform.hpp"
#include "utils/bhop_constants.hpp"

#include <span>
#include <optional>
#include <expected>


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
        ~node_t() {
            (void)cleanup();
        }

        node_t(const node_t&)            = default;
        node_t& operator=(const node_t&) = default;
        node_t(node_t&&)                 = default;
        node_t& operator=(node_t&&)      = default;

        // Create an instance of the library as an active node
        [[nodiscard]] static std::expected<node_t, types::error_t> create(const config_t& config) {
            node_t node;
            auto   ret = node.init(config);
            if (ret != types::error_t::ERR_OK) {
                return std::unexpected(ret);
            }
            return node;
        }

        // This tears down all active sessions when deinitializing the library.
        // It is also called from the destructor
        types::error_t deinit() {
            cleanup();
            return types::error_t::ERR_OK;
        }

        // Open and close a session.
        // NOTE: close drops all received packets up to that point.
        // A callback is registered in init(...) and is invoked when the session is closed
        // asynchronously due to whatever reason and that reason is passed to the callback.
        types::error_t open(const bhop::node_id_t& node_id) {
            return types::error_t::ERR_OK;
        }

        types::error_t close(const bhop::node_id_t& node_id) {
            return types::error_t::ERR_OK;
        }

        // This returns a span to all the IDs of all nodes with which exists a connection.
        // Returns std::nullopt if no open sessions.
        [[nodiscard]] std::optional<std::span<bhop::node_id_t>> get_open_sessions() {
            return {};
        }

        // Add a contact with its ID
        types::error_t add_contact(const bhop::node_id_t& node_id) {
            return types::error_t::ERR_OK;
        }

        // Remove a contact with its ID
        types::error_t delete_contact(const bhop::node_id_t& node_id) {
            return types::error_t::ERR_OK;
        }

        // Send a message to a peer. It fails if the ID has not been
        // saved before or a session does not exist with that peer.
        types::error_t send(const bhop::node_id_t& node_id, std::span<const uint8_t> msg) {
            return types::error_t::ERR_OK;
        }

        // NOTE: For recption of messages, a callback is registered in init(...) and that is called when a message is
        // received. A span to the received message and the node ID of the sender node are passed to the callback.

        // Gets the debug counter. Pretty self explanatory
        uint32_t get_debug_counter() {
            return m_debug_counter;
        }

    private:
        node_t() = default;

        uint32_t m_debug_counter{};

        types::error_t init(const config_t& config) {
            return types::error_t::ERR_OK;
        }

        types::error_t cleanup() {
            types::error_t error = types::error_t::ERR_OK;
            // Close all open sessions
            auto sessions = get_open_sessions();
            if (sessions) {
                for (auto session : sessions.value()) {
                    error = close(session);
                }
            }
            // TODO: Erase all internal state
            return types::error_t::ERR_OK;
        }
    };

} // namespace bhop::active

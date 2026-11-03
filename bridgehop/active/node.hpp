#pragma once


#include "utils/types.hpp"
#include "utils/bhop_constants.hpp"

#include <span>
#include <optional>
#include <expected>


namespace bhop::active {

    struct config_t {};

    class node_t {
    public:
        ~node_t();

        node_t(const node_t&)            = default;
        node_t& operator=(const node_t&) = default;
        node_t(node_t&&)                 = default;
        node_t& operator=(node_t&&)      = default;

        // Create an instance of the library
        [[nodiscard]] static std::expected<node_t, types::error_t> create(const config_t& config);

        // deinit tears down all active sessions when deinitializing the library.
        // It is also called from the destructor
        types::error_t deinit();

        // Open and close a session.
        // NOTE: close drops all received packets up to that point.
        // A callback is registered in init(...) and is invoked when the session is closed
        // asynchronously due to whatever reason and that reason is passed to the callback.
        types::error_t open(const bhop::node_id_t& id);
        types::error_t close(const bhop::node_id_t& id);

        // This returns a span to all the IDs of all nodes with which exists a connection.
        // Returns std::nullopt if no open sessions.
        [[nodiscard]] std::optional<std::span<bhop::node_id_t>> get_open_sessions();

        // Add/remove a contact with its ID
        types::error_t add_contact(const bhop::node_id_t& id);
        types::error_t delete_contact(const bhop::node_id_t& id);

        // Send a message to a peer. It fails if the ID has not been
        // saved before or a session does not exist with that peer.
        types::error_t send(const bhop::node_id_t& id, std::span<const uint8_t> msg);

        // NOTE: For recption of messages, a callback is registered in init(...) and that is called when a message is
        // received. A span to the received message and the node ID of the sender node are passed to the callback.

        // The application should call this whenever the transport layer receives a packet
        void recv_packet(std::span<uint8_t> packet);

    private:
        node_t();
        types::error_t init(const config_t& config);
    };

} // namespace bhop::active

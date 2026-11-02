#pragma once


#include "../utils/types.hpp"
#include "../utils/bhop_constants.hpp"

#include <span>
#include <optional>


namespace bhop::active {

    using node_t = std::span<uint8_t, bhop::NODE_ID_LEN_BYTES>;

    // Initialize and deinitialize the library (when functioning as an active node).
    // NOTE: deinit tears down all active sessions when deinitializing down the library.
    types::error_t init(const types::config_t& config);
    types::error_t deinit();

    // Open and close a session.
    // NOTE: close drops all received packets up to that point.
    // A callback is registered in init(...) and is invoked when the session is closed
    // asynchronously due to whatever reason and that reason is passed to the callback.
    types::error_t open(node_t id);
    types::error_t close(node_t id);

    // This returns a span to all the IDs of all nodes with which exists a connection.
    // Returns std::nullopt if no open sessions.
    [[nodiscard]] std::optional<std::span<bhop::node_id_t>> get_open_sessions();

    // Send data to a peer. When send(...) is called, it checks whether a session is already
    // established with the target node. If no connection, it tries to open a session with
    // the node and returns an error if that open attempts fail. It does not retry on failure.
    // If the ID is new, it is saved to storage and then the application tries to resend the messaage.
    types::error_t send(node_t id, std::span<const uint8_t> msg);

    // NOTE: For recption, a callback is registered in init(...) and that is called when a message is received.
    // A span to the received message and the node ID of the sender node are passed to the callback

} // namespace bhop::active

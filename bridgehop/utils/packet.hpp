#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/types.hpp"

#include <array>
#include <cstdint>


namespace bhop::packet {

    // The packet header. Never encrypted.
    struct [[__gnu__::__packed__]] header_t {
        uint8_t           frame{};     // Holds the TTL, protocol version and packet type
        bhop::packet_id_t packet_id{}; // The hash of the entire payload excluding the frame
        bhop::node_id_t   sender_id{}; // The node ID of the sender
        bhop::node_id_t   target_id{}; // The node ID of the target
    };

    // The different payload types
    struct [[__gnu__::__packed__]] handshake_t {
        uint64_t                  counter{};         // Sender's handshake counter
        bhop::x25519_key_t        x25519_pub_key{};  // Sender's ephemeral ECDH public key
        bhop::ed25519_signature_t signature{};       // Sender's EdDSA (R, S) signature
        bhop::ed25519_key_t       ed25519_pub_key{}; // Sender's long term EdDSA public key
    };

    struct [[__gnu__::__packed__]] data_t {
        bhop::aes_auth_tag_t auth_tag{};    // Authentication tag
        uint32_t             msg_counter{}; // Sender's message counter
        uint8_t              num_chunks{};  // Number of chunks for this message
        uint8_t              chunk_idx{};   // Index of this chunk in the message
    };

    struct [[__gnu__::__packed__]] message_ack_t {
        bhop::aes_auth_tag_t auth_tag{};    // Authentication tag
        uint32_t             msg_counter{}; // Sender's message counter
        uint32_t             ack_counter{}; // COunter of the message sent
    };

    struct [[__gnu__::__packed__]] close_t {
        bhop::aes_auth_tag_t  auth_tag{};    // Authentication tag
        uint32_t              msg_counter{}; // Sender's message counter
        types::close_reason_t reason{};      // Reason for closing the session
    };

    static_assert(sizeof(header_t) == bhop::HEADER_PACKET_SIZE);
    static_assert(sizeof(handshake_t) == bhop::HANDSHAKE_PACKET_SIZE);
    static_assert(sizeof(data_t) == bhop::DATA_PACKET_SIZE);
    static_assert(sizeof(message_ack_t) == bhop::MESSAGE_ACK_PACKET_SIZE);
    static_assert(sizeof(close_t) == bhop::CLOSE_PACKET_SIZE);

} // namespace bhop::packet

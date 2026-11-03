#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/types.hpp"

#include <array>
#include <cstdint>


namespace bhop::packet {

    // The packet header. Never encrypted.
    // The frame field determines how to interpret the remaining bytes
    struct [[__gnu__::__packed__]] header_t {
        uint8_t           frame{};
        bhop::packet_id_t packet_id{};
        bhop::node_id_t   sender_id{};
        bhop::node_id_t   target_id{};
    };

    // The different payload types
    struct [[__gnu__::__packed__]] handshake_t {
        uint64_t                  counter{};
        bhop::x25519_key_t        x25519_pub_key{};
        bhop::ed25519_signature_t signature;
        bhop::ed25519_key_t       ed25519_pub_key{};
    };

    struct [[__gnu__::__packed__]] data_t {
        bhop::aes_auth_tag_t auth_tag{};
        uint32_t             msg_counter{};
        uint8_t              num_chunks{};
        uint8_t              chunk_id{};
    };

    struct [[__gnu__::__packed__]] message_ack_t {
        bhop::aes_auth_tag_t auth_tag{};
        uint32_t             msg_counter{};
        uint32_t             ack_counter{};
    };

    struct [[__gnu__::__packed__]] close_t {
        bhop::aes_auth_tag_t  auth_tag{};
        uint32_t              msg_counter{};
        types::close_reason_t reason{};
    };

} // namespace bhop::packet

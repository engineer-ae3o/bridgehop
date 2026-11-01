#pragma once


#include "utils/config.hpp"

#include <array>
#include <cstdint>


namespace bhop::packet {

    using node_id_t   = std::array<uint8_t, config::NODE_ID_LEN_BYTES>;
    using packet_id_t = std::array<uint8_t, config::PACKET_ID_LEN_BYTES>;

    using auth_tag_t = std::array<uint8_t, config::AES_CCM_AUTH_TAG_LEN_BYTES>;

    using x25519_key_t        = std::array<uint8_t, config::X25519_KEY_LEN_BYTES>;
    using ed25519_key_t       = std::array<uint8_t, config::Ed25519_KEY_LEN_BYTES>;
    using ed25519_signature_t = std::array<uint8_t, config::Ed25519_SIGNATURE_ELEMENT_LEN_BYTES>;

    // The packet header. Never encrypted.
    // The frame field determines how to interpret the remaining bytes
    struct [[__gnu__::__packed__]] header_t {
        uint8_t     frame{};
        packet_id_t packet_id{};
        node_id_t   sender_id{};
        node_id_t   target_id{};
    };

    // The different payload types
    struct [[__gnu__::__packed__]] handshake_t {
        uint64_t     counter{};
        x25519_key_t x25519_pub_key{};
        struct {
            ed25519_signature_t R{};
            ed25519_signature_t S{};
        } signature;
        ed25519_key_t ed25519_pub_key{};
    };

    struct [[__gnu__::__packed__]] data_t {
        auth_tag_t auth_tag{};
        uint32_t   msg_counter{};
        uint8_t    num_chunks{};
        uint8_t    chunk_id{};
    };

    struct [[__gnu__::__packed__]] message_ack_t {
        auth_tag_t auth_tag{};
        uint32_t   msg_counter{};
        uint32_t   ack_counter{};
    };

    struct [[__gnu__::__packed__]] close_t {
        auth_tag_t auth_tag{};
        uint32_t   msg_counter{};
        uint8_t    reason{};
    };

} // namespace bhop::packet

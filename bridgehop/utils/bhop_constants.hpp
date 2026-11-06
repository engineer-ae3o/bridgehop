#pragma once


#include <array>
#include <limits>
#include <cstdint>
#include <string_view>


namespace bhop::bhop {


    // This file contains constants from the protocol specification.
    // They should NOT be modified under any circumstance.
    // The configurable constants are over at utils/config.hpp


    // Protocol version
    constexpr inline uint32_t         BRIDGEHOP_VERSION     = 0;
    constexpr inline std::string_view BRIDGEHOP_VERSION_STR = "v0.1.0";

    // Bit positions of the various fields in the frame field of the packet header
    constexpr inline uint32_t HOP_COUNT_BIT_POS  = 0;
    constexpr inline uint32_t HOP_COUNT_BIT_MASK = 0b1111;

    constexpr inline uint32_t VERSION_BIT_POS  = 4;
    constexpr inline uint32_t VERSION_BIT_MASK = 0b11;

    constexpr inline uint32_t PACKET_TYPE_BIT_POS  = 6;
    constexpr inline uint32_t PACKET_TYPE_BIT_MASK = 0b11;

    static_assert(((HOP_COUNT_BIT_MASK << HOP_COUNT_BIT_POS) | (VERSION_BIT_MASK << VERSION_BIT_POS) |
                   (PACKET_TYPE_BIT_MASK << PACKET_TYPE_BIT_POS)) == std::numeric_limits<uint8_t>::max());

    // Sizes of the different packet types
    constexpr inline uint32_t HEADER_PACKET_SIZE      = 41;
    constexpr inline uint32_t HANDSHAKE_PACKET_SIZE   = 136;
    constexpr inline uint32_t DATA_PACKET_SIZE        = 22;
    constexpr inline uint32_t MESSAGE_ACK_PACKET_SIZE = 24;
    constexpr inline uint32_t CLOSE_PACKET_SIZE       = 21;

    constexpr inline uint32_t HANDSHAKE_FULL_PACKET_SIZE   = HEADER_PACKET_SIZE + HANDSHAKE_PACKET_SIZE;
    constexpr inline uint32_t DATA_PACKET_METADATA_SIZE    = HEADER_PACKET_SIZE + DATA_PACKET_SIZE;
    constexpr inline uint32_t MESSAGE_ACK_FULL_PACKET_SIZE = HEADER_PACKET_SIZE + MESSAGE_ACK_PACKET_SIZE;
    constexpr inline uint32_t CLOSE_FULL_PACKET_SIZE       = HEADER_PACKET_SIZE + CLOSE_PACKET_SIZE;

    // Sizes of the IDs for a node and a packet
    constexpr inline uint32_t PACKET_ID_LEN_BYTES = 8;
    constexpr inline uint32_t NODE_ID_LEN_BYTES   = 16;

    // Sizes of the AES parameters (AES-128-CCM) and the output from the HKDF
    constexpr inline uint32_t AES_KEY_LEN_BYTES      = 128 / 8;
    constexpr inline uint32_t AES_NONCE_LEN_BYTES    = 12;
    constexpr inline uint32_t AES_AUTH_TAG_LEN_BYTES = 16;
    constexpr inline uint32_t HKDF_OUTPUT_LEN_BYTES  = (AES_KEY_LEN_BYTES * 2) + (AES_NONCE_LEN_BYTES * 2);

    // Assymetric crypto keys and signature sizes. Covers the public and private keys
    constexpr inline uint32_t X25519_KEY_LEN_BYTES             = 32;
    constexpr inline uint32_t Ed25519_SEED_LEN_BYTES           = 32;
    constexpr inline uint32_t Ed25519_KEY_LEN_BYTES            = 32;
    constexpr inline uint32_t Ed25519_SIGNATURE_HALF_LEN_BYTES = 32;


    // Useful aliases
    using node_id_t   = std::array<uint8_t, NODE_ID_LEN_BYTES>;
    using packet_id_t = std::array<uint8_t, PACKET_ID_LEN_BYTES>;

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    using sha256_digest_t = std::array<uint8_t, 256 / 8>;
    using sha256_hkdf_t   = std::array<uint8_t, bhop::HKDF_OUTPUT_LEN_BYTES>;

    using aes_key_t      = std::array<uint8_t, AES_KEY_LEN_BYTES>;
    using aes_nonce_t    = std::array<uint8_t, AES_NONCE_LEN_BYTES>;
    using aes_auth_tag_t = std::array<uint8_t, AES_AUTH_TAG_LEN_BYTES>;

    using x25519_key_t   = std::array<uint8_t, X25519_KEY_LEN_BYTES>;
    using ed25519_key_t  = std::array<uint8_t, Ed25519_KEY_LEN_BYTES>;
    using ed25519_seed_t = std::array<uint8_t, Ed25519_SEED_LEN_BYTES>;

    // The EdDSA (R, S) signature
    struct [[__gnu__::__packed__]] ed25519_signature_t {
        std::array<uint8_t, Ed25519_SIGNATURE_HALF_LEN_BYTES> R{};
        std::array<uint8_t, Ed25519_SIGNATURE_HALF_LEN_BYTES> S{};
    };


} // namespace bhop::bhop

#pragma once


#include <cstdint>
#include <string_view>


namespace bhop::constants {


    // This file contains constants from the protocl specification.
    // They are not to be tampered with.


    // Protocol version
    constexpr inline uint32_t         BRIDGEHOP_VERSION     = 0;
    constexpr inline std::string_view BRIDGEHOP_VERSION_STR = "v0.1.0";


    // Bit positions of the various fields in the frame field of the packet header
    constexpr inline uint32_t HOP_COUNT_BIT_POS   = 0;
    constexpr inline uint32_t VERSION_BIT_POS     = 4;
    constexpr inline uint32_t PACKET_TYPE_BIT_POS = 6;


    // Maximum hops per packet and dedupe cache parameters
    constexpr inline uint32_t MAX_HOPS_PER_PACKET          = 10;
    constexpr inline uint32_t DEDUPE_CACHE_SIZE            = 2048;
    constexpr inline uint32_t DEDUPE_CACHE_ENTRY_PERIOD_MS = 120 * 1000;


    // Before a packet is retransmitted, it is made to wait a random delay between this interval
    constexpr inline uint32_t PROPAGATION_DELAY_MIN_MS = 0;
    constexpr inline uint32_t PROPAGATION_DELAY_MAX_MS = 250;


    // Various timeouts for different states
    constexpr inline uint32_t HANDSHAKE_TIMEOUT_MS    = 60 * 1000;
    constexpr inline uint32_t SESSION_IDLE_TIMEOUT_MS = 60 * 60 * 1000;
    constexpr inline uint32_t REASSEMBLY_TIMEOUT_MS   = 45 * 1000;
    constexpr inline uint32_t MESSAGE_ACK_TIMEOUT_MS  = 120 * 1000;


    // Sizes of the IDs for a node and a packet
    constexpr inline uint32_t PACKET_ID_LEN_BYTES = 8;
    constexpr inline uint32_t NODE_ID_LEN_BYTES   = 16;


    // Sizes of the AES parameters (AES-128-CCM)
    constexpr inline uint32_t AES_CCM_KEY_LEN_BYTES      = 128 / 8;
    constexpr inline uint32_t AES_CCM_NONCE_LEN_BYTES    = 12;
    constexpr inline uint32_t AES_CCM_AUTH_TAG_LEN_BYTES = 16;


    // Assymetric crypto keys and signature sizes. Covers the public and private keys
    constexpr inline uint32_t X25519_KEY_LEN_BYTES                = 32;
    constexpr inline uint32_t Ed25519_KEY_LEN_BYTES               = 32;
    constexpr inline uint32_t Ed25519_SIGNATURE_ELEMENT_LEN_BYTES = 32;

} // namespace bhop::constants

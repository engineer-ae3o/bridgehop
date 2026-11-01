#pragma once


#include <cstdint>


namespace bhop::config {

    constexpr uint32_t BRIDGEHOP_VERSION = 1;

    constexpr uint32_t PACKET_ID_LEN_BYTES = 8;
    constexpr uint32_t NODE_ID_LEN_BYTES   = 16;

    constexpr uint32_t Ed25519_SIGNATURE_ELEMENT_LEN_BYTES = 32;

    constexpr uint32_t X25519_KEY_LEN_BYTES  = 32;
    constexpr uint32_t Ed25519_KEY_LEN_BYTES = 32;

    constexpr uint32_t AES_CCM_AUTH_TAG_LEN_BYTES = 16;

} // namespace bhop::config

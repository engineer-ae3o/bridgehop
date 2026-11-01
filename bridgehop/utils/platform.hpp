#pragma once


#include "utils/bhop_constants.hpp"

#include <span>
#include <cstdint>


// This file contains the platform specific functions to be provided
// for this implementation of BridgeHop to function properly

// These include the transport layer (LoRa, sockets etc), AES-128-CCM, SHA256
// X25519 (ECDHE with Curve25519), Ed25519 (EdDSA with Curve25519) and system
// clock functions


namespace bhop::platform {

    void aes_128_ccm_encrypt(std::span<const uint8_t> key,
                             std::span<const uint8_t> plaintext,
                             std::span<const uint8_t> nonce,
                             std::span<uint8_t>       ciphertext,
                             std::span<uint8_t>       auth_tag);

} // namespace bhop::platform

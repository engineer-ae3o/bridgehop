#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/types.hpp"

#include <span>
#include <cstdint>
#include <etl/delegate.h>


// This file contains the platform specific functions to be provided
// for this implementation of BridgeHop to function properly

// These include the transport layer (LoRa, sockets etc), AES-128-CCM, SHA256
// X25519 (ECDHE with Curve25519), Ed25519 (EdDSA) and filesystem ops.
// NOTE: The library is guaranteed to pass in valid arguments.


namespace bhop::platform {

    // Encrypt data with AES-128-CCM and get the authentication tag
    void aes_128_ccm_encrypt(std::span<const uint8_t, bhop::AES_KEY_LEN_BYTES>   key,
                             std::span<const uint8_t, bhop::AES_NONCE_LEN_BYTES> nonce,
                             std::span<uint8_t, bhop::AES_AUTH_TAG_LEN_BYTES>    auth_tag,
                             std::span<const uint8_t>                            plaintext,
                             std::span<uint8_t>                                  ciphertext);

    // Returns true if the data is decrypted and authenticated successfully, false otherwise
    bool aes_128_ccm_decrypt(std::span<const uint8_t, bhop::AES_KEY_LEN_BYTES>      key,
                             std::span<const uint8_t, bhop::AES_NONCE_LEN_BYTES>    nonce,
                             std::span<const uint8_t, bhop::AES_AUTH_TAG_LEN_BYTES> auth_tag,
                             std::span<uint8_t>                                     plaintext,
                             std::span<const uint8_t>                               ciphertext);

    // Generate the (R, S) EdDSA signature
    void ed25519_generate_signature(std::span<const uint8_t, bhop::Ed25519_KEY_LEN_BYTES>         public_key,
                                    std::span<const uint8_t, bhop::Ed25519_KEY_LEN_BYTES>         private_key,
                                    std::span<const uint8_t>                                      data,
                                    std::span<uint8_t, bhop::Ed25519_SIGNATURE_ELEMENT_LEN_BYTES> r_signature,
                                    std::span<uint8_t, bhop::Ed25519_SIGNATURE_ELEMENT_LEN_BYTES> s_signature);

    // Returns true if the data and signature are verified successfully, false otherwise
    bool ed25519_verify_signature(std::span<const uint8_t, bhop::Ed25519_KEY_LEN_BYTES>               public_key,
                                  std::span<const uint8_t, bhop::Ed25519_KEY_LEN_BYTES>               private_key,
                                  std::span<const uint8_t>                                            data,
                                  std::span<const uint8_t, bhop::Ed25519_SIGNATURE_ELEMENT_LEN_BYTES> r_signature,
                                  std::span<const uint8_t, bhop::Ed25519_SIGNATURE_ELEMENT_LEN_BYTES> s_signature);

    // Generate a session's ECDHE keys
    void x25519_generate_keys(std::span<uint8_t, bhop::X25519_KEY_LEN_BYTES> public_key,
                              std::span<uint8_t, bhop::X25519_KEY_LEN_BYTES> private_key);

    // Get the shared secret from our private key and the peer's public keys
    void x25519_get_shared_secret(std::span<const uint8_t, bhop::X25519_KEY_LEN_BYTES> peer_public_key,
                                  std::span<const uint8_t, bhop::X25519_KEY_LEN_BYTES> our_private_key,
                                  std::span<uint8_t, bhop::X25519_KEY_LEN_BYTES>       shared_secret);

    // Expand the X25519 shared secret into the data to serve as the session keys
    void sha256_hkdf_expand(std::span<const uint8_t, bhop::X25519_KEY_LEN_BYTES> shared_secret,
                            std::span<uint8_t, bhop::HKDF_OUTPUT_LEN_BYTES>      output);

    // Hash data with SHA256
    void sha256_hash(std::span<const uint8_t> data, std::span<uint8_t, 256 / 8> digest);

    // Generate random bytes
    void rng_get(std::span<uint8_t> bytes);

    // Transfer data over the physical layer
    void transmit(std::span<const uint8_t> packet);

    // Because receiving is an inherently asynchronous event, we don't have a plain receive function
    // as we would have to poll or sleep the main thread, both of which are blocking and not what we
    // want. Instead, we register a callback which gets called when a packet is received and the main
    // application passes in a span to the received data. It is only called once by this library.
    void register_receive_callback(const etl::delegate<void(std::span<uint8_t> packet)>& callback);

    // Read and write into files used by this library. By the time the application calls into this library,
    // these files should have already been opened. Errors on file ops should be handled by the caller
    // since those failure modes are invisible to the library and are implementation details.
    void   file_read(types::file_t file, std::span<uint8_t> data, uint32_t offset);
    void   file_write(types::file_t file, std::span<const uint8_t> data, uint32_t offset);
    size_t get_file_size(types::file_t file);

} // namespace bhop::platform

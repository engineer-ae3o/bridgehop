#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/types.hpp"

#include <span>
#include <cstdint>
#include <concepts>


// This file contains the platform specific functions to be provided
// for this implementation of BridgeHop to function properly

// These include the transport layer (LoRa, sockets etc), AES-128-CCM, SHA256
// X25519 (ECDHE with Curve25519), Ed25519 (EdDSA) and filesystem ops.
// NOTE: The library is guaranteed to use these functions properly.


namespace bhop::platform {

    template<class T>
    concept transport_t =
        requires(const T& t, std::span<const uint8_t> packet, void (*callback)(void*, std::span<const uint8_t>)) {
            // Transfer data over the physical layer.
            // Returns true if packet sent successfully, false otherwise
            { t.transmit(packet) } -> std::same_as<bool>;
            // Since reception is an inherently asynchronous event, we register a callback that gets
            // called when a packet is received, and the app passes a span to packet to our callback
            { t.register_recv_cb(callback) };
        };

    template<class T>
    concept aes_128_ccm_t = requires(const T&                   t,
                                     const bhop::aes_key_t&     key,
                                     const bhop::aes_nonce_t&   nonce,
                                     const bhop::aes_auth_tag_t auth_tag,
                                     std::span<const uint8_t>   input,
                                     std::span<uint8_t>         output) {
        // Encrypt data with AES-128-CCM and get the authentication tag
        // input is the plaintext and output is the ciphertext
        { t.encrypt(key, nonce, auth_tag, input, output) };
        // Returns true if the data is decrypted and authenticated successfully, false otherwise
        // input is the ciphertext and output is the plaintext
        { t.decrypt(key, nonce, auth_tag, output, input) } -> std::same_as<bool>;
    };

    template<class T>
    concept ed25519_t = requires(const T&                         t,
                                 const bhop::ed25519_seed_t&      seed,
                                 const bhop::ed25519_key_t&       pub_key,
                                 std::span<const uint8_t>         data,
                                 const bhop::ed25519_signature_t& signature_input,
                                 bhop::ed25519_signature_t&       signature_output) {
        // Derive the public key from the node's seed
        { t.get_pub_key_from_seed(seed, pub_key) };
        // The API takes in the seed and handles the derivation of the public
        // and private keys from the seed and does the necessary operation(s)
        // Generate the (R, S) EdDSA signature
        { t.generate_signature(seed, data, signature_output) };
        // Returns true if the data and signature are verified successfully, false otherwise
        { t.verify_signature(seed, data, signature_input) } -> std::same_as<bool>;
    };

    template<class T>
    concept x25519_t = requires(const T& t, bhop::x25519_key_t& output, const bhop::x25519_key_t& input) {
        // Generate a session's ECDHE keys
        // The first parameter is the public key, the second is the private key
        { t.generate_keys(output, output) };
        // Get the shared secret from our private key and the peer's public keys
        // The first parameter is the peer's public key, the second our private key and the third the shared secret
        { t.get_shared_secret(input, input, output) } -> std::same_as<bool>;
    };

    template<class T>
    concept sha256_rng_t = requires(const T&                  t,
                                    std::span<uint8_t>        bytes,
                                    std::span<const uint8_t>  data,
                                    bhop::sha256_digest_t&    digest,
                                    const bhop::x25519_key_t& shared_secret,
                                    bhop::sha256_hkdf_t&      hkdf_output) {
        // Generate random bytes
        { t.rng_get(bytes) };
        // Hash data with SHA256
        { t.hash(data, digest) };
        // Expand the X25519 shared secret into the data to serve as the session keys
        { t.hkdf_expand(shared_secret, hkdf_output) };
    };

    template<class T, class timer_t>
    concept system_timer_t = requires(const T& t, timer_t id, void (*callback)(void*, timer_t), uint32_t timeout_ms) {
        // Get the value of the system tick in milliseconds
        { t.now_ms() } -> std::same_as<uint64_t>;
        // Register a callback to be called after a timeout. Returns the ID of the created timer.
        // The timer for the callback should started immediately.
        { t.register_callback(callback, timeout_ms) } -> std::same_as<timer_t>;
        // Cancels an already created callback with its ID
        { t.cancel_callback(id) };
    };

    template<class T>
    concept filesystem_t = requires(
        const T& t, types::file_t file, std::span<uint8_t> rw_data, std::span<const uint8_t> ro_data, uint32_t offset) {
        // Read and write into files used by this library. By the time the application calls into this
        // library, these files should have already been opened. Errors on file ops should be handled by
        // the caller since those failure modes are invisible to the library and are implementation details.
        { t.file_read(file, rw_data, offset) };
        { t.file_write(file, ro_data, offset) };
        { t.get_file_size(file) } -> std::same_as<size_t>;
    };

} // namespace bhop::platform

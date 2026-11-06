#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/types.hpp"

#include <bit>
#include <span>
#include <cstdint>
#include <concepts>


// This file contains the platform specific functions to be provided
// for this implementation of BridgeHop to function properly

// These include the transport layer (LoRa, sockets etc), AES-128-CCM, SHA256
// X25519 (ECDHE with Curve25519), Ed25519 (EdDSA) and filesystem ops.
// NOTE: The library is guaranteed to use these functions properly.


namespace bhop::platform {


    // The protocol spec enforces a byte order of little endian
    // This library only supports little endian systems, which is most microcontrollers
    static_assert(std::endian::native == std::endian::little);


    template<class T>
    concept transport_t = requires(
        T& type, std::span<const uint8_t> packet, void (*callback)(void*, std::span<const uint8_t>), void* ctx) {
        // Transfer data over the physical layer.
        // Returns true if packet sent successfully, false otherwise
        { type.transmit(packet) } -> std::same_as<bool>;
        // Since reception is an inherently asynchronous event, we register a callback that gets
        // called when a packet is received, and the app passes a span to packet to our callback
        // The library only calls this once at startup
        { type.register_recv_cb(callback, ctx) };
    };

    template<class T>
    concept aes_128_ccm_t = requires(T&                          type,
                                     const bhop::aes_key_t&      key,
                                     const bhop::aes_nonce_t&    nonce,
                                     std::span<const uint8_t>    aad,
                                     const bhop::aes_auth_tag_t& auth_tag_input,
                                     bhop::aes_auth_tag_t&       auth_tag_output,
                                     std::span<const uint8_t>    input,
                                     std::span<uint8_t>          output) {
        // Encrypt data with AES-128-CCM and get the authentication tag
        // input is the plaintext and output is the ciphertext
        { type.encrypt(key, nonce, aad, auth_tag_output, input, output) };
        // Returns true if the data is decrypted and authenticated successfully, false otherwise
        // input is the ciphertext and output is the plaintext
        { type.decrypt(key, nonce, aad, auth_tag_input, output, input) } -> std::same_as<bool>;
    };

    template<class T>
    concept ed25519_t = requires(T&                               type,
                                 const bhop::ed25519_seed_t&      seed,
                                 std::span<const uint8_t>         data,
                                 bhop::ed25519_key_t&             pub_key_output,
                                 const bhop::ed25519_key_t&       peer_pub_key,
                                 const bhop::ed25519_signature_t& signature_input,
                                 bhop::ed25519_signature_t&       signature_output) {
        // Derive the public key from the node's seed
        { type.get_pub_key_from_seed(seed, pub_key_output) };
        // The API takes in the seed and handles the derivation of the public
        // and private keys from the seed and does the necessary operation(s)
        // Generate the (R, S) EdDSA signature
        { type.generate_signature(seed, data, signature_output) };
        // Returns true if the data and signature are verified successfully, false otherwise
        { type.verify_signature(peer_pub_key, data, signature_input) } -> std::same_as<bool>;
    };

    template<class T>
    concept x25519_t = requires(T& type, bhop::x25519_key_t& output, const bhop::x25519_key_t& input) {
        // Generate a session's ECDHE keys
        // The first parameter is our public key, the second is our private key
        { type.generate_keys(output, output) };
        // Get the shared secret from our private key and the peer's public keys
        // The first parameter is the peer's public key, the second our private key and the third, the shared secret
        { type.get_shared_secret(input, input, output) } -> std::same_as<bool>;
    };

    template<class T>
    concept rng_t = requires(T& type, std::span<uint8_t> bytes) {
        // Generate random bytes
        { type.get(bytes) };
    };

    template<class T>
    concept sha256_t = requires(T&                        type,
                                std::span<const uint8_t>  data,
                                bhop::sha256_digest_t&    digest,
                                std::span<const uint8_t>  salt,
                                std::span<const uint8_t>  info_str,
                                const bhop::x25519_key_t& shared_secret,
                                bhop::sha256_hkdf_t&      hkdf_output) {
        // Hash data with SHA256
        { type.hash(data, digest) };
        // Expand the X25519 shared secret into the data to serve as the session keys
        { type.hkdf(salt, info_str, shared_secret, hkdf_output) };
    };

    // Used so systems with 32 bit system ticks don't promote to uint64_t
    template<class T>
    concept tick_t = std::same_as<T, uint32_t> || std::same_as<T, uint64_t>;

    template<class T>
    concept system_t = requires(T&                         type,
                                typename T::timer_handle_t timer,
                                void (*callback)(void*, T::timer_handle_t),
                                void*              ctx,
                                uint32_t           timeout_ms,
                                std::span<uint8_t> src) {
        // Get the value of the system tick in milliseconds
        { type.now_ms() } -> tick_t;
        // Register a callback to be called after a timeout. Returns the ID of the created timer.
        // The timer for the callback should started immediately.
        { type.register_callback(callback, ctx, timeout_ms) } -> std::same_as<typename T::timer_handle_t>;
        // Cancels an already created callback with its ID
        { type.cancel_callback(timer) };
        // Put the calling thread to sleep indefinitely till a callback wakes it up
        { type.sleep() };
        // Wipe a memory region to 0s securely. NOTE: Do NOT use memset from the stdlib
        { type.secure_memset(src) };
    };

    template<class T>
    concept filesystem_t = requires(
        T& type, types::file_t file, std::span<uint8_t> rw_data, std::span<const uint8_t> ro_data, uint32_t offset) {
        // Read and write into files used by this library. By the time the application calls into this
        // library, these files should have already been opened. Errors on file ops should be handled by
        // the caller since those failure modes are invisible to the library and are implementation details.
        { type.file_read(file, rw_data, offset) };
        { type.file_write(file, ro_data, offset) };
        { type.get_file_size(file) } -> std::same_as<size_t>;
    };

} // namespace bhop::platform

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
    concept transport = requires(
        T& type, std::span<const uint8_t> packet, void (*callback)(void*, std::span<const uint8_t>), void* ctx) {
        // Transfer data over the physical layer.
        { type.transmit(packet) };
        // Since reception is an inherently asynchronous event, we register a callback that gets
        // called when a packet is received, and the app passes a span to packet to our callback
        // The library only calls this once at startup
        { type.register_recv_cb(callback, ctx) };
    };

    template<class T>
    concept timers = requires(T&                         type,
                              typename T::timer_handle_t timer,
                              void (*callback)(void*, typename T::timer_handle_t),
                              void*    ctx,
                              uint32_t timeout_ms) {
        // Register a callback to be called after a timeout. Returns the ID of the created timer.
        // The timer for the callback should be started immediately.
        { type.register_callback(callback, ctx, timeout_ms) } -> std::same_as<typename T::timer_handle_t>;
        // Cancels and deletes an already created callback with its ID
        { type.cancel_callback(timer) };
    };

    template<class T>
    concept filesystem = requires(
        T& type, types::file_t file, std::span<uint8_t> rw_data, std::span<const uint8_t> ro_data, uint32_t offset) {
        // Read and write into files used by this library. By the time the application calls into this
        // library, these files should have already been opened. Errors on file ops should be handled by
        // the caller since those failure modes are invisible to the library and are implementation details.
        { type.read(file, rw_data, offset) };
        { type.write(file, ro_data, offset) };
        { type.get_size(file) } -> std::same_as<size_t>;
    };

    template<class T>
    concept aes_128_ccm = requires(const bhop::aes_key_t&      key,
                                   const bhop::aes_nonce_t&    nonce,
                                   std::span<const uint8_t>    aad,
                                   const bhop::aes_auth_tag_t& auth_tag_input,
                                   bhop::aes_auth_tag_t&       auth_tag_output,
                                   std::span<const uint8_t>    input,
                                   std::span<uint8_t>          output) {
        // Encrypt data with AES-128-CCM and get the authentication tag
        // input is the plaintext and output is the ciphertext
        { T::encrypt(key, nonce, aad, auth_tag_output, input, output) };
        // Returns true if the data is decrypted and authenticated successfully, false otherwise
        // input is the ciphertext and output is the plaintext
        { T::decrypt(key, nonce, aad, auth_tag_input, output, input) } -> std::same_as<bool>;
    };

    template<class T>
    concept ed25519 = requires(const bhop::ed25519_seed_t&      seed,
                               std::span<const uint8_t>         data,
                               bhop::ed25519_key_t&             pub_key_output,
                               const bhop::ed25519_key_t&       peer_pub_key,
                               const bhop::ed25519_signature_t& signature_input,
                               bhop::ed25519_signature_t&       signature_output) {
        // Derive the public key from the node's seed
        { T::get_pub_key_from_seed(seed, pub_key_output) };
        // The API takes in the seed and handles the derivation of the public
        // and private keys from the seed and does the necessary operation(s)
        // Generate the (R, S) EdDSA signature
        { T::generate_signature(seed, data, signature_output) };
        // Returns true if the data and signature are verified successfully, false otherwise
        { T::verify_signature(peer_pub_key, data, signature_input) } -> std::same_as<bool>;
    };

    template<class T>
    concept x25519 = requires(bhop::x25519_key_t& output, const bhop::x25519_key_t& input) {
        // Generate a session's ECDHE keys
        // The first parameter is our public key, the second is our private key
        { T::generate_keys(output, output) };
        // Get the shared secret from our private key and the peer's public keys
        // The first parameter is the peer's public key, the second our private key and the third, the shared secret
        // Return true if the secret is generated successfully, false otherwise
        { T::get_shared_secret(input, input, output) } -> std::same_as<bool>;
    };

    template<class T>
    concept rng = requires(std::span<uint8_t> bytes) {
        // Generate random bytes
        { T::get(bytes) };
    };

    template<class T>
    concept sha256 = requires(std::span<const uint8_t>  data,
                              bhop::sha256_digest_t&    digest,
                              std::span<const uint8_t>  salt,
                              std::span<const uint8_t>  info,
                              const bhop::x25519_key_t& ikm,
                              bhop::sha256_hkdf_t&      hkdf_output) {
        // Hash data with SHA256
        { T::hash(data, digest) };
        // Expand the X25519 shared secret into the data to serve as the session keys
        { T::hkdf(salt, info, ikm, hkdf_output) };
    };

    // Used so systems with 32 bit system ticks don't promote to uint64_t
    template<class T>
    concept tick = std::same_as<T, uint32_t> || std::same_as<T, uint64_t>;

    template<class T>
    concept system = requires(std::span<uint8_t> src) {
        typename T::tick_type_t;
        requires tick<typename T::tick_type_t>;
        // Get the value of the system tick in milliseconds
        { T::now_ms() } -> std::same_as<typename T::tick_type_t>;
        // Put the calling thread to sleep indefinitely till a callback wakes it up
        // Only used in passive node builds
        { T::sleep() };
        // Wipe a memory region to 0s securely. NOTE: Do NOT use memset from the stdlib
        { T::secure_memset(src) };
    };

} // namespace bhop::platform

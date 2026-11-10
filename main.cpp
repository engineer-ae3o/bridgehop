#include "active/node.hpp"
#include <print>
#include <cstdlib>


struct other_t {
    // concept aes_128_ccm_t
    static void encrypt(const bhop::bhop::aes_key_t&   key,
                        const bhop::bhop::aes_nonce_t& nonce,
                        std::span<const uint8_t>       aad,
                        bhop::bhop::aes_auth_tag_t&    auth_tag_input,
                        std::span<const uint8_t>       input,
                        std::span<uint8_t>             output) {
        (void)key;
        (void)nonce;
        (void)aad;
        (void)auth_tag_input;
        (void)input;
        (void)output;
    }
    static bool decrypt(const bhop::bhop::aes_key_t&      key,
                        const bhop::bhop::aes_nonce_t&    nonce,
                        std::span<const uint8_t>          aad,
                        const bhop::bhop::aes_auth_tag_t& auth_tag_input,
                        std::span<uint8_t>                output,
                        std::span<const uint8_t>          input) {
        (void)key;
        (void)nonce;
        (void)aad;
        (void)auth_tag_input;
        (void)input;
        (void)output;
        return true;
    }

    // concept ed25519_t
    static void get_pub_key_from_seed(const bhop::bhop::ed25519_seed_t& seed,
                                      bhop::bhop::ed25519_key_t&        pub_key_output) {
        (void)seed;
        (void)pub_key_output;
    }
    static void generate_signature(const bhop::bhop::ed25519_seed_t&      seed,
                                   std::span<const uint8_t>               data,
                                   const bhop::bhop::ed25519_signature_t& signature_output) {
        (void)seed;
        (void)data;
        (void)signature_output;
    }
    static bool verify_signature(const bhop::bhop::ed25519_key_t&       peer_pub_key,
                                 std::span<const uint8_t>               data,
                                 const bhop::bhop::ed25519_signature_t& signature_input) {
        (void)peer_pub_key;
        (void)data;
        (void)signature_input;
        return true;
    }

    // concept x25519_t
    static void generate_keys(bhop::bhop::x25519_key_t& pub, bhop::bhop::x25519_key_t& priv) {
        (void)pub;
        (void)priv;
    }
    static bool get_shared_secret(const bhop::bhop::x25519_key_t& peer_pub,
                                  const bhop::bhop::x25519_key_t& our_priv,
                                  bhop::bhop::x25519_key_t&       secret) {
        (void)peer_pub;
        (void)our_priv;
        (void)secret;
        return true;
    }

    // concept rng_t
    static void get(std::span<uint8_t> bytes) {
        std::ranges::fill(bytes, 0);
    }

    // concept sha256_t
    static void hash(std::span<const uint8_t> data, bhop::bhop::sha256_digest_t& digest) {
        (void)data;
        (void)digest;
    }
    static void hkdf(std::span<const uint8_t>        salt,
                     std::span<const uint8_t>        info_str,
                     const bhop::bhop::x25519_key_t& ikm,
                     bhop::bhop::sha256_hkdf_t&      hkdf_output) {
        (void)salt;
        (void)info_str;
        (void)ikm;
        (void)hkdf_output;
    }

    // concept system_t
    using tick_type_t = uint64_t;
    static tick_type_t now_ms() {
        return 0;
    }
    static void sleep() {
        while (true);
    }
    static void secure_memset(std::span<uint8_t> src) {
        for (volatile size_t i = 0; i < src.size(); i += 1) {
            src[i] = 0;
        }
    }
};

template<bhop::platform::transport  transport_t,
         bhop::platform::filesystem filesystem_t,
         bhop::platform::timers     timers_t>
using active_node_t =
    bhop::active::node_t<transport_t, filesystem_t, timers_t, other_t, other_t, other_t, other_t, other_t, other_t>;


struct stateful {
    // concept transport_t
    void transmit(std::span<const uint8_t> packet) {
        (void)packet;
    }
    void register_recv_cb(void (*callback)(void*, std::span<const uint8_t>), void* ctx) {
        (void)callback;
        (void)ctx;
    }

    // concept timers_t
    using timer_handle_t = int;
    timer_handle_t register_callback(void (*callback)(void*, timer_handle_t), void* ctx, uint32_t timeout_ms) {
        (void)callback;
        (void)ctx;
        (void)timeout_ms;
        return 0;
    }
    void cancel_callback(timer_handle_t timer) {
        (void)timer;
    }

    // concept filesystem_t
    void read(bhop::types::file_t file, std::span<uint8_t> rw_data, uint32_t offset) {
        (void)file;
        (void)rw_data;
        (void)offset;
    }
    void write(bhop::types::file_t file, std::span<const uint8_t> ro_data, uint32_t offset) {
        (void)file;
        (void)ro_data;
        (void)offset;
    }
    size_t get_size(bhop::types::file_t file) {
        (void)file;
        return 0;
    }
};

int main() {
    std::println("Starting simulator");

    active_node_t<stateful, stateful, stateful> node;
    // Start the active node
    auto ret = node.init({});
    if (ret != bhop::types::error_t::ERR_OK) {
        std::println("Failed to initialize the active node library");
        return EXIT_FAILURE;
    }

    ret = node.open({});
    if (ret != bhop::types::error_t::ERR_OK) {
        std::println("Failed to initialize the active node library");
        return EXIT_FAILURE;
    }

    ret = node.send({}, {});
    if (ret != bhop::types::error_t::ERR_OK) {
        std::println("Failed to initialize the active node library");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

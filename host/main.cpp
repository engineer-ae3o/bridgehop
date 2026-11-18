#include "active/node.hpp"


struct other_t {
    // concept aes_128_ccm_t
    static void encrypt(const bhop::bhop::aes_key_t&   key,
                        const bhop::bhop::aes_nonce_t& nonce,
                        std::span<const uint8_t>       aad,
                        bhop::bhop::aes_auth_tag_t&    auth_tag_input,
                        std::span<const uint8_t>       input,
                        std::span<uint8_t>             output) {
    }
    static bool decrypt(const bhop::bhop::aes_key_t&      key,
                        const bhop::bhop::aes_nonce_t&    nonce,
                        std::span<const uint8_t>          aad,
                        const bhop::bhop::aes_auth_tag_t& auth_tag_input,
                        std::span<uint8_t>                output,
                        std::span<const uint8_t>          input) {
        return true;
    }

    // concept ed25519_t
    static void get_pub_key_from_seed(const bhop::bhop::ed25519_seed_t& seed,
                                      bhop::bhop::ed25519_key_t&        pub_key_output) {
    }
    static void generate_signature(const bhop::bhop::ed25519_seed_t&      seed,
                                   std::span<const uint8_t>               data,
                                   const bhop::bhop::ed25519_signature_t& signature_output) {
    }
    static bool verify_signature(const bhop::bhop::ed25519_key_t&       peer_pub_key,
                                 std::span<const uint8_t>               data,
                                 const bhop::bhop::ed25519_signature_t& signature_input) {
        return true;
    }

    // concept x25519_t
    static void generate_keys(bhop::bhop::x25519_key_t& pub, bhop::bhop::x25519_key_t& priv) {
    }
    static bool get_shared_secret(const bhop::bhop::x25519_key_t& peer_pub,
                                  const bhop::bhop::x25519_key_t& our_priv,
                                  bhop::bhop::x25519_key_t&       secret) {
        return true;
    }

    // concept rng_t
    static void get(std::span<uint8_t> bytes) {
        for (auto& byte : bytes) {
            byte = 0;
        }
    }

    // concept sha256_t
    static void hash(std::span<const uint8_t> data, bhop::bhop::sha256_digest_t& digest) {
    }
    static void hkdf(std::span<const uint8_t>        salt,
                     std::span<const uint8_t>        info_str,
                     const bhop::bhop::x25519_key_t& shared_secret,
                     bhop::bhop::sha256_hkdf_t&      hkdf_output) {
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

template<bhop::platform::transport transport, bhop::platform::filesystem filesystem, bhop::platform::timers timers>
using active_node_t =
    bhop::active::node_t<transport, filesystem, timers, other_t, other_t, other_t, other_t, other_t, other_t>;


struct stateful {
    // concept transport_t
    void transmit(std::span<const uint8_t> packet) {
    }
    void register_recv_cb(void (*callback)(void*, std::span<const uint8_t>), void* ctx) {
    }

    // concept timers_t
    using timer_handle_t = int;
    timer_handle_t register_callback(void (*callback)(void*, timer_handle_t), void* ctx, uint32_t timeout_ms) {
        return 0;
    }
    void cancel_callback(timer_handle_t timer) {
    }

    // concept filesystem_t
    void read(bhop::types::file_t file, std::span<uint8_t> rw_data, uint32_t offset) {
    }
    void write(bhop::types::file_t file, std::span<const uint8_t> ro_data, uint32_t offset) {
    }
    size_t get_size(bhop::types::file_t file) {
        return 0;
    }
};

int main() {
    active_node_t<stateful, stateful, stateful> node;
    // Start the active node
    auto ret = node.init({});
    ret      = node.open({});
    ret      = node.send({}, {});
}

#pragma once


#include "utils/bhop_constants.hpp"
#include "utils/common.hpp"
#include "utils/config.hpp"

#include <cstdint>


namespace bhop::types {

    // All error codes used by this library
    enum class [[nodiscard]] error_t : uint8_t {
        ERR_OK = 0,
        ERR_INVALID_ARG,
        ERR_INVALID_STATE,
        ERR_SESSION_ALREADY_OPEN,
        ERR_SESSION_FAILED_TO_OPEN,
        ERR_CONTACT_ALREADY_EXISTS,
        ERR_CONTACT_NOT_EXIST,
        ERR_CONTACT_LIST_FULL,
        ERR_BUSY,
    };

    [[nodiscard]] constexpr const char* error_code_to_string(error_t error) {
        using enum error_t;
        switch (error) {
            case ERR_OK:
                return "ERR_OK";
            case ERR_INVALID_ARG:
                return "ERR_INVALID_ARG";
            case ERR_INVALID_STATE:
                return "ERR_INVALID_STATE";
            case ERR_SESSION_ALREADY_OPEN:
                return "ERR_SESSION_ALREADY_OPEN";
            case ERR_SESSION_FAILED_TO_OPEN:
                return "ERR_SESSION_FAILED_TO_OPEN";
            case ERR_CONTACT_ALREADY_EXISTS:
                return "ERR_CONTACT_ALREADY_EXISTS";
            case ERR_CONTACT_NOT_EXIST:
                return "ERR_CONTACT_NOT_EXIST";
            case ERR_CONTACT_LIST_FULL:
                return "ERR_CONTACT_LIST_FULL";
            case ERR_BUSY:
                return "ERR_BUSY";
            default:
                return "ERR_UNKNOWN";
        }
    }

    // Reasons for ending a session
    enum class close_reason_t : uint8_t {
        CLOSED_BY_USER       = 0, // The session was closed by the application
        IDLE_TIMEOUT         = 1, // The idle timeout and no message packet(s) was sent/received
        EVICTED_FOR_SPACE    = 2, // The library ran out of memory and evicted a session to use for another
        CONFIRMATION_TIMEOUT = 3, // The timeout for waiting for a message ack expired
        OTHER                = 4, // Other/unknown/unspecifed reasons
    };

    // Represents the type of a packet and how to interpret it
    enum class packet_t : uint8_t {
        DATA        = 0b00, // Packet is for regular message ciphertext chunks
        HANDSHAKE   = 0b01, // Packet is for a handshake
        MESSAGE_ACK = 0b10, // Packet is for a message delivery confirmation
        END_SESSION = 0b11, // Packet is to close a sesson by a peer
    };

    // The files being used by the protocol
    enum class file_t : uint8_t {
        CONFIG,   // Stores configuration data
        CONTACTS, // Stores all registered contacts and their handshake counter
    };

    // Event type passed to user callback
    enum class event_t : uint8_t {};

    // Data stored in the config file
    struct config_file_t {};


    // An efficient bitmask representing the contact slots that are in use
    // Space efficient and stops us from having to do a linear scan of the file
    using contacts_file_header_t = std::array<uint8_t, utils::ceil_div(config::MAX_CONTACTS, 8)>;

    // The contacts file stores the number of contacts first, then the contacts. A contact slot stores whether
    // it is in use, the node ID of the contact and the number of valid handshakes with said contact. This allows
    // us to freely delete contact. Deletion would fragment the file storage, and you'd have to push elements up
    // and/or down if there was no way to know if a contact slot was in use, and this can be convoluted to implement.
    struct contact_slot_t {
        bhop::node_id_t node_id{};
        uint64_t        handshake_counter{};
    };

} // namespace bhop::types

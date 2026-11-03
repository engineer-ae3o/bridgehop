#pragma once


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
        ERR_CONTACT_NOT_NON_EXISTENT,
    };

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
        TEMP,     // Used to store data temporarily when the application needs memory
    };

} // namespace bhop::types

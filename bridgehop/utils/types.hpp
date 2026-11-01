#pragma once


#include <array>
#include <utility>
#include <cstdint>


namespace bhop::types {

    // Reasons for ending a session
    enum class close_reason_t : uint8_t {
        CLOSED_BY_USER       = 0,
        IDLE_TIMEOUT         = 1,
        EVICTED_FOR_SPACE    = 2,
        CONFIGMATION_TIMEOUT = 3,
    };

    enum class packet_t : uint8_t {
        DATA        = 0b00,
        HANDSHAKE   = 0b01,
        MESSAGE_ACK = 0b10,
        END_SESSION = 0b11,
    };

} // namespace bhop::types

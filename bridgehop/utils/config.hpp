#pragma once


#include <cstdint>


namespace bhop::config {


    // This file contains configurable constants used by this implementation of BridgeHop.
    // They can modified safely.


    // Maxmimum active sessions at any given point
    constexpr inline uint32_t MAX_ACTIVE_SESSIONS = 8;

    // Maximum number of contacts that can be saved.
    // Note that deletion of contacts is not supported
    constexpr inline uint32_t MAX_CONTACTS = 500;


} // namespace bhop::config

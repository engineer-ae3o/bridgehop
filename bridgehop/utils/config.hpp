#pragma once


#include "utils/bhop_constants.hpp"

#include <cstdint>


namespace bhop::config {


    // This file contains configurable constants used by this implementation of BridgeHop.
    // They can be freely modified.


    // Maximum hops per packet and dedupe cache parameters
    constexpr inline uint32_t MAX_HOPS_PER_PACKET           = 10;
    constexpr inline uint32_t DEDUPE_CACHE_SIZE             = 2048;
    constexpr inline uint32_t DEDUPE_CACHE_ENTRY_TIMEOUT_MS = 120 * 1000;

    // Maxmimum active sessions at any given point and maxmimum chunked packets allowed for a given message
    constexpr inline uint32_t MAX_ACTIVE_SESSIONS    = 8;
    constexpr inline uint32_t MAX_CHUNKS_PER_MESSAGE = 16;

    // To be filled by the user for the maximum size of a packet allowed by the physical layer
    constexpr inline uint32_t MAX_PACKET_SIZE = 255;

    // The maximum message size allowed
    constexpr inline uint32_t MAX_DATA_SIZE_PER_PACKET = MAX_PACKET_SIZE - bhop::DATA_PACKET_METADATA_SIZE;
    constexpr inline uint32_t MAX_MESSAGE_SIZE_BYTES   = MAX_CHUNKS_PER_MESSAGE * MAX_DATA_SIZE_PER_PACKET;

    // Maximum number of contacts that can be saved.
    constexpr inline uint32_t MAX_CONTACTS = 500;

    // Before a packet is retransmitted, it is made to wait a random delay between this interval
    constexpr inline uint32_t PROPAGATION_DELAY_MIN_MS = 0;
    constexpr inline uint32_t PROPAGATION_DELAY_MAX_MS = 250;

    // Various timeouts for different states
    constexpr inline uint32_t HANDSHAKE_TIMEOUT_MS    = 60 * 1000;
    constexpr inline uint32_t SESSION_IDLE_TIMEOUT_MS = 60 * 60 * 1000;
    constexpr inline uint32_t REASSEMBLY_TIMEOUT_MS   = 45 * 1000;
    constexpr inline uint32_t MESSAGE_ACK_TIMEOUT_MS  = 120 * 1000;


    static_assert(MAX_PACKET_SIZE > bhop::HANDSHAKE_FULL_PACKET_SIZE);
    static_assert(MAX_PACKET_SIZE > bhop::DATA_PACKET_METADATA_SIZE);
    static_assert(MAX_PACKET_SIZE > bhop::MESSAGE_ACK_FULL_PACKET_SIZE);
    static_assert(MAX_PACKET_SIZE > bhop::CLOSE_FULL_PACKET_SIZE);


} // namespace bhop::config

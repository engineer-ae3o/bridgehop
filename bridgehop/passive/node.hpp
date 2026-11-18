#pragma once


namespace bhop::passive {

    class node_t {
        // This function assumes it has the calling thread to itself.
        // So all hardware and necessary resources should be initialized
        // and running before this is called. It sets up the library and
        // starts listening for any packets.
        // NOTE: It is mutually exclusive with the API from the active namespace
        [[noreturn]] void run();
    };

} // namespace bhop::passive

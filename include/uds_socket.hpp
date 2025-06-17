#pragma once
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <random>
#include <chrono>
#include <thread>
#include <map>
#include <string>
#include <fcntl.h>
#include <poll.h>
#include <atomic>

#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/counter.h>

namespace monitor
{
    class UdsSocket {
    public:
        UdsSocket(const std::string& bind_path,
                  prometheus::Family<prometheus::Counter>& counter_family);

        ~UdsSocket();

        bool setup();

        void add_target(const std::string& name, const std::string& path);

        bool send_to(const std::string& target_name);

        void start_receiving();

        void stop_receiving();

    private:
        std::string generate_random_message();

        int _socket_fd;
        const std::string& _bind_path;
        prometheus::Family<prometheus::Counter>& _counter_family;
        prometheus::Counter* _tx_counter;
        prometheus::Counter* _rx_counter;
        std::map<std::string, sockaddr_un> _targets;
        std::thread _recv_thread;
        std::atomic<bool> _receiving;
    };
}

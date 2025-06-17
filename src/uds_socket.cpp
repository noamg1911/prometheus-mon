#include "uds_socket.hpp"

namespace monitor
{
    UdsSocket::UdsSocket(const std::string& bind_path,
                         prometheus::Family<prometheus::Counter>& counter_family) :
    _socket_fd(-1),
    _bind_path(bind_path),
    _counter_family(counter_family),
    _receiving(false)
    {}

    bool UdsSocket::setup()
    {
        _socket_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
        if (_socket_fd < 0)
        {
            perror("socket");
            return false;
        }

        unlink(_bind_path.c_str());
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, _bind_path.c_str(), sizeof(addr.sun_path) - 1);

        if (bind(_socket_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        {
            perror("bind");
            return false;
        }

        _tx_counter = &_counter_family.Add({{"dir", "tx"}, {"socket", _bind_path}});
        _rx_counter = &_counter_family.Add({{"dir", "rx"}, {"socket", _bind_path}});

        return true;
    }

    void UdsSocket::add_target(const std::string& name, const std::string& path)
    {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
        _targets[name] = addr;
    }

    bool UdsSocket::send_to(const std::string& target_name)
    {
        if (_targets.count(target_name) == 0)
        {
            printf("Unknown target %s\n", target_name.c_str());
            return false;
        }

        std::string message = this->generate_random_message();
        const sockaddr_un& addr = _targets[target_name];
        int send_return_code = sendto(_socket_fd, message.c_str(), message.size(), 0,
                                      reinterpret_cast<const sockaddr*>(&addr), sizeof(addr));
        if (send_return_code < 0)
        {
            // perror("sendto");
            printf("Error %d sending!\n", send_return_code);
            return false;
        }

        _tx_counter->Increment();
        printf("Sent to %s\n", target_name.c_str());
        return true;
    }

    void UdsSocket::start_receiving()
    {
        _receiving = true;
        _recv_thread = std::thread([this]() {
            char buffer[256]{};
            struct pollfd pfd;
            pfd.fd = _socket_fd;
            pfd.events = POLLIN;

            while (_receiving) {
                int ret = poll(&pfd, 1, 500); // 500ms timeout
                if (ret > 0 && (pfd.revents & POLLIN)) {
                    ssize_t n = recvfrom(_socket_fd, buffer, sizeof(buffer) - 1, 0, nullptr, nullptr);
                    if (n > 0) {
                        buffer[n] = '\0';
                        _rx_counter->Increment();
                        printf("received %s\n", buffer);
                    }
                }
            }
        });
    }

    void UdsSocket::stop_receiving()
    {
        _receiving = false;
        if (_recv_thread.joinable())
        {
            _recv_thread.join();
        }
    }

    UdsSocket::~UdsSocket()
    {
        this->stop_receiving();
        if (_socket_fd >= 0)
        {
            close(_socket_fd);
        }
        unlink(_bind_path.c_str());
    }

    std::string UdsSocket::generate_random_message()
    {
        static const char charset[] = "abcdefghijklmnopqrstuvwxyz";
        static std::default_random_engine eng(std::random_device{}());
        static std::uniform_int_distribution<int> len_dist(1, 20);
        static std::uniform_int_distribution<int> char_dist(0, sizeof(charset) - 2);

        int len = len_dist(eng);
        std::string message;
        for (int i = 0; i < len; ++i)
        {
            message += charset[char_dist(eng)];
        }

        message += '\0';
        return message;
    }
} // namespace monitor

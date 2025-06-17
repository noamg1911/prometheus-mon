#include <prometheus/exposer.h>
#include <prometheus/registry.h>
#include <prometheus/counter.h>
#include <prometheus/gauge.h>
#include <thread>
#include <chrono>
#include <memory>
#include "uds_socket.hpp"

using namespace monitor;

int main()
{
    prometheus::Exposer exposer{"0.0.0.0:9091"};
    auto registry = std::make_shared<prometheus::Registry>();

    prometheus::Family<prometheus::Counter>& counter_family = prometheus::BuildCounter()
                                                                            .Name("uds_received_messages_total")
                                                                            .Help("UDS socket receiving traffic counts")
                                                                            .Register(*registry);

    exposer.RegisterCollectable(registry);

    const std::string address1 = "/tmp/uds1.sock";
    const std::string address2 = "/tmp/uds2.sock";

    UdsSocket uds1(address1, counter_family);
    UdsSocket uds2(address2, counter_family);

    uds1.setup();
    uds2.setup();
    
    uds1.add_target("uds2", address2);
    uds2.add_target("uds1", address1);
    
    uds1.start_receiving();
    uds2.start_receiving();

    for (int i = 0; i < 100; ++i) {
        uds1.send_to("uds2");
        uds2.send_to("uds1");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
    // int curState = 0;
    // while (true) {
    //     // uds1.work();
    //     // uds2.work();
    //     state.Set(curState % 3);
    //     curState++;
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    // return 0;
}

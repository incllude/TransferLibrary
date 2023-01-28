#include "transfer/service.h"


transfer::service* transfer::service::get() {

    static auto &&ser = service();
    return &ser;
}

boost::asio::io_context* transfer::service::get_io() {

    static auto &&ser = service();
    return &(ser.io);
}

void transfer::service::run() {

    auto s = service::get_io();
    s->run();
}
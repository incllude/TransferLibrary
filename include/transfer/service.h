#ifndef SERVICE_TRANSFER_H
#define SERVICE_TRANSFER_H
#define BOOST_BIND_GLOBAL_PLACEHOLDERS

/*! \file service.h
 *  \brief Определение класса service
 */

#include <map>
#include <boost/asio.hpp>

namespace transfer {

    /*! \class service
     *  \brief Внутренний класс для корректной работы классов sender и receiver\n Не предполагает использование пользователем
     */

    class service {
    private:

        service() = default;
        boost::asio::io_context io;

    public:

        static void run();
        static service* get();
        static boost::asio::io_context* get_io();
    };
}

#endif

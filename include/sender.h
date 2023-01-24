#ifndef TRANSFER_SENDER_H
#define TRANSFER_SENDER_H

/*! \file sender.h
 *  \brief Определение класса sender
 */

#include "sender_session.h"
#include "service.h"
#include "logger.h"
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>

namespace transfer{

    /*! \class sender
     *  \brief Класс с методами для отправки файлов и папок\n Предполагает использование пользователем\n Работает только с run()
     */

    template <uint64_t size_of_pack> class sender {
    private:

#ifdef __linux__
        char slash = '/';
#elif defined(_WIN32) || defined(WIN32)
        char slash = '\\';
#endif

        logger log = logger("send");
        void do_accept(const bool with_delay, const std::string address, const std::string way);

    public:

        void send(const std::string ip_v4_address, const std::string way_to_file);
        void send_safe(const std::string ip_v4_address, const std::string way_to_file);
        void enable_logging();
        void disable_logging();
        void set_log_level_info();
        void set_log_level_error();

        /*! \brief Конструктор по умолчанию, включающий логирование ошибок и информацию об отправке данных
         */
        sender() {

            if (size_of_pack == 0) throw std::invalid_argument("Pack's size can not equal to 0");
            enable_logging();
        }
    };
}

/*! \brief Позволяет отправить файл или папку, содержащую другие объекты\n Не гарантирует, что все объекты будут доставлены в целости
 */

template <uint64_t n> void transfer::sender<n>::send(const std::string ip_v4_address, const std::string way_to_file) {

    std::thread thread(&sender::do_accept, this, false, ip_v4_address, way_to_file);
    thread.join();
}

/*! \brief Позволяет отправить файл или папку, содержащую другие объекты\n Безопасно доставляет объекты
 */

template <uint64_t n> void transfer::sender<n>::send_safe(const std::string ip_v4_address, const std::string way_to_file) {

    std::thread thread(&sender::do_accept, this, true, ip_v4_address, way_to_file);
    thread.join();
}

/*! \brief Включает логирование
 */

template <uint64_t n> void transfer::sender<n>::enable_logging() {

   log.turn_on();
}

/*! \brief Выключает логирование
 */

template <uint64_t n> void transfer::sender<n>::disable_logging() {

   log.turn_off();
}

/*! \brief После использования этого метода в log файле будут показаны только ошибки
 */

template <uint64_t n> void transfer::sender<n>::set_log_level_error() {

    log.set_minimum_level_error();
}

/*! \brief После использования этого метода в log файле будут отображаться ошибки и информация об отправке данных
 */

template <uint64_t n> void transfer::sender<n>::set_log_level_info() {

    log.set_minimum_level_info();
}

template <uint64_t n> void transfer::sender<n>::do_accept(const bool with_delay, const std::string address, const std::string way) {

    std::make_shared<transfer::sender_session<n>>(&log, with_delay, slash)->send_object(address, way);
}

#endif

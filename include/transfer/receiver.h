#ifndef TRANSFER_RECEIVER_H
#define TRANSFER_RECEIVER_H

/*! \file receiver.h
 *  \brief Определение класса receiver
 */

#include "receiver_session.h"
#include "service.h"
#include "logger.h"

namespace transfer {

    /*! \class receiver
     *  \brief Класс с методами для получения файлов и папок\n Предполагает использование пользователем\n Работает только с run()
     */

    template <uint64_t size_of_pack> class receiver {
    private:

#ifdef __linux__
        char slash = '/';
#elif defined(_WIN32) || defined(WIN32)
        char slash = '\\';
#endif

        short port;
        std::string default_add;
        std::string default_way;
        ip::tcp::acceptor acceptor;
        logger log = logger("receive");

        void do_accept(const std::string add, const std::string way);
        void check_way(std::string& way);

    public:

        void receive(const std::string add, const std::string way);
        void receive(const std::string add);
        void receive();
        void enable_logging();
        void disable_logging();
        void set_log_level_info();
        void set_log_level_error();
        void set_way(const std::string way);
        void set_add(const std::string add);

        /*!
         * \brief Коснтруктор с 2 аргументами, включающий регистрацию ошибок и информацию об отправке данных
         * @param size_of_pack - неизменяемое количество байт, которое будет записано за один раз
         * @param port - порт на котором будет построено соединение
         * @param way - путь, в который будут сохранены полученные объекты
         */

        receiver(const short port, const std::string way) :
                acceptor(*(service::get_io()), ip::tcp::endpoint(tcp::v4(), port)), port(port) {
            
            acceptor.close();

            if (size_of_pack == 0) throw std::invalid_argument("Pack's size can not equal to 0");
            std::string cur_way = way;
            check_way(cur_way);
            default_way = cur_way;
            enable_logging();
        }

        /*!
         * \brief Коснтруктор с 3 аргументами, включающий регистрацию ошибок и информацию об отправке данных
         * @param size_of_pack - неизменяемое количество байт, которое будет записано за один раз
         * @param port - порт на котором будет построено соединение
         * @param way - путь, в который будут сохранены полученные объекты
         * @param add - добавочнаяя часть, которая будет записана в конце имени каждого полученного файла. Если это папка, имя папки не изменится
         */
        receiver(const short port, const std::string way, const std::string add) :
                acceptor(*(service::get_io()), ip::tcp::endpoint(tcp::v4(), port)), default_add(add), port(port) {

            acceptor.close();

            if (size_of_pack == 0) throw std::invalid_argument("Pack's size can not equal to 0");
            std::string cur_way = way;
            check_way(cur_way);
            default_way = cur_way;
            enable_logging();
        }
    };
}

/*! \brief Позволяет получить 1 файл или чистую папку без файлов в ней
 */

template <uint64_t n> void transfer::receiver<n>::receive() {

    receive(default_add, default_way);
}

/*! \brief Позволяет получить 1 файл или чистую папку без файлов в ней
 *  @param add - добавочная часть, которая будет записана в конце имени только этого полученного файла. Если это папка, имя папки не изменится
 */

template <uint64_t n> void transfer::receiver<n>::receive(const std::string add) {

    receive(add, default_way);
}

/*! \brief Позволяет получить 1 файл или чистую папку без файлов в ней
 *  @param add - добавочная часть, которая будет записана в конце имени полученного файла только в этот раз. Если это папка, имя папки не изменится
 *  @param way - путь, в который будут сохранены полученный объект только в этот раз
 */

template <uint64_t n> void transfer::receiver<n>::receive(const std::string add, const std::string way) {

    std::string cur_way = way;
    if (cur_way != default_way) check_way(cur_way);
    do_accept(add, cur_way);
}

/*! \brief Включает логирование
 */

template <uint64_t n> void transfer::receiver<n>::enable_logging() {

    log.turn_on();
}

/*! \brief Выключает логирование
 */

template <uint64_t n> void transfer::receiver<n>::disable_logging() {

    log.turn_off();
}

/*! \brief После использования этого метода в log файле будут показаны только ошибки
 */

template <uint64_t n> void transfer::receiver<n>::set_log_level_error() {

    log.set_minimum_level_error();
}

/*! \brief После использования этого метода в log файле будут отображаться ошибки и информация об отправке данных
 */

template <uint64_t n> void transfer::receiver<n>::set_log_level_info() {

    log.set_minimum_level_info();
}

/*! \brief Изменение пути по умолчанию для полученных объектов
 *  @param way - путь, в который будут сохранены полученные объекты по умолчанию
 */

template <uint64_t n> void transfer::receiver<n>::set_way(const std::string way) {

    check_way(way);
    default_way = way;
}

/*! \brief Изменение добавочной части по умолчанию для полученных объектов
 *  @param add - добавочная часть, которая будет записана в конце имени каждого полученного файла. Если это папка, имя папки не изменится
 */

template <uint64_t n> void transfer::receiver<n>::set_add(const std::string add) {

    default_add = add;
}

template <uint64_t n> void transfer::receiver<n>::check_way(std::string& way) {

    if (way.back() == '/' || way.back() == '\\') way = way.substr(0, way.size() - 1);
    std::string error = "Invalid way was received";

    int sl = way.rfind(slash);
    if (sl == way.npos) throw std::invalid_argument(error);
    try {
        directory_iterator object(way.substr(0, sl)), end;
        for (; object != end && object->path().string() != way; object++) {}
        if (object == end || !is_directory(*object)) throw std::invalid_argument(error);
    }
    catch (std::exception e) {
        log.write_error(way, error);
        throw std::invalid_argument(error);
    }
}

template <uint64_t n> void transfer::receiver<n>::do_accept(const std::string add, const std::string way) {
    if (!acceptor.is_open()) acceptor.operator=(ip::tcp::acceptor(*(service::get_io()), ip::tcp::endpoint(tcp::v4(), port)));

    acceptor.async_accept([this, add, way](boost::system::error_code error, ip::tcp::socket socket) {
        if (!error)
            std::make_shared<transfer::receiver_session<n>>(std::move(socket), way, &log, add, &acceptor, slash)->run();
        if (error) {
            std::cout << error.message() << '\n';
            do_accept(add, way);
        }
    });
}

#endif

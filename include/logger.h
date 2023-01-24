#ifndef TRANSFER_LOGGER_H
#define TRANSFER_LOGGER_H

/*! \file logger.h
 *  \brief Определение класса logger
 */

#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace transfer {

    /*! \class logger
     *  \brief Внутренний класс для логирования\n Может быть использован пользователем
     */

    class logger {
    private:

        bool can = false;
        std::string name = "";
        void init_logging();

    public:

        logger(const std::string name) : name(name) {

            init_logging();
        }
        void turn_on();
        void turn_off();
        void set_minimum_level_info();
        void set_minimum_level_error();
        void write_info(const std::string where, const std::string message);
        void write_error(const std::string where, const std::string message);
    };
}

#endif

#include "transfer/logger.h"


void transfer::logger::init_logging() {

      boost::log::register_simple_formatter_factory<boost::log::trivial::severity_level, char>("Severity");
      boost::log::add_file_log(boost::log::keywords::file_name = (name + ".log"), boost::log::keywords::format = "[%TimeStamp%] [%Severity%] %Message%");
      boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
      boost::log::add_common_attributes();
}

void transfer::logger::turn_on() {

    can = true;
}

void transfer::logger::turn_off() {
    
    can = false;
}

void transfer::logger::write_error(const std::string where, const std::string message) {

    if (can) BOOST_LOG_TRIVIAL(error) << "[in " << where << "] " << message;
}

void transfer::logger::write_info(const std::string where, const std::string message) {
    
    if (can) BOOST_LOG_TRIVIAL(info) << "[in " << where << "] " << message;
}

void transfer::logger::set_minimum_level_info() {

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

void transfer::logger::set_minimum_level_error() {

    boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

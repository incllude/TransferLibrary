#ifndef TRANSFER_SESSION_H
#define TRANSFER_SESSION_H

/*! \file receiver_session.h
 *  \brief Определение класса receiver_session
 */

#include <set>
#include <fstream>
#include <stdint.h>
#include <iostream>
#include "logger.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>


using namespace boost::asio;
using namespace boost::system;
using namespace boost::asio::ip;
using namespace boost::filesystem;

namespace transfer {

    /*! \class receiver_session
     *  \brief Внутренний класс для корректной работы класса receiver\n Не предполагает использование пользователем
     */

    template <uint64_t size_of_pack> class receiver_session : public std::enable_shared_from_this<receiver_session<size_of_pack>> {
    private:

        boost::asio::streambuf buffer;
        boost::array<char, size_of_pack> pack;
        ip::tcp::acceptor* accept;
        ip::tcp::socket socket;
        std::ofstream output_file;
        std::string default_way;
        std::string extension;
        int file_size;
        char slash;
        logger* log;
        std::string type;
        std::string name;
        std::string mode;
        std::string add;
        std::string way;

    public:

        receiver_session(ip::tcp::socket socket, const std::string way, logger* log, const std::string ad, ip::tcp::acceptor* ac, const char s) :
                socket(std::move(socket)), default_way(way), log(log), add(ad), accept(ac), slash(s) {}
        void run();

    private:

        void write_info(const std::string message);
        void write_error(const std::string message);
        void wait_for_request();
        void read_buffer(const boost::system::error_code error);
        void make_folder();
        void handle_read_pack();
        void read_pack(const boost::system::error_code error, size_t bytes);
        void retry(const boost::system::error_code error);
        std::string name_correction(const std::string& way, const std::string& name, const std::string& extension);

    };
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::write_error(const std::string message) {

    log->write_error(name + (extension.empty() ? "" : "." + extension), message);
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::write_info(const std::string message) {

    log->write_info(name + (extension.empty() ? "" : "." + extension), message);
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::run() {

    write_info("Creating session on: " + socket.remote_endpoint().address().to_string() + '\n');
    wait_for_request();
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::wait_for_request() {

    async_read_until(socket, buffer, "\n\n", boost::bind(&receiver_session::read_buffer, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::read_buffer(const boost::system::error_code error) {

    if (error) {
        socket.close();
        return;
    }

    char sep;
    std::string str_file_size;
    std::istream request_stream(&buffer);
    std::getline(request_stream, mode);
    std::getline(request_stream, type);
    std::getline(request_stream, way);
    std::getline(request_stream, name);
    std::getline(request_stream, extension);
    std::getline(request_stream, str_file_size);
    request_stream >> sep >> sep;

    file_size = stoi(str_file_size);
    if (way == "`") way = "";
    if (name == "`") name = "";
    if (extension == "`") extension = "";
    if (type == "regular") name = name + add;

    write_info("Start writing");
    if (type == "regular") handle_read_pack();
    if (type == "folder") make_folder();
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::make_folder() {

    boost::filesystem::create_directory(default_way + way + slash + name);
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::handle_read_pack() {

    output_file.open(default_way + way + slash + name + (extension.empty() ? "" : "." + extension),
                     std::ios_base::binary | std::ios_base::out | (mode == "start_writing" ? std::ios_base::trunc : std::ios_base::app));

    async_read(socket, boost::asio::buffer(pack.c_array(), pack.size()),
               boost::bind(&receiver_session::read_pack, this->shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::read_pack(const boost::system::error_code error, size_t got_bytes) {

    if (error && error.message() != "End of file") {
        write_error(error.message());
        socket.close();
        write_error(error.message());
        accept->async_accept(socket, boost::bind(&receiver_session::retry, this->shared_from_this(), boost::asio::placeholders::error));
        return;
    }

    if (got_bytes > 0) {
        try {
            output_file.write(pack.c_array(), got_bytes);
            write_info("Write " + std::to_string(got_bytes) + " bytes, total " + std::to_string(output_file.tellp()) + " bytes");
        }
        catch (std::exception e) {
            write_error(e.what());
            return;
        }

        if (output_file.tellp() >= file_size || error.message() == "End of file") {
            socket.close();
            return;
        }
    }

    async_read(socket, boost::asio::buffer(pack.c_array(), pack.size()),
               boost::bind(&receiver_session::read_pack, this->shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

template <uint64_t bytes> void transfer::receiver_session<bytes>::retry(const boost::system::error_code error) {

    if (!error) {
        output_file.close();
        this->shared_from_this()->run();
    }
}

#endif

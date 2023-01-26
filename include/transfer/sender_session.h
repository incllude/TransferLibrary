#ifndef TRANSFER_SENDER_SESSION_H
#define TRANSFER_SENDER_SESSION_H

/*! \file sender_session.h
 *  \brief Определение класса sender_session
 */

#include <fstream>
#include <stdint.h>
#include "logger.h"
#include "service.h"
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/filesystem.hpp>


using namespace boost::asio;
using namespace boost::system;
using namespace boost::filesystem;

namespace transfer {

    /*! \class sender_session
     *  \brief Внутренний класс для корректной работы класса sender\n Не предполагает использование пользователем
     */

    template <uint64_t size_of_pack> class sender_session : public std::enable_shared_from_this<sender_session<size_of_pack>> {
    private:

        recursive_directory_iterator start;
        recursive_directory_iterator end;
        boost::array<char, size_of_pack> buf;
        std::string final_path;
        std::string extension;
        std::string add_way;
        std::string type;
        std::string name;
        std::string mode;
        std::string way;
        std::string ip;
        logger* log;
        char slash;
        int port;
        int now_size;
        int file_size;
        int delay {0};
        ip::tcp::socket socket;
        std::ifstream source_file;
        boost::asio::streambuf request;

    public:

        sender_session(logger* l, const bool with_delay, const char s) : socket(*service::get_io()), log(l), slash(s) {

            if (with_delay) delay = 10;
        }
        void send_object(const std::string address, const std::string file_way);

    private:

        void go_to_next();
        void set_closing_way();
        void write_info(const std::string& message);
        void write_error(const std::string& message);
        void send_again(boost::system::error_code error);
        void retry_connect(void (sender_session::*func)(boost::system::error_code));
        void send_after_error(boost::system::error_code error, void (sender_session::*func)(boost::system::error_code));
        void send_file(recursive_directory_iterator file);
        void handle_on_write(boost::system::error_code error);
        void handle_write_data(boost::system::error_code error);
        void write_data(boost::system::error_code error);
        void write_folder(const boost::system::error_code error);
        void init(const std::string& address, const std::string& file);
        void check(const std::string& address, const int& port);
        std::map<std::string, std::string> get_file(const std::string& file);
        static std::string get_ip(const std::string& address);
        static std::string get_port(const std::string& address);
        void set_milliseconds();
        int check_way();
    };
}

template <uint64_t n> void transfer::sender_session<n>::write_error(const std::string& message) {

    log->write_error(name + (extension.empty() ? "" : "." + extension), message);
}

template <uint64_t n> void transfer::sender_session<n>::write_info(const std::string& message) {

    log->write_info(name + (extension.empty() ? "" : "." + extension), message);
}

template <uint64_t n> void transfer::sender_session<n>::retry_connect(void (sender_session::*func)(boost::system::error_code)) {

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    write_error("Trying to connect");
    ip::tcp::endpoint endpoint(ip::address::from_string(ip), port);
    socket.async_connect(endpoint, boost::bind(&sender_session::send_after_error, this->shared_from_this(), boost::asio::placeholders::error, func));
}

template <uint64_t n> void transfer::sender_session<n>::send_after_error(boost::system::error_code error, void (sender_session::*func)(boost::system::error_code)) {

    if (error) {
        write_error(error.message());
        retry_connect(func);
        return;
    }

    std::ostream req_stream(&request);
    req_stream << mode << '\n' << type << '\n' << (add_way.empty() ? "`" : add_way) << '\n' << name << '\n' << (extension.empty() ? "`" : extension) << '\n' << file_size << "\n\n";

    async_write(socket, request, boost::bind(func, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t n> void transfer::sender_session<n>::send_again(boost::system::error_code error) {

    if (error) {
        void (sender_session::*func)(boost::system::error_code);
        write_error(error.message());
        func = &sender_session::send_again;
        retry_connect(func);
        return;
    }
    write_info("Send " + std::to_string(source_file.gcount()) + " bytes, total " + std::to_string(now_size) + " bytes again");
    async_write(socket, buffer(buf.c_array(), source_file.gcount()), boost::bind(&sender_session::write_data, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t n> void transfer::sender_session<n>::go_to_next() {

    source_file.close();
    start++;
    send_file(start);
}

template <uint64_t n> void transfer::sender_session<n>::send_object(const std::string address, const std::string file_way) {

    init(address, file_way);

    set_closing_way();
    send_file(start);
}

template <uint64_t n> void transfer::sender_session<n>::set_closing_way() {

    start = recursive_directory_iterator(way);
    directory_iterator next(way), d_end;
    for (; start != end && start->path().string() != way + slash + name + (extension.empty() ? "" : '.' + extension); start++)
        if (next->path().string() == start->path().string()) next++;
    next++;
    final_path = next == d_end ? "" : next->path().string();
}

template <uint64_t n> void transfer::sender_session<n>::send_file(recursive_directory_iterator file) {

    std::ostream req_stream(&request);
    ip::tcp::endpoint endpoint(ip::address::from_string(ip), port);
    socket = ip::tcp::socket(*service::get_io());

    if (file == end || file->path().string() == final_path) return;

    if (is_directory(*file)) type = "folder";
    if (is_regular_file(*file)) type = "regular";
    auto parts = get_file(file->path().string());
    add_way = parts["way"].substr(way.size());
    extension = parts["extension"];
    mode = "start_writing";
    name = parts["name"];
    now_size = 0;

    source_file.open(parts["way"] + "/" + name + (extension.empty() ? "" : "." + extension), std::ios_base::binary | std::ios_base::ate);
    file_size = (type == "regular" ? (int)source_file.tellg() : 0);
    source_file.seekg(0);
    req_stream << mode << '\n' << type << '\n' << (add_way.empty() ? "`" : add_way) << '\n' << name << '\n' << (extension.empty() ? "`" : extension) << '\n' << file_size << "\n\n";

    write_info("Request size: " + std::to_string(request.size()));

    socket.async_connect(endpoint, boost::bind(&sender_session::handle_on_write, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t n> void transfer::sender_session<n>::handle_on_write(boost::system::error_code error) {

    void (sender_session::*func)(boost::system::error_code);
    func = type == "regular" ? &sender_session::handle_write_data : &sender_session::write_folder;

    if (error) {
        write_error(error.message());
        mode = "start_writing";
        retry_connect(func);
        return;
    }

    async_write(socket, request, boost::bind(func, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t n> void transfer::sender_session<n>::handle_write_data(const boost::system::error_code error) {

    if (error && error.message() != "Connection reset by peer") {
        write_error(error.message());
        void (sender_session::*func)(boost::system::error_code);
        func = &sender_session::handle_write_data;
        mode = "start writing";
        retry_connect(func);
        return;
    }

    write_data(error);
}

template <uint64_t n> void transfer::sender_session<n>::write_data(const boost::system::error_code error) {

    std::this_thread::sleep_for(std::chrono::milliseconds(delay));

    void (sender_session::*func)(boost::system::error_code);
    func = &sender_session::write_data;
    if (error && error.message() != "Connection reset by peer") {
        write_error(error.message());
        func = &sender_session::send_again;
        mode = "finish_writing";
        retry_connect(func);
        return;
    }

    if (now_size >= file_size || !source_file) { go_to_next(); return; }

    source_file.read(buf.c_array(), buf.size());
    if (source_file.gcount() <= 0) {
        write_error("read file error");
        go_to_next();
        return;
    }

    now_size += (int)source_file.gcount();
    write_info("Send " + std::to_string(source_file.gcount()) + " bytes, total " + std::to_string(now_size) + " bytes");

    async_write(socket, buffer(buf.c_array(), source_file.gcount()), boost::bind(func, this->shared_from_this(), boost::asio::placeholders::error));
}

template <uint64_t n> void transfer::sender_session<n>::write_folder(const boost::system::error_code error) {

    if (error && error.message() != "Connection reset by peer") {
        write_error(error.message());
        void (sender_session::*func)(boost::system::error_code);
        func = &sender_session::write_folder;
        mode = "start_writing";
        retry_connect(func);
        return;
    }

    go_to_next();
    return;
}

template <uint64_t n> void transfer::sender_session<n>::init(const std::string& address, const std::string& file) {

    auto parts = get_file(file);
    way = parts["way"];
    name = parts["name"];
    extension = parts["extension"];

    ip = get_ip(address);
    port = std::stoi(get_port(address));

    check(ip, port);
    set_milliseconds();
}

template <uint64_t n> void transfer::sender_session<n>::check(const std::string& address_, const int& port_) {

    std::string error = "Invalid address was received";

    try {
        ip::tcp::endpoint check(ip::address::from_string(address_), port_);
    }
    catch (std::exception e) {
        write_error(error);
        throw std::invalid_argument(error);
    }
}

template <uint64_t n> std::map<std::string, std::string> transfer::sender_session<n>::get_file(const std::string& file) {

    std::map<std::string, std::string> parts;
    int dt = file.rfind('.');
    int sl = file.rfind(slash);

    parts["extension"] = dt != file.npos ? file.substr(dt + 1) : "";
    parts["name"] = sl != file.npos ? file.substr(sl + 1, (dt == file.npos ? file.size() : dt) - sl - 1) : "";
    parts["way"] = sl != file.npos ? file.substr(0, sl) : "";
    if (name == "" && extension != "") {
        swap(parts["name"], parts["extension"]);
        parts["name"] = '.' + parts["name"];
    }

    return parts;
}

template <uint64_t n> std::string transfer::sender_session<n>::get_ip(const std::string& address) {

    std::string ip;
    int sep = address.find(':');
    ip = address.substr(0, sep);
    return ip;
}

template <uint64_t n> std::string transfer::sender_session<n>::get_port(const std::string& address) {

    std::string port;
    int sep = address.rfind(':');
    port = address.substr(sep + 1);
    return port;
}

template <uint64_t n> void transfer::sender_session<n>::set_milliseconds() {

    int file_count = check_way();
    if (delay != 0) delay += file_count / 7;
}

template <uint64_t n> int transfer::sender_session<n>::check_way() {

    std::string error = "Invalid way was received";
    int files = 1;

    try {
        directory_iterator next(way), d_end;
        recursive_directory_iterator object(way);
        for (; object != end && object->path().string() != way + slash + name + (extension.empty() ? "" : '.' + extension); object++)
            if (next->path().string() == object->path().string()) next++;

        if (object == end) {
            write_error(error);
            throw std::invalid_argument(error);
        }
        next++;
        std::string next_way;
        if (next != d_end) next_way = next->path().string();
        for (; object != end && object->path().string() != next_way; object++) if (is_regular_file(*object)) files++;
    }
    catch (std::exception e) {
        write_error(error);
        throw std::invalid_argument(error);
    }
    return files;
}

#endif

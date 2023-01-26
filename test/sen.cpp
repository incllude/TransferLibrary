#include <vector>
#include <chrono>
#include <iostream>
#include <stdint.h>
#include <filesystem>
#include <transfer/run.h>
#include <transfer/sender.h>

using namespace std;
using namespace transfer;

int main() {

#ifdef __linux__
    string add_path = "/files/";
#elif defined(_WIN32) || defined(WIN32)
    string add_path = "\\files\\";
#endif

    bool is_safe = false;
    uint64_t size_of_pack, file_name, safe, port = 4000;
    string ip_address, cur_way = filesystem::current_path().string();
    cur_way = cur_way.substr(0, cur_way.rfind("test") + 4);
    cout << cur_way << '\n';

    vector<uint64_t> available_sizes = {1024, 1048576, 268435456};
    vector<string> available_files = {"archive.zip", "constitution.pdf", "orwell_1984.txt", "kotik.gif"};
    sender<1024> s_1kb;
    sender<1048576> s_1mb;
    sender<268435456> s_256mb;


    cout << "Select the size of the packet to be sent from ";
    for (int i = 0 ; i < available_sizes.size(); i++) cout << available_sizes[i] << "(" << i + 1 << ")" << " ";
    cout << "- "; cin >> size_of_pack;
    while (size_of_pack < 1 || size_of_pack > available_sizes.size()) {cout << "Choose the right size - "; cin >> size_of_pack; }

    cout << "Select ip address (127.0.0.1(0)) - "; cin >> ip_address;
    if (ip_address == "0") ip_address = "127.0.0.1";

    cout << "Select file from ";
    for (int i = 0 ; i < available_files.size(); i++) cout << available_files[i] << "(" << i + 1 << ")" << " ";
    cout << "- "; cin >> file_name;
    while (file_name < 1 || file_name > available_files.size()) { cout << "Choose the right file - "; cin >> file_name; }

    cout << "Select send from safe(1) or unsafe(2) - "; cin >> safe;
    while(safe != 1 && safe != 2) { cout << "Choose the right send - "; cin >> safe; }
    if (safe == 1) is_safe = true;

    string file = cur_way + add_path + available_files[file_name - 1];
    auto start = std::chrono::high_resolution_clock::now();
    switch (available_sizes[size_of_pack - 1]) {
        case 1024:
            if (is_safe) s_1kb.send_safe(ip_address + ':' + to_string(port), file);
            else s_1kb.send(ip_address + ':' + to_string(port), file);
            break;
        case 1048576:
            if (is_safe) s_1mb.send_safe(ip_address + ':' + to_string(port), file);
            else s_1mb.send(ip_address + ':' + to_string(port), file);
            break;
        case 268435456:
            if (is_safe) s_256mb.send_safe(ip_address + ':' + to_string(port), file);
            else s_256mb.send(ip_address + ':' + to_string(port), file);
            break;
    }
    run();
    auto end = std::chrono::high_resolution_clock::now();

    double duration = std::chrono::duration<double>(end - start).count();
    cout << "It took " << duration << " seconds" << '\n';

    return 0;
}


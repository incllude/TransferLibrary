#include <ctime>
#include <vector>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <transfer/run.h>
#include <transfer/sender.h>
#include <transfer/receiver.h>

using namespace std;
using namespace transfer;

#ifdef __linux__
    string add_path = "/files/";
#elif defined(_WIN32) || defined(WIN32)
    string add_path = "\\files\\";
#endif

int main() {

    srand(time(nullptr));
    bool is_safe = false;
    uint64_t type_of_work, size_of_pack, file_name, safe, port = 4000;
    string ip_address, cur_way = filesystem::current_path().string();
    cur_way = cur_way.substr(0, cur_way.rfind("test") + 4);

    vector<uint64_t> available_sizes = {1024, 1048576, 268435456};
    vector<string> available_files = {"archive.zip", "constitution.pdf", "orwell_1984.txt", "kotik.gif"};
    sender<1024> s_1kb;
    sender<1048576> s_1mb;
    sender<268435456> s_256mb;
    receiver<1048576> r_1mb(4000, cur_way, "_rec");

    cout << "Select the type of work send(1), receive(2) - ";
    cin >> type_of_work;
    while (type_of_work != 1 && type_of_work != 2) { cout << "Choose the right type - "; cin >> type_of_work; }

    auto start = std::chrono::high_resolution_clock::now();
    
    if (type_of_work == 1) {

        cout << "Select ip address (write 0 to send on this computer) - "; cin >> ip_address;
        if (ip_address == "0") ip_address = "127.0.0.1";

        size_of_pack = rand() % 3;
        file_name = rand() % 4;
        if (rand() % 2 == 1) is_safe = true;

        string file = cur_way + add_path + available_files[file_name];
        cout << "Sending " << available_files[file_name] << " with " << available_sizes[size_of_pack] << " package size and " << (is_safe ? "unsafe" : "safe") << " mode" << '\n';
        start = std::chrono::high_resolution_clock::now();
        switch (available_sizes[size_of_pack]) {
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
        if (ip_address == "127.0.0.1") {
            r_1mb.receive();
            type_of_work = 2;
        }    
    } else {
        
        start = std::chrono::high_resolution_clock::now();
        r_1mb.receive();
    }

    run();
    auto end = std::chrono::high_resolution_clock::now();
    double duration = std::chrono::duration<double>(end - start).count();
    cout << "It took " << duration << " seconds" << '\n';

    if (type_of_work == 2) {
        
        string file;
        boost::filesystem::directory_iterator start(cur_way), end;
        for (; start->path().string().find("_rec") == start->path().string().npos; start++) {}
        file = start->path().string();

        bool equals = true;
        std::ifstream copy(file, ios_base::in | ios_base::binary);
        file = file.substr(file.rfind('/') == file.npos ? file.rfind('\\') : file.rfind('/'));
        file.erase(file.rfind('_'), 4);
        file = cur_way + add_path + file;
        std::ifstream original(file, ios_base::in | ios_base::binary);

        if (!copy.good() || !original.good()) equals = false;
        char buf1[1024], buf2[1024];
        while (equals && (copy.good() || original.good())) {
            copy.read(buf1, 1024);
            original.read(buf2, 1024);

            if (copy.gcount() != original.gcount() || memcmp(buf1, buf2, copy.gcount()) != 0) equals = false;
        }

        if (equals) cout << "Received file equals to original file" << '\n';
        else cout << "Received file does NOT equals to original file" << '\n';

    }

    return 0;
}

#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <transfer/run.h>
#include <transfer/receiver.h>
#include <boost/filesystem.hpp>

using namespace std;
using namespace transfer;

int main() {

#ifdef __linux__
    string add_path = "/files";
#elif defined(_WIN32) || defined(WIN32)
    string add_path = "\\files";
#endif

    string file;
    string cur_way = filesystem::current_path().string();
    cur_way = cur_way.substr(0, cur_way.rfind("test") + 4);
    receiver<1048576> r_1mb(4000, cur_way, "_rec");

    auto time_start = std::chrono::high_resolution_clock::now();
    r_1mb.receive();
    run();
    auto time_end = std::chrono::high_resolution_clock::now();

    double duration = std::chrono::duration<double>(time_end - time_start).count();
    cout << "It took " << duration << " seconds" << '\n';

    boost::filesystem::directory_iterator start(cur_way), end;
    for (; start != end; start++)
        if (start->path().string().find("_rec") != start->path().string().npos) {
            file = start->path().string();
            break;
        }

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
    return 0;
}

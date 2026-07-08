#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <packr/fs_node.hpp>
#include <fcntl.h>
#include <filesystem>
#include <cstring>
#include <getopt.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <string>
#include <print>

namespace fs = std::filesystem;

static void print_help();

int main(int argc, char** argv) {
    int cur_opt;
    std::string src_path{};
    std::string named_as{};
    bool op_provided{false}; // Whether a -p or -u option was provided
    // bool no_metadata{false};
    bool path_provided{false};
    // bool path_absolute{false};
    packr::OP_TYPE operation;

    while((cur_opt = getopt(argc, argv, "pushl:a:")) != -1) {
        switch(static_cast<char>(cur_opt)) {
        case 'p':
            op_provided = true;
            operation = packr::OP_TYPE::PACK;
            break;

        case 'u':
            if(op_provided) {
                std::println(stderr, "Can't pack and unpack at the same time!\n");
                return 1;
            }
            op_provided = true;
            operation = packr::OP_TYPE::UNPACK;
            break;

        case 's':
            // no_metadata = true;
            break;

        case 'l':
            path_provided = true;
            src_path = std::string{optarg};
            break;

        case 'h':
            print_help();
            return 0;
            break;

        case 'a':
            named_as = optarg;
            break;

        case '?':
            print_help();
            return 1;

        default:
            return 1;
        }
    }

    if(!op_provided || !path_provided) {
        print_help();
        return 1;
    }

    if(*(src_path.data()) == '/') {
        // path_absolute = true;
    } else {
        char* cwd = getcwd(nullptr, 0);
        std::optional<std::string> src_path_temp{packr::join_to_path(src_path, cwd)};
        if(!src_path_temp) {
            return 1;
        }

        src_path = src_path_temp.value();
        free(cwd);
        std::println("target: {}", src_path);
    }

    if(src_path.back() == '/') {
        src_path.pop_back(); // To so that functions such as add_dirname..etc don't mess up(as they need the index of the last /)
    }

    if(operation == packr::OP_TYPE::PACK) {
        // packr::pack_header dir_data{dir, src_path, DEFAULT_ROOT_DIR};
        fs::directory_entry dir_ent{src_path};
        if(!fs::is_directory(dir_ent)) {
            std::println("Target is NOT a directory!");
            return 1;
        }

        packr::pack_header dir_data{dir_ent, DEFAULT_ROOT_DIR};
        if(!dir_data.success) {
            std::println("pack_header constructor: {}", std::strerror(errno));
            return 1;
        }
        if(!named_as.empty()) {
            packr::add_dirname(&dir_data, named_as, src_path);
        }

        std::println("dir name: {}", static_cast<char*>(dir_data.dirname));
        std::println("dir name length: {}", static_cast<packr::u16>(dir_data.dirname_length));
        std::println("dir size is: {}", static_cast<packr::u64>(dir_data.size));

        std::println("total_dir_count: {}", static_cast<packr::u64>(dir_data.total_dir_count));
        std::println("total_file_count: {}", static_cast<packr::u64>(dir_data.total_file_count));
        std::println("total_entry_count: {}", static_cast<packr::u64>(dir_data.total_entry_count));

        std::println("child_dir_count: {}", static_cast<packr::u64>(dir_data.child_dir_count));
        std::println("child_file_count: {}", static_cast<packr::u64>(dir_data.child_file_count));
        std::println("child_entry_count: {}", static_cast<packr::u64>(dir_data.child_entry_count));

        std::println("last access time: {}", static_cast<packr::u64>(dir_data.acc_time));
        std::println("last modification time: {}", static_cast<packr::u64>(dir_data.mod_time));
        std::println("last status change time: {}", static_cast<packr::u64>(dir_data.sc_time));
        std::println("mode: {}", static_cast<packr::u64>(dir_data.mode));

        const std::string extension{".packr"};
        std::string pack_file_str{};
        pack_file_str.reserve(extension.length() + strlen(dir_data.dirname));

        pack_file_str += dir_data.dirname;
        pack_file_str += extension;

        std::println("pack file str: {}", pack_file_str);

        packr::File_W pack_file_stream{fs::directory_entry(pack_file_str)};

        std::println("EXISTS: {}", pack_file_stream.entry_obj().exists());
        if(!pack_file_stream.setup_stream(packr::open_type::fresh)) {
            std::println("FAILED TO SETUP STREAM");
            return 1;
        }

        if(!dir_data.pack(dir_ent, pack_file_stream, DEFAULT_ROOT_DIR)) {
            perror("pack()");
            std::println(stderr, "pack(): {}", strerror(errno));
            return 1;
        }

        // Cleanup
    } else {
        packr::File_R pack_file{fs::directory_entry(src_path)};
        if(!pack_file.setup_stream(packr::open_type::exists)) {
            std::println(stderr, "FAILED TO SETUP STREAM");
            return 1;
        }

        if(!packr::dir_entry::unpack(pack_file, 0)) {
            std::println(stderr, "unpack(): {}", strerror(errno));
            return 1;
        }

        std::println("unpack sucess");
    }

    return 0;
}

static void print_help() {
    std::println("Usage:\n -p: pack a directory\n -u: unpack a .packr file\n -s: neglect metadata\n -l: path to directory\n "
                 "-h: help");
}

#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/entry.hpp>
#include <packr/fs_node.hpp>
#include <filesystem>
#include <cstring>
#include <getopt.h>
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

    if(!packr::curate_src_path(src_path)) {
        return 1;
    }
    std::println("target: {}", src_path);

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

        print_dir_data(dir_data);

        std::string pack_filename{packr::create_pack_filename(dir_data)};
        std::println("pack filename: {}", pack_filename);

        packr::File_W pack_file_stream{fs::directory_entry(pack_filename)};

        if(!pack_file_stream.setup_stream(packr::open_type::fresh)) {
            std::println("FAILED TO SETUP STREAM");
            return 1;
        }

        if(!dir_data.pack(dir_ent, pack_file_stream, DEFAULT_ROOT_DIR)) {
            perror("pack()");
            std::println(stderr, "pack(): {}", strerror(errno));
            return 1;
        }

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

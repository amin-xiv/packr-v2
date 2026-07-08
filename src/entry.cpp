#include <ios>
#include <packr/entry.hpp>
#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/fs_node.hpp>
#include <filesystem>
#include <string>
#include <sys/stat.h>
#include <cstring>
#include <print>
#include <dirent.h>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include <system_error>
#include <utility>

namespace fs = std::filesystem;

namespace packr {

dir_entry::dir_entry(const std::filesystem::directory_entry& dir, u32 nest_count) {
    if(!fs::exists(dir.symlink_status())) {
        this->success = false;
        return;
    }
    // TODO: Add support for a symbolic dir_entry

    // Other members are default intialized during construction

    this->entry_class = (nest_count - 1) == 0 ? entry_class_t::CHILD_ENT : entry_class_t::NESTED_ENT;
    add_dirname(this, "", std::string{dir.path().string().data(), dir.path().string().size()});
    this->mode = std::to_underlying(dir.symlink_status().permissions());

    struct stat ent_stat;

    for(const fs::directory_entry& entry : fs::directory_iterator(dir)) {
        std::string full_path{entry.path().string()};

        std::println("current entry: {}", full_path);

        if(lstat(full_path.data(), &ent_stat) == -1) {
            this->success = false;
            return;
        }

        if(fs::is_directory(entry.symlink_status())) {
            dir_entry data_inner{entry, nest_count + 2};

            if(!data_inner.success) {
                this->success = false;
                return;
            }

            this->size += data_inner.size;
            this->total_entry_count++;
            this->total_dir_count++;

            // Add data_inner's total counts just in case there was nested directories
            this->total_dir_count += data_inner.total_dir_count;
            this->total_entry_count += data_inner.total_entry_count;
            this->total_file_count += data_inner.total_file_count;

            // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
            // we can increment child counts
            if(nest_count == 0) {
                this->child_entry_count++;
                this->child_dir_count++;
            }

            std::println("added_dirname: {}", this->dirname);
        } else {
            if(fs::is_regular_file(entry.symlink_status())) {
                this->size += entry.file_size();
                this->total_entry_count++;
                this->total_file_count++;

                // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
                // we can increment child counts
                if(nest_count == 0) {
                    this->child_entry_count++;
                    this->child_file_count++;
                }

                //  TODO: Add support for symlinks also here

            } else {
            }
        }
    }

    if(nest_count == 0) {
        struct stat root_stat;
        if(lstat(dir.path().string().data(), &root_stat) == -1) {
            return;
        }

        this->acc_time = root_stat.st_atim.tv_sec + NSEC_TO_SEC(root_stat.st_atim.tv_nsec);
        this->mod_time = root_stat.st_mtim.tv_sec + NSEC_TO_SEC(root_stat.st_mtim.tv_nsec);
        this->sc_time = root_stat.st_ctim.tv_sec + NSEC_TO_SEC(root_stat.st_ctim.tv_nsec);
    }

    this->success = true;
}

file_entry::file_entry(const std::filesystem::path& file_path) {
    // Dummy error code object to avoid exceptions
    std::error_code err;

    struct stat file_stat;
    if(lstat(file_path.c_str(), &file_stat) == -1) {
        this->success = false;
        return;
    }

    File file{file_path};
    if(!file) {
        this->success = false;
        return;
    }

    if(!fs::exists(file.entry_obj().symlink_status())) {
        this->success = false;
        return;
    }

    const std::string& actual_filename{file_path.filename().string()};
    std::strcpy(this->filename, actual_filename.data());

    // TODO: SYMLINKSS!!
    this->size = fs::file_size(file_path, err);
    if(err) { // i.e. if err.value() > 0
        this->success = false;
        return;
    }

    this->filename_length = actual_filename.length(); // +1 to count the \0

    // TODO: DOUBLES??
    this->acc_time = file_stat.st_atim.tv_sec + NSEC_TO_SEC(file_stat.st_atim.tv_nsec);
    this->mod_time = file_stat.st_mtim.tv_sec + NSEC_TO_SEC(file_stat.st_mtim.tv_nsec);
    this->sc_time = file_stat.st_ctim.tv_sec + NSEC_TO_SEC(file_stat.st_ctim.tv_nsec);

    if(fs::is_regular_file(file.entry_obj().symlink_status())) {
        this->type = file_type::regular;
    }

    this->mode = std::to_underlying((file.entry_obj().symlink_status().permissions()));
    this->success = true;
}
bool dir_entry::pack_dir(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts, const u32 nest_count) {
    //(for future use) bool no_metadata{(opts & P_NOMETADATA) != 0};
    dir_entry dir_header_copy{*this};

    // write the dir header upfront only if it's the intial pack header(nest_count
    // = 0)
    if(nest_count == 0) {
        if(!pack_file.write(reinterpret_cast<char*>(this), sizeof(dir_entry))) {
            return false;
        }
    }

    special_marker dir_marker_start = {.type = ENT_DIR_START};
    if(!pack_file.write(reinterpret_cast<char*>(&dir_marker_start), sizeof(special_marker))) {
        return false;
    }

    // i.e. if nest_count > 0
    if(nest_count > 0) {
        if(!pack_file.write(reinterpret_cast<char*>(this), sizeof(dir_entry))) {
            return false;
        }
    }

    for(const fs::directory_entry& curr_ent : fs::directory_iterator(dir)) {
        const std::string full_path{curr_ent.path().string()};

        std::println("current entry to pack: {}", full_path);

        if(fs::is_directory(curr_ent.symlink_status())) {
            dir_entry dir_data_inner{curr_ent, DEFAULT_ROOT_DIR};
            if(!dir_data_inner.success) {
                return false;
            }

            if(!dir_data_inner.pack_dir(curr_ent, pack_file, opts, nest_count + 1)) {
                return false;
            }

            dir_header_copy.total_dir_count--;
            dir_header_copy.total_entry_count--;

            if(nest_count == 0) {
                dir_header_copy.child_entry_count--;
                dir_header_copy.child_dir_count--;
            }

            // TODO: fix this

        } /*else if(S_ISLNK(ent_stat.st_mode)) {
            // i don't wanna handle any symlinks rn
            continue;
        }*/
        else {
            file_entry file_data{full_path};
            if(!file_data.success) {
                return false;
            }

            special_marker file_marker = {.type = ENT_FILE};
            if(!pack_file.write(reinterpret_cast<char*>(&file_marker), sizeof(special_marker))) {
                return false;
            }

            if(!pack_file.write(reinterpret_cast<char*>(&file_data), sizeof(file_entry))) {
                return false;
            }

            // check if file has actually some data and size != 0 before writing file
            // contents
            if(file_data.size > 0) {
                File_R file_stream{full_path};
                if(!file_stream.setup_stream(open_type::exists)) {
                    return false;
                }

                // if the file has actual contents and not empty
                std::string read_buff{};
                read_buff.reserve(file_data.size);
                memset(read_buff.data(), '\0', file_data.size);
                if(!file_stream.read(read_buff.data(), static_cast<std::streamsize>(file_data.size))) {
                    return false;
                }

                // TODO: file_data.size() must not be greater than std::streamsize
                if(!pack_file.write(read_buff.data(), static_cast<std::streamsize>(file_data.size))) {
                    return false;
                }

                dir_header_copy.total_file_count--;
                dir_header_copy.total_entry_count--;

                if(nest_count == 0) {
                    dir_header_copy.child_entry_count--;
                    dir_header_copy.child_file_count--;
                }
            }
        }
    }

    special_marker dir_marker_end{.type = ENT_DIR_END};
    return pack_file.write(reinterpret_cast<char*>(&dir_marker_end), sizeof(special_marker)); // bool
}

bool pack_header::pack(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts) {
    special_marker pack_start_marker{.type = PACK_START};
    if(!pack_file.write(reinterpret_cast<char*>(&pack_start_marker), sizeof(special_marker))) {
        return false;
    }

    if(!pack_dir(dir, pack_file, opts, DEFAULT_ROOT_DIR)) {
        return false;
    }

    special_marker pacK_end_marker{.type = PACK_END};
    return !(!pack_file.write(reinterpret_cast<char*>(&pacK_end_marker), sizeof(special_marker)));
}

bool dir_entry::unpack_dir(File_R& pack_file, const u8 opts, const u32 nest_count) {
    // 'opts' flag is for future use(maybe lol)

    // Flag to keep looping
    bool read_pack_file{true};

    while(read_pack_file) {
        special_marker curr_marker;
        if(!pack_file.read(reinterpret_cast<char*>(&curr_marker), sizeof(special_marker))) {
            return false;
        }
        switch(curr_marker.type) {
        case PACK_START:
            // this shouldn't be here, it should be already read before this function is invoked
            return false;
            break;

        case PACK_END:
            read_pack_file = false; // Terminate the while-loop
            break;

        case ENT_FILE:
            {
                /* On its own block to prevent pollution the case() namespace */
                file_entry curr_file_data;
                if(!pack_file.read(reinterpret_cast<char*>(&curr_file_data), sizeof(file_entry))) {
                    return false;
                }

                const char* unnamed_filename = "unamed-file"; // Just in case the file had no name for some reason
                if(curr_file_data.filename_length < 1) {
                    // Copying it into curr_file_data.filename so a flag isn't needed
                    memcpy(curr_file_data.filename, unnamed_filename, strlen(unnamed_filename));
                }

                File_W target_file{curr_file_data.filename};
                if(!target_file.setup_stream(open_type::fresh)) {
                    return false;
                }

                if(curr_file_data.size > 0) {
                    std::string file_data_buff{};
                    file_data_buff.reserve(curr_file_data.size);

                    if(!pack_file.read(file_data_buff.data(), static_cast<std::streamsize>(curr_file_data.size))) {
                        return false;
                    }

                    if(!target_file.write(file_data_buff.data(), static_cast<std::streamsize>(curr_file_data.size))) {
                        return false;
                    }
                }
            }
            break;

        case ENT_DIR_START:
            {
                /* On its own block to prevent pollution the case() namespace */
                dir_entry curr_dir_data;
                if(!pack_file.read(reinterpret_cast<char*>(&curr_dir_data), sizeof(dir_entry))) {
                    return false;
                }

                const char* unnamed_dirname = "unamed-directory"; // Just in case the dir had no name for some reason
                if(curr_dir_data.dirname_length < 1) {
                    // Copying it into curr_dir_data.dirname so a flag isn't needed
                    memcpy(curr_dir_data.dirname, unnamed_dirname, strlen(unnamed_dirname));
                }

                if(mkdir(curr_dir_data.dirname, curr_dir_data.mode) == -1) {
                    return false;
                }
                char* cwd{getcwd(nullptr, 0)};
                if(cwd == nullptr) {
                    return false;
                }
                // The path of the newely created directory
                std::optional<std::string> target_dir_path{join_to_path(curr_dir_data.dirname, cwd)};
                if(!target_dir_path) {
                    free(cwd);
                    return false;
                }

                std::string& target_dir_path_str{target_dir_path.value()};
                if(chdir(target_dir_path_str.data()) == -1) {
                    free(cwd);
                    return false;
                }

                if(!unpack_dir(pack_file, opts, nest_count + 1)) {
                    free(cwd);
                    return false;
                }

                // Return to curr after unpacking the sub dir
                if(chdir(cwd) == -1) {
                    free(cwd);
                    return false;
                }

                free(cwd);
            }
            break;

        case ENT_DIR_END:
            read_pack_file = false;
            break;
        default:
            return false;
            break;
        }
    }

    return true;
}

bool dir_entry::unpack(File_R& pack_file, const u8 opts) {
    // Reading PACK_START
    special_marker pack_start_marker;
    if(!pack_file.read(reinterpret_cast<char*>(&pack_start_marker), sizeof(special_marker))) {
        return false;
    }
    if(pack_start_marker.type != PACK_START) {
        return false;
    }

    // Reading pack_header
    dir_entry pack_header;
    if(!pack_file.read(reinterpret_cast<char*>(&pack_header), sizeof(dir_entry))) {
        return false;
    }

    // This marks the start of the target root directory
    special_marker initial_dir_start_marker;
    if(!pack_file.read(reinterpret_cast<char*>(&initial_dir_start_marker),
                       static_cast<std::streamsize>(sizeof(special_marker)))) {
        return false;
    }
    if(initial_dir_start_marker.type != ENT_DIR_START) {
        return false;
    }

    // Making the root directory and changing into it
    if(mkdir(pack_header.dirname, pack_header.mode) == -1) {
        if(errno != EEXIST) {
            return false;
        }
    }
    char* cwd = getcwd(nullptr, 0);
    if(cwd == nullptr) {
        return false;
    }
    std::optional<std::string> root_dir_path{join_to_path(pack_header.dirname, cwd)};
    if(!root_dir_path) {
        return false;
    }

    std::string& root_dir_path_str{root_dir_path.value()};
    if(chdir(root_dir_path_str.data()) == -1) {
        free(cwd);
        return false;
    }

    if(!unpack_dir(pack_file, 0, DEFAULT_ROOT_DIR)) {
        free(cwd);
        return false;
    }

    free(cwd);
    return true;
}

} // namespace packr

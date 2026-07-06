#include <packr/entry.hpp>
#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/fs_node.hpp>
#include <filesystem>
// #include <stdexcept>
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

    //    struct dirent* entry;
    //   struct stat ent_stat;

    for(const fs::directory_entry& entry : fs::directory_iterator(dir)) {
        std::string full_path{entry.path().string()};

        std::println("current entry: {}", full_path);

        // if(lstat(full_path_str.data(), &ent_stat) == -1) {
        //  this->success = false;
        // return;
        //}

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

    // get timestamps and mode
    // if(nest_count == 0) {
    //        struct stat root_stat;
    //       const char* dir_str_ptr{dir_str.data()};
    //      if(lstat(dir_str_ptr, &root_stat) == -1) {
    //         return;
    //   }

    /*
    this->acc_time = root_stat.st_atim.tv_sec + NSEC_TO_SEC(root_stat.st_atim.tv_nsec);
    this->mod_time = root_stat.st_mtim.tv_sec + NSEC_TO_SEC(root_stat.st_mtim.tv_nsec);
    this->sc_time = root_stat.st_ctim.tv_sec + NSEC_TO_SEC(root_stat.st_ctim.tv_nsec);
    */
    //}

    this->success = true;
}

file_entry::file_entry(const std::filesystem::path& file_path, const u32 nest_count) {
    // Dummy error code object to avoid exceptions
    std::error_code err;

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

    this->size = fs::file_size(file_path, err);
    if(err) { // i.e. if err.value() > 0
        this->success = false;
        return;
    }

    this->filename_length = actual_filename.length(); // +1 to count the \0

    // DOUBLES??
    /*
    this->acc_time = file_stat.st_atim.tv_sec + NSEC_TO_SEC(file_stat.st_atim.tv_nsec);
    this->mod_time = file_stat.st_mtim.tv_sec + NSEC_TO_SEC(file_stat.st_mtim.tv_nsec);
    this->sc_time = file_stat.st_ctim.tv_sec + NSEC_TO_SEC(file_stat.st_ctim.tv_nsec);
    */

    this->mode = std::to_underlying((file.entry_obj().symlink_status().permissions()));
    this->success = true;
}
bool dir_entry::pack_dir(const std::filesystem::directory_entry& dir, FILE* pack_file, const u8 opts, const u32 nest_count) {
    //(for future use) bool no_metadata{(opts & P_NOMETADATA) != 0};
    dir_entry dir_header_copy{*this};

    // write the dir header upfront only if it's the intial pack header(nest_count
    // = 0)
    if(nest_count == 0) {
        if(fwrite(this, sizeof(dir_entry), 1, pack_file) < 1) {
            return false;
        }
    }

    special_marker dir_marker_start = {.type = ENT_DIR_START};
    if(fwrite(&dir_marker_start, sizeof(special_marker), 1, pack_file) < 1) {
        return false;
    }

    // i.e. if nest_count > 0
    if(nest_count > 0) {
        if(fwrite(this, sizeof(dir_entry), 1, pack_file) < 1) {
            return false;
        }

        std::println("(to write)dir_data_inner->dirname: {}", this->dirname);
    }

    struct stat ent_stat;
    struct dirent* curr_ent;
    for(const fs::directory_entry& curr_ent : fs::directory_iterator(dir)) {
        const std::string full_path{curr_ent.path().string()};

        std::println("current entry to pack: {}", full_path);

        // if(lstat(full_path_str.data(), &ent_stat) == -1) {
        //    closedir(dir);
        //   return false;
        //}

        if(fs::is_directory(curr_ent.symlink_status())) {
            // dir_entry type because pack_header is just a typedef of 'struct
            //
            dir_entry dir_data_inner{curr_ent, DEFAULT_ROOT_DIR};
            std::println("(fresh)dir_data_inner->dirname: {}", dir_data_inner.dirname);
            std::println("full_path_str: {}", full_path);
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
            file_entry file_data{full_path, nest_count + 1};
            if(!file_data.success) {
                return false;
            }

            special_marker file_marker = {.type = ENT_FILE};
            if(fwrite(&file_marker, sizeof(special_marker), 1, pack_file) < 1) {
                return false;
            }

            if(fwrite(&file_data, sizeof(file_entry), 1, pack_file) < 1) {
                return false;
            }

            // check if file has actually some data and size != 0 before writing file
            // contents
            if(file_data.size > 0) {
                FILE* file_stream{fopen(full_path.data(), "r")};
                if(file_stream == nullptr) {
                    return false;
                }

                // if the file has actual contents and not empty
                std::string read_buff{};
                read_buff.reserve(file_data.size);
                memset(read_buff.data(), '\0', file_data.size);
                if(fread(read_buff.data(), file_data.size, 1, file_stream) < 1) {
                    fclose(file_stream);
                    return false;
                }

                if(fwrite(read_buff.data(), file_data.size, 1, pack_file) < 1) {
                    fclose(file_stream);
                    return false;
                }

                fclose(file_stream);
                sync(); // to ensure data is actually residing on the file before next
                        // iteration

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
    if(fwrite(&dir_marker_end, sizeof(special_marker), 1, pack_file) < 1) {
        return false;
    }

    sync(); // to ensure data is actually residing on the file
    return true;
}

bool pack_header::pack(const std::filesystem::directory_entry& dir, FILE* pack_file, const u8 opts) {
    special_marker pack_start_marker{.type = PACK_START};
    if(fwrite(&pack_start_marker, sizeof(special_marker), 1, pack_file) < 1) {
        return false;
    }

    if(!pack_dir(dir, pack_file, opts, DEFAULT_ROOT_DIR)) {
        return false;
    }

    special_marker pacK_end_marker{.type = PACK_END};
    return !(fwrite(&pacK_end_marker, sizeof(special_marker), 1, pack_file) < 1);
}

bool dir_entry::unpack_dir(FILE* pack_file, u8 opts, u32 nest_count) {
    // 'opts' flag is for future use(maybe lol)

    // Flag to keep looping
    bool read_pack_file{true};

    while(read_pack_file) {
        special_marker curr_marker;
        if(fread(&curr_marker, sizeof(special_marker), 1, pack_file) < 1) {
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
                if(fread(&curr_file_data, sizeof(file_entry), 1, pack_file) < 1) {
                    return false;
                }

                const char* unnamed_filename = "unamed-file"; // Just in case the file had no name for some reason
                if(curr_file_data.filename_length < 1) {
                    // Copying it into curr_file_data.filename so a flag isn't needed
                    memcpy(curr_file_data.filename, unnamed_filename, strlen(unnamed_filename));
                }

                FILE* target_file{fopen(curr_file_data.filename, "w")};
                if(target_file == nullptr) {
                    return false;
                }

                if(curr_file_data.size > 0) {
                    std::string file_data_buff{};
                    file_data_buff.reserve(curr_file_data.size);

                    if(fread(file_data_buff.data(), curr_file_data.size, 1, pack_file) < 1) {
                        return false;
                    }

                    if(fwrite(file_data_buff.data(), curr_file_data.size, 1, target_file) < 1) {
                        return false;
                    }

                    fclose(target_file);
                }
            }
            break;

        case ENT_DIR_START:
            {
                /* On its own block to prevent pollution the case() namespace */
                dir_entry curr_dir_data;
                if(fread(&curr_dir_data, sizeof(dir_entry), 1, pack_file) < 1) {
                    return false;
                }

                // Verify entry is actually a directory
                if(!S_ISDIR(curr_dir_data.mode)) {
                    return false;
                }

                std::println("unpacked dirname: {}", curr_dir_data.dirname);
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

bool dir_entry::unpack(FILE* pack_file, u8 opts, u32 nest_count) {
    // Reading PACK_START
    special_marker pack_start_marker;
    if(fread(&pack_start_marker, sizeof(special_marker), 1, pack_file) < 1) {
        return false;
    }
    if(pack_start_marker.type != PACK_START) {
        return false;
    }

    // Reading pack_header
    dir_entry pack_header;
    if(fread(&pack_header, sizeof(dir_entry), 1, pack_file) < 1) {
        return false;
    }

    // This marks the start of the target root directory
    special_marker initial_dir_start_marker;
    if(fread(&initial_dir_start_marker, sizeof(special_marker), 1, pack_file) < 1) {
        return false;
    }
    if(initial_dir_start_marker.type != ENT_DIR_START) {
        return false;
    }

    // Verify whether data is corrupted or no
    if(!S_ISDIR(pack_header.mode)) {
        return false;
    };

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

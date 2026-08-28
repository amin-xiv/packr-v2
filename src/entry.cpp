#include <packr/entry.hpp>
#include <packr/types.hpp>
#include <packr/utils.hpp>
#include <packr/fs_node.hpp>
#include <packr/misc_structs.hpp>
#include <filesystem>
#include <ios>
#include <cassert>
#include <string>
#include <sys/stat.h>
#include <print>
#include <cstring>
#include <dirent.h>
#include <unistd.h>
#include <system_error>
#include <utility>
#include <limits>

// TODO: Now we need to pack recursive symlinks as symlinks instead of simply discarding them
// TODO: make sure it works with cycle_test/
// NOTE: Stopped at pack_dir_as_symlink, which should be integrated to packing/unpacking
//
// BUG: symlink dirs aren't re-symlinked to their targets

namespace fs = std::filesystem;

namespace packr {

static bool inc_dir_ent_dir_count(dir_entry& dir, const fs::directory_entry& entry, const u32 nest_count, const u8 opts,
                                  anc_map_t& anc_table) {
    dir_entry data_inner{entry, nest_count, opts, anc_table};

    if(data_inner.m_success == dir_entry_ret_code::fail) {
        std::string err_msg{"in inc_dir_ent_dir_count, dir_entry.m_success returned false for entry.path(): " +
                            entry.path().string()};
        debug_log(err_msg);
        dir.m_success = dir_entry_ret_code::fail;
        return false;
    }

    if(data_inner.m_success == dir_entry_ret_code::recursive) {
        std::string err_msg{"skipped a recursive code path: " + entry.path().string()};
        debug_log(err_msg, log_type::info);
        // not returning to allow increasing counts as this directory will be packed as a symlink
    }

    dir.m_size += data_inner.m_size;
    dir.m_total_entry_count++;
    dir.m_total_dir_count++;

    // Add data_inner's total counts just in case there was nested directories
    dir.m_total_dir_count += data_inner.m_total_dir_count;
    dir.m_total_entry_count += data_inner.m_total_entry_count;
    dir.m_total_file_count += data_inner.m_total_file_count;

    // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
    // we can increment child counts
    if((nest_count - 1) == 0) {
        dir.m_child_entry_count++;
        dir.m_child_dir_count++;
    }

    // just to make sure we don't mess up the arithmetic any point during development
    assert(nest_count != std::numeric_limits<decltype(nest_count)>::max()); // i.e. it's not -1 for some reason

    return true;
}

static void inc_dir_ent_file_count(dir_entry& dir, const fs::directory_entry& entry, const u32 nest_count, bool add_size = true) {
    std::error_code err;

    if(add_size) {
        dir.m_size += static_cast<ssize_t>(entry.file_size(err));
    }
    dir.m_total_entry_count++;
    dir.m_total_file_count++;

    // if nest_count == 0(DEFAULT_ROOT_DIR) then we are at root directory, so
    // we can increment child counts
    if(nest_count == 0) {
        dir.m_child_entry_count++;
        dir.m_child_file_count++;
    }
}

static bool handle_dir_ancestory(const struct stat& stat_obj, anc_map_t& anc_table, bool compare_only = false) {
    // returns true if current dev and ino numbers are included within the ancestors table

    const std::string dev_ino_str{std::to_string(stat_obj.st_dev) + std::to_string(stat_obj.st_ino)};
    const dev_ino_t dev_ino{.dev = stat_obj.st_dev, .ino = stat_obj.st_ino};
    const bool collide{!anc_table.empty() && anc_table.contains(dev_ino_str)};

    if(!collide && !compare_only) {
        [[maybe_unused]] const auto emplace_res{anc_table.try_emplace(dev_ino_str, dev_ino)};
        assert(emplace_res.second && "failed to insert dev_ino into anc_table"); // i.e. insertion has actually taken palce
    }

    return collide;
}

dir_entry::dir_entry(const std::filesystem::directory_entry& dir, u32 nest_count, const u8 opts, anc_map_t& anc_table) {
    // No need to care whether this object is a symlink itself or not since it would be handled externally
    // anc_table includes the device and inode numbers of previous parents(obtained by stat), to detect and avoid
    // endless recursion(that is caused by having a symlink inside a directory pointing to one of its parent directories)

    std::error_code err;

    if(!fs::exists(dir.symlink_status(err)) || !fs::is_directory(dir)) {
        std::string err_msg{"dir_entry constructor, the path " + dir.path().string() + " doesn't exist or is not a directory"};
        debug_log(err_msg);
        m_success = dir_entry_ret_code::fail;
        return;
    }

    // Other members are default intialized during construction
    m_entry_class = (nest_count - 1) == 0 ? entry_class_t::CHILD_ENT : entry_class_t::NESTED_ENT;
    add_dirname(this, "", std::string{dir.path().string().data(), dir.path().string().size()});

    const bool follow_symlinks{(opts & O_SYM) > 0};

    Directory main_dir_obj{dir.path()};

    struct stat main_stat;
    int stat_res{};

    if(!main_dir_obj) {
        std::string err_msg{"failed to intialize a Directory object at dir_entry constructor, with error message: \n" +
                            std::string{main_dir_obj.err()}};
        debug_log(err_msg);
        m_success = dir_entry_ret_code::fail;
        return;
    }

    if(main_dir_obj.type() == dir_type::symlink) {
        if(main_dir_obj.secondary_path().empty()) {
            std::string err_msg{"dir symlink " + dir.path().string() + " doesn't have a target"};
            debug_log(err_msg);
            m_success = dir_entry_ret_code::fail;
            return;
        }
    }

    stat_res = stat(dir.path().c_str(), &main_stat);

    if(stat_res == -1) {
        std::string err_msg{"stat_res returned -1 at dir_entry constructor with dir.path(): " + dir.path().string()};
        debug_log(err_msg);
        m_success = dir_entry_ret_code::fail;
        return;
    }
    assert(stat_res != -1);

    if(handle_dir_ancestory(main_stat, anc_table)) {
        m_success = dir_entry_ret_code::recursive;
        return;
    }

    m_acc_time = {.sec = main_stat.st_atim.tv_sec, .nsec = main_stat.st_atim.tv_nsec};
    m_mod_time = {.sec = main_stat.st_mtim.tv_sec, .nsec = main_stat.st_mtim.tv_nsec};
    m_sc_time = {.sec = main_stat.st_ctim.tv_sec, .nsec = main_stat.st_ctim.tv_nsec};

    m_mode = std::to_underlying(main_dir_obj.entry_obj().status().permissions());

    for(const fs::directory_entry& entry : fs::directory_iterator(dir, err)) {
        std::string full_path{entry.path().string()};

        packr::debug_log("current entry: " + full_path, log_type::info);

        fs::file_status ent_sym_status{entry.symlink_status(err)};

        if(fs::is_directory(ent_sym_status)) {
            struct stat inner_stat{};
            stat_res = 0;
            stat_res = stat(entry.path().c_str(), &inner_stat);

            if(stat_res == -1) {
                std::string err_msg{"stat_res returned -1 with dir.path(): " + dir.path().string()};
                debug_log(err_msg);
                m_success = dir_entry_ret_code::fail;
                return;
            }
            assert(stat_res != -1);

            // compare_only as the table would be assumed unaltered at the nest inc_dir_ent_dir_count call
            if(handle_dir_ancestory(inner_stat, anc_table, true)) {
                continue;
            }

            if(!inc_dir_ent_dir_count(*this, entry, nest_count + 1, opts, anc_table)) {
                packr::debug_log("ERROR COLLECTING DIRECTORY DATA: " + full_path);
            }

        } else if(fs::is_regular_file(ent_sym_status)) {
            inc_dir_ent_file_count(*this, entry, nest_count);

        } else if(fs::is_symlink(ent_sym_status)) {
            fs::path secondary_path{packr::read_symlink(entry.path())};

            fs::directory_entry secondary_entry{secondary_path, err};
            if(secondary_path.empty() || !fs::exists(secondary_entry.symlink_status(err))) {
                std::println(stderr, "WARNING: Couldn't read symlink target path: {}", secondary_path.string());
                std::println(stderr, "    -> symlinked by: {}", full_path);
                inc_dir_ent_file_count(*this, entry, nest_count, false);
                continue;
            }

            if(!follow_symlinks) {
                // Here we do not add 1 to nest_count as this file is in the same current directory that has the same nest_count
                inc_dir_ent_file_count(*this, entry, nest_count, false);

            } else {
                if(fs::is_regular_file(secondary_entry)) {
                    inc_dir_ent_file_count(*this, secondary_entry, nest_count);

                } else if(fs::is_directory(secondary_entry)) {
                    struct stat inner_stat{};
                    stat_res = 0;
                    stat_res = stat(secondary_entry.path().c_str(), &inner_stat);

                    if(stat_res == -1) {
                        std::string err_msg{"stat_res returned -1 with dir.path(): " + dir.path().string()};
                        debug_log(err_msg);
                        m_success = dir_entry_ret_code::fail;
                        return;
                    }
                    assert(stat_res != -1);

                    // compare_only as the table would be assumed unaltered at the nest inc_dir_ent_dir_count call
                    if(handle_dir_ancestory(inner_stat, anc_table, true)) {
                        continue;
                    }

                    if(!inc_dir_ent_dir_count(*this, entry, nest_count + 1, opts, anc_table)) {
                        packr::debug_log("ERROR COLLECTING (symlinked)DIRECTORY DATA: " + secondary_path.string());
                        packr::debug_log("    -> symlinked by: " + full_path, log_type::none);
                    }
                }
            }

        } else {
            std::println(stderr, "Ignoring a special file: ", entry.path().string());
        }
    }

    const std::string dev_ino_str{std::to_string(main_stat.st_dev) + std::to_string(main_stat.st_ino)};
    anc_table.erase(dev_ino_str);
    m_success = dir_entry_ret_code::success;
}

file_entry::file_entry(const std::filesystem::path& file_path, const u8 opts) {
    std::error_code err; // To avoid exceptions

    const bool follow_symlinks{(opts & O_SYM) > 0};
    bool symlink_target_exists{};

    File file_obj{file_path};

    if(!file_obj) {
        std::string err_msg{"failed to intialize a File object at file_entry constructor, with error message: \n" +
                            std::string{file_obj.err()} + " and file_path: " + file_path.string()};
        debug_log(err_msg);
        m_success = false;
        return;
    }

    const fs::path target_parent{file_path.parent_path()};
    fs::path temp_secondary_path{file_obj.secondary_path().is_relative() ? target_parent / file_obj.secondary_path()
                                                                         : file_obj.secondary_path()};

    if(follow_symlinks && file_obj.type() == file_type::symlink && fs::exists(temp_secondary_path, err)) {
        symlink_target_exists = true;
    }

    struct stat file_stat;
    int stat_res{};

    if(follow_symlinks && symlink_target_exists) {
        stat_res = stat(file_path.c_str(), &file_stat);

    } else {
        stat_res = lstat(file_path.c_str(), &file_stat); // lstat works well with regular files and broken symlinks
    }

    assert(stat_res != -1);
    // TEST: case
    if(stat_res == -1) {
        std::string err_msg{"stat_res returned -1 at file_entry constructor with file_path: " + file_path.string()};
        debug_log(err_msg);
        m_success = false;
        return;
    }

    const std::string& actual_filename{file_path.filename().string()};
    std::strcpy(m_filename, actual_filename.data());

    if((follow_symlinks && symlink_target_exists) || file_obj.type() != file_type::symlink) {
        m_size = static_cast<ssize_t>(fs::file_size(file_path, err));
        if(err) { // i.e. if err.value() > 0
            std::string err_msg{"in file_entry constructor with path " + file_path.string() +
                                ", fs::file_size.err() was greater than zero"};
            debug_log(err_msg);
            m_success = false;
            return;
        }
    }

    m_filename_length = actual_filename.length();

    memcpy(m_secondary_path, file_obj.secondary_path().c_str(), file_obj.secondary_path().string().length());
    m_secondary_path_length = file_obj.secondary_path().string().length();

    m_acc_time = {.sec = file_stat.st_atim.tv_sec, .nsec = file_stat.st_atim.tv_nsec};
    m_mod_time = {.sec = file_stat.st_mtim.tv_sec, .nsec = file_stat.st_mtim.tv_nsec};
    m_sc_time = {.sec = file_stat.st_ctim.tv_sec, .nsec = file_stat.st_ctim.tv_nsec};

    m_type = file_obj.type();

    m_mode =
        std::to_underlying((follow_symlinks && symlink_target_exists) ? (file_obj.entry_obj().status().permissions())
                                                                      : (file_obj.entry_obj().symlink_status(err).permissions()));
    m_success = true;
}

dir_sym_entry::dir_sym_entry(const fs::directory_entry& dir) {
    std::error_code err; // To avoid exceptions
    assert(fs::is_symlink(dir, err));

    File_sym file_obj{dir};
    const fs::path& symlink_path{dir.path()}; // the actual target symlink to pack

    if(!file_obj) {
        std::string err_msg{"failed to intialize a File object at dir_sym_entry constructor, with error message: \n" +
                            std::string{file_obj.err()} + " and symlink_path: " + dir.path().string()};
        debug_log(err_msg);
        m_success = false;
        return;
    }

    struct stat stat_obj;
    int stat_res{lstat(symlink_path.c_str(), &stat_obj)};

    assert(stat_res != -1);
    if(stat_res == -1) {
        std::string err_msg{"stat_res returned -1 at dir_sym_entry constructor with symlink_path: " + symlink_path.string()};
        debug_log(err_msg);
        m_success = false;
        return;
    }

    const std::string& actual_filename{symlink_path.filename().string()};
    std::strcpy(m_name, actual_filename.data());

    m_name_length = actual_filename.length();

    const fs::path& plain_target_path{fs::read_symlink(dir, err)};
    memcpy(m_secondary_path, plain_target_path.c_str(), plain_target_path.string().length());
    m_secondary_path_length = plain_target_path.string().length();

    m_acc_time = {.sec = stat_obj.st_atim.tv_sec, .nsec = stat_obj.st_atim.tv_nsec};
    m_mod_time = {.sec = stat_obj.st_mtim.tv_sec, .nsec = stat_obj.st_mtim.tv_nsec};
    m_sc_time = {.sec = stat_obj.st_ctim.tv_sec, .nsec = stat_obj.st_ctim.tv_nsec};
    m_mode = std::to_underlying(file_obj.entry_obj().symlink_status(err).permissions());

    m_success = true;
}

static bool pack_handle_regular_file(std::string_view full_path, File_W& pack_file, const u8 opts,
                                     std::string_view alt_filename = "") {
    // alt_filename is used to name a followed symlink as the name of the symlink rather than target's symlink
    // to preserve dir structure

    file_entry file_data{full_path, opts};

    if(!file_data.m_success) {
        std::string err_msg{"in pack_handle_regular_file, file_data.m_success was false with full_path: " +
                            std::string{full_path}};
        debug_log(err_msg);
        return false;
    }

    if(alt_filename.length() > 0) {
        memset(file_data.m_filename, '\0', file_data.m_filename_length);

        memcpy(file_data.m_filename, alt_filename.data(), alt_filename.length());
        file_data.m_filename_length = alt_filename.size();
    }

    special_marker file_marker = {.type = ENT_FILE};
    if(!pack_file.write(reinterpret_cast<char*>(&file_marker), sizeof(special_marker))) {
        std::string err_msg{"in pack_handle_regular_file, pack_file.write() failed with file_path: " + std::string{full_path}};
        debug_log(err_msg);
        return false;
    }

    if(!pack_file.write(reinterpret_cast<char*>(&file_data), sizeof(file_entry))) {
        std::string err_msg{"in pack_handle_regular_file, pack_file.write() failed with file_path: " + std::string{full_path}};
        debug_log(err_msg);
        return false;
    }

    // check if file has actually some data and size != 0 before writing data
    if(file_data.m_size > 0) {
        File_R file_stream{full_path};
        if(!file_stream) {
            std::string err_msg{"in pack_handle_regular_file, file_stream's constructor failed with full_path: " +
                                std::string{full_path} + " and m_error_message: " + std::string{file_stream.err()}};
            debug_log(err_msg);
            return false;
        }

        if(!file_stream.setup_stream(open_type::exists)) {
            std::string err_msg{"in pack_handle_regular_file, file_stream.setup_stream() failed with file_path: " +
                                std::string{full_path}};
            debug_log(err_msg);
            return false;
        }

        off_t pack_file_offset{pack_file.get_offset()};

        bool copy_res{packr::copy_file_range(file_stream, 0, pack_file, pack_file_offset, file_data.m_size)};

        assert(copy_res);

        if(!copy_res) {
            std::string err_msg{"failed to copy files in pack_handle_regular_file"};
            debug_log(err_msg);
        }
    }

    return true;
}

static bool pack_dir_as_symlink(const fs::directory_entry& dir, File_W& pack_file) {
    std::error_code err;
    assert(fs::is_symlink(dir, err) && fs::is_directory(dir, err));

    dir_sym_entry ent_data{dir};
    assert(ent_data.m_success);

    special_marker file_marker = {.type = ENT_DIR_SYM};
    if(!pack_file.write(reinterpret_cast<char*>(&file_marker), sizeof(special_marker))) {
        std::string err_msg{"in pack_dir_as_symlink, pack_file.write() failed with target path: " + dir.path().string()};
        debug_log(err_msg);
        return false;
    }

    if(!pack_file.write(reinterpret_cast<char*>(&ent_data), sizeof(dir_sym_entry))) {
        std::string err_msg{"in pack_dir_as_symlink, pack_file.write() failed with target path: " + dir.path().string()};
        debug_log(err_msg);
        return false;
    }

    return true;
}

static bool pack_handle_dir(const fs::directory_entry& entry, File_W& pack_file, const u8 opts, const u32 nest_count,
                            anc_map_t& anc_table) {
    dir_entry dir_data{entry, nest_count, opts, anc_table};
    if(dir_data.m_success == dir_entry_ret_code::fail) {
        std::string err_msg{"in pack_handle_dir, dir_data_inner.m_success failed with entry.path(): " + entry.path().string()};
        debug_log(err_msg);
        return false;
    }

    if(dir_data.m_success == dir_entry_ret_code::recursive) {
        [[maybe_unused]] bool res{pack_dir_as_symlink(entry, pack_file)};
        assert(res && "pack_dir_as_symlink failed in pack_handle_dir");

        std::string err_msg{"skipped a recursive code path: " + entry.path().string()};
        debug_log(err_msg, log_type::info);
        return true;
    }

    if(!dir_data.pack_dir(entry, pack_file, opts, nest_count + 1, anc_table)) {
        std::string err_msg{"in pack_handle_dir, dir_data_inner.pack_dir() failed with entry.path(): " + entry.path().string()};
        debug_log(err_msg);
        return false;
    }

    return true;
}

// This is to be called inside of pack_handle_symlink
static bool pack_a_symlink(std::string_view full_path, File_W& pack_file, const u8 opts) {
    file_entry file_data{full_path, opts};

    special_marker file_marker = {.type = ENT_FILE};
    if(!pack_file.write(reinterpret_cast<char*>(&file_marker), sizeof(special_marker))) {
        std::string err_msg{"in pack_a_symlink, pack_file.write() failed with full_path: " + std::string{full_path}};
        debug_log(err_msg);
        return false;
    }

    if(!pack_file.write(reinterpret_cast<char*>(&file_data), sizeof(file_entry))) {
        std::string err_msg{"in pack_a_symlink, pack_file.write() failed with full_path: " + std::string{full_path}};
        debug_log(err_msg);
        return false;
    }

    assert(file_data.m_size >= 0);
    // check if file has actually some data and size != 0 before writing file
    // contents
    if(file_data.m_size > 0) {
        File_R file_stream{full_path};
        if(!file_stream.setup_stream(open_type::exists)) {
            return false;
        }

        off_t pack_file_offset{pack_file.get_offset()};

        bool copy_res{packr::copy_file_range(file_stream, 0, pack_file, pack_file_offset, file_data.m_size)};

        if(!copy_res) {
            std::string err_msg{"failed to copy files in pack_a_symlink"};
            debug_log(err_msg);
        }
        assert(copy_res);
    }

    return true;
}

static void pack_handle_symlink(const fs::directory_entry& entry, File_W& pack_file, const u8 opts, const u32 nest_count,
                                anc_map_t& anc_table) {
    std::error_code err;
    const bool follow_symlinks{(opts & O_SYM) > 0};
    const std::string full_path{entry.path().string()};
    fs::path target_path{packr::read_symlink(entry.path())};

    if(follow_symlinks && fs::is_regular_file(entry, err)) {
        assert(!target_path.empty() && "Failed to get symlink target path while handling a symlink");

        [[maybe_unused]] bool res{
            pack_handle_regular_file(target_path.string(), pack_file, opts, entry.path().filename().string())};
        assert(res && "Failed to handle regular file while handling its corrosponding symlink");

    } else if(follow_symlinks && fs::is_directory(entry, err)) {
        assert(!target_path.empty() && "Failed to get symlink target path while handling a symlink");

        [[maybe_unused]] bool res{pack_handle_dir(entry, pack_file, opts, nest_count, anc_table)};
        assert(res && "Failed to handle regular file while handling its corrosponding symlink");

    } else {
        assert(fs::is_symlink(entry, err)); // make sure it is not a special files since we ignore them
        // Other cases grouped here because:
        // 1) If !follow_symlinks && file is regular, it would be handled correctly in pack_a_symlink
        // 2) If file has no valid target, it would also be handled correctly
        // opts xor'd with O_SYM so that it's packed as a symlink rather than trying to get to its target(as it doesn't exist)
        [[maybe_unused]] bool res{pack_a_symlink(full_path, pack_file, follow_symlinks ? (opts ^ O_SYM) : opts)};
        assert(res && "Failed to handle regular file while handling its corrosponding symlink");
    }
}

bool dir_entry::pack_dir(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts, const u32 nest_count,
                         anc_map_t& anc_table) {
    std::error_code err;

    // write the dir header upfront only if it's the intial pack header(nest_count = 0)
    if(nest_count == 0) {
        if(!pack_file.write(reinterpret_cast<char*>(this), sizeof(dir_entry))) {
            return false;
        }
    }

    special_marker dir_marker_start = {.type = ENT_DIR_START};
    if(!pack_file.write(reinterpret_cast<char*>(&dir_marker_start), sizeof(special_marker))) {
        return false;
    }

    if(nest_count > 0) {
        if(!pack_file.write(reinterpret_cast<char*>(this), sizeof(dir_entry))) {
            return false;
        }
    }

    struct stat main_stat;
    int stat_res{stat(dir.path().c_str(), &main_stat)};

    if(stat_res == -1) {
        std::string err_msg{"stat_res returned -1 at dir_entry::pack_dir with dir.path(): " + dir.path().string()};
        debug_log(err_msg);
        m_success = dir_entry_ret_code::fail;
        return false;
    }
    assert(stat_res != -1);

    // No need to check here for return value since pack_handle_dir should already check for ancestory matches
    handle_dir_ancestory(main_stat, anc_table); // To add current ino and dev nums to the table

    for(const fs::directory_entry& curr_ent : fs::directory_iterator(dir, err)) {
        const std::string full_path{curr_ent.path().string()};

        packr::debug_log("current entry to pack: " + full_path, log_type::info);

        const fs::file_status ent_sym_status{curr_ent.symlink_status(err)};

        // Returns false in case curr_ent is a symlink to a directory
        if(fs::is_directory(ent_sym_status)) {
            [[maybe_unused]] bool res{pack_handle_dir(curr_ent, pack_file, opts, nest_count, anc_table)};
            assert(res && "Handling a directory failed");

        } else if(fs::is_symlink(ent_sym_status)) {
            pack_handle_symlink(curr_ent, pack_file, opts, nest_count, anc_table);

        } else if(fs::is_regular_file(ent_sym_status)) {
            // std::string_view full_path, dir_entry& dir_header_copy, File_W& pack_file, const u32 nest_count, const u8 opts
            [[maybe_unused]] bool res{pack_handle_regular_file(full_path, pack_file, opts)};
            assert(res && "Handling a regular file failed");
        } else {
            std::println(stderr, "WARNING skipping a special file: {}", curr_ent.path().string());
        }
    }

    const std::string dev_ino_str{std::to_string(main_stat.st_dev) + std::to_string(main_stat.st_ino)};
    anc_table.erase(dev_ino_str);

    special_marker dir_marker_end{.type = ENT_DIR_END};
    return pack_file.write(reinterpret_cast<char*>(&dir_marker_end), sizeof(special_marker)); // bool
}

bool pack_header::pack(const std::filesystem::directory_entry& dir, File_W& pack_file, const u8 opts) {
    special_marker pack_start_marker{.type = PACK_START};
    if(!pack_file.write(reinterpret_cast<char*>(&pack_start_marker), sizeof(special_marker))) {
        return false;
    }

    if(!pack_file.write(reinterpret_cast<const char*>(&opts), sizeof(opts))) {
        return false;
    }

    anc_map_t root_anc_table{};
    if(!pack_dir(dir, pack_file, opts, DEFAULT_ROOT_DIR, root_anc_table)) {
        return false;
    }

    special_marker pacK_end_marker{.type = PACK_END};
    return (pack_file.write(reinterpret_cast<char*>(&pacK_end_marker), sizeof(special_marker)));
}

bool dir_entry::unpack_dir(File_R& pack_file, const u8 opts, const u32 nest_count) {
    std::error_code err;
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
                file_entry curr_file_data;
                if(!pack_file.read(reinterpret_cast<char*>(&curr_file_data), sizeof(file_entry))) {
                    std::string err_msg{"in dir_entry::unpack_dir, pack_file.read() failed"};
                    debug_log(err_msg);
                    return false;
                }

                static const char* unnamed_filename = "unamed-file"; // Just in case the file had no name for some reason
                if(curr_file_data.m_filename_length < 1) {
                    // Copying it into curr_file_data.filename so a flag isn't needed
                    memcpy(curr_file_data.m_filename, unnamed_filename, strlen(unnamed_filename));
                }

                if(curr_file_data.m_type == file_type::symlink) {
                    assert(curr_file_data.m_filename_length > 0);

                    fs::directory_entry curr_file_fs{curr_file_data.m_filename};
                    // TEST: case
                    fs::create_symlink(curr_file_data.m_secondary_path_length > 0 ? curr_file_data.m_secondary_path : "",
                                       curr_file_fs, err);
                    curr_file_fs.refresh();
                    assert(fs::exists(curr_file_fs.symlink_status(err)));

                    continue;
                }

                // The rest is for regular files
                File_W target_file{curr_file_data.m_filename};
                if(!target_file.setup_stream(open_type::fresh)) {
                    std::string err_msg{
                        "in dir_entry::unpack_dir, target_file.setup_stream() failed with curr_file_data.m_filename: " +
                        std::string{curr_file_data.m_filename}};
                    debug_log(err_msg);
                    return false;
                }

                if(curr_file_data.m_size > 0) {
                    off_t pack_file_offset{pack_file.get_offset()};

                    bool copy_res{packr::copy_file_range(pack_file, pack_file_offset, target_file, 0, curr_file_data.m_size)};

                    assert(copy_res);

                    if(!copy_res) {
                        std::string err_msg{"failed to copy files in unpack_dir"};
                        debug_log(err_msg);
                    }
                }
            }
            break;

        case ENT_DIR_START:
            {
                dir_entry curr_dir_data;
                if(!pack_file.read(reinterpret_cast<char*>(&curr_dir_data), sizeof(dir_entry))) {
                    std::string err_msg{"in dir_entry::unpack_dir, pack_file.read() failed"};
                    debug_log(err_msg);
                    return false;
                }

                static const char* unnamed_dirname = "unamed-directory"; // Just in case the dir had no name for some reason
                if(curr_dir_data.m_dirname_length < 1) {
                    // Copying it into curr_dir_data.dirname so a flag isn't needed
                    memcpy(curr_dir_data.m_dirname, unnamed_dirname, strlen(unnamed_dirname));
                }

                if(mkdir(curr_dir_data.m_dirname, curr_dir_data.m_mode) == -1) {
                    std::string err_msg{"in dir_entry::unpack_dir, mkdir failed with curr_dir_data.m_dirname: " +
                                        std::string{curr_dir_data.m_dirname}};
                    debug_log(err_msg);
                    return false;
                }
                char* cwd{getcwd(nullptr, 0)};
                if(cwd == nullptr) {
                    std::string err_msg{"in dir_entry::unpack_dir, getcwd failed"};
                    debug_log(err_msg);
                    return false;
                }
                // The path of the newely created directory
                std::optional<std::string> target_dir_path{join_to_path(curr_dir_data.m_dirname, cwd)};
                if(!target_dir_path) {
                    std::string err_msg{"in dir_entry::unpack_dir, join_to_path failed with curr_dir_data.m_dirname: " +
                                        std::string{curr_dir_data.m_dirname} + " and cwd: " + std::string{cwd}};
                    debug_log(err_msg);
                    free(cwd);
                    return false;
                }

                std::string& target_dir_path_str{target_dir_path.value()};
                if(chdir(target_dir_path_str.data()) == -1) {
                    std::string err_msg{"in dir_entry::unpack_dir, chdir failed with target_dir_path: " + target_dir_path_str};
                    debug_log(err_msg);
                    free(cwd);
                    return false;
                }

                if(!unpack_dir(pack_file, opts, nest_count + 1)) {
                    std::string err_msg{"in dir_entry::unpack_dir, chdir failed with target_dir_path: " + target_dir_path_str};
                    debug_log(err_msg);
                    free(cwd);
                    return false;
                }

                // Return to curr after unpacking the sub dir
                if(chdir(cwd) == -1) {
                    std::string err_msg{"in dir_entry::unpack_dir, chdir failed with cwd: " + std::string{cwd}};
                    debug_log(err_msg);
                    free(cwd);
                    return false;
                }

                free(cwd);
            }
            break;

        case ENT_DIR_END:
            read_pack_file = false;
            break;

        case ENT_DIR_SYM:
            {
                dir_sym_entry ent_data;
                if(!pack_file.read(reinterpret_cast<char*>(&ent_data), sizeof(dir_sym_entry))) {
                    std::string err_msg{"in dir_entry::unpack_dir, pack_file.read() failed"};
                    debug_log(err_msg);
                    return false;
                }

                assert(ent_data.m_name_length > 0);

                fs::directory_entry ent_fs{ent_data.m_name};
                fs::create_symlink(ent_data.m_secondary_path_length > 0 ? ent_data.m_secondary_path : "", ent_fs, err);
                ent_fs.refresh();
                assert(fs::exists(ent_fs.symlink_status(err)));
            }
            break;

        default:
            std::string err_msg{"in dir_entry::unpack_dir, unkown curr_marker.type"};
            debug_log(err_msg);
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
        std::string err_msg{"in dir_entry::unpack, pack_file.read() failed"};
        debug_log(err_msg);
        return false;
    }
    if(pack_start_marker.type != PACK_START) {
        std::string err_msg{"in dir_entry::unpack, didn't find PACK_STARTER marker"};
        debug_log(err_msg);
        return false;
    }

    u8 pack_file_opts{};
    if(!pack_file.read(reinterpret_cast<char*>(&pack_file_opts), sizeof(opts))) {
        std::string err_msg{"in dir_entry::unpack, pack_file.read() failed"};
        debug_log(err_msg);
        return false;
    }

    // Reading pack_header
    dir_entry pack_header;
    if(!pack_file.read(reinterpret_cast<char*>(&pack_header), sizeof(dir_entry))) {
        std::string err_msg{"in dir_entry::unpack, pack_file.read() failed"};
        debug_log(err_msg);
        return false;
    }

    // This marks the start of the target root directory
    special_marker initial_dir_start_marker;
    if(!pack_file.read(reinterpret_cast<char*>(&initial_dir_start_marker),
                       static_cast<std::streamsize>(sizeof(special_marker)))) {
        std::string err_msg{"in dir_entry::unpack, pack_file.read() failed"};
        debug_log(err_msg);
        return false;
    }
    if(initial_dir_start_marker.type != ENT_DIR_START) {
        std::string err_msg{"in dir_entry::unpack, initial_dir_start_marker.type wasn't ENT_DIR_START"};
        debug_log(err_msg);
        return false;
    }

    // Making the root directory and changing into it
    if(mkdir(pack_header.m_dirname, pack_header.m_mode) == -1) {
        if(errno != EEXIST) {
            std::string err_msg{
                "in dir_entry::unpack, mkdir failed with an errno other than EEXIST, with pack_header.m_dirname " +
                std::string{pack_header.m_dirname} + " and pack_header.m_mode: " + std::to_string(pack_header.m_mode)};
            debug_log(err_msg);
            return false;
        }
    }
    char* cwd = getcwd(nullptr, 0);
    if(cwd == nullptr) {
        std::string err_msg{"in dir_entry::unpack, getcwd failed"};
        debug_log(err_msg);
        return false;
    }
    std::optional<std::string> root_dir_path{join_to_path(pack_header.m_dirname, cwd)};
    if(!root_dir_path) {
        std::string err_msg{"in dir_entry::unpack, join_to_path failed with pack_header.m_dirname: " +
                            std::string{pack_header.m_dirname} + " and cwd: " + std::string{cwd}};
        debug_log(err_msg);
        return false;
    }

    std::string& root_dir_path_str{root_dir_path.value()};
    if(chdir(root_dir_path_str.data()) == -1) {
        std::string err_msg{"in dir_entry::unpack, chdir failed with root_dir_path_str: " + root_dir_path_str};
        debug_log(err_msg);
        free(cwd);
        return false;
    }

    if(!unpack_dir(pack_file, 0, DEFAULT_ROOT_DIR)) {
        std::string err_msg{"in dir_entry::unpack, unpack_dir failed"};
        debug_log(err_msg);
        free(cwd);
        return false;
    }

    free(cwd);
    return true;
}

} // namespace packr

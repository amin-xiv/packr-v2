# packr-v2

Simple directory archiver.

## Description

Given a directory, the tool packs it into a single ``.packr`` file, that can be unpacked using the same tool.

## Getting Started

### Build Requirements

- GCC +V13 (to support ``C++ 23``)
- CMake +v4.0.0
- Any Linux system
- GoogleTest (for testing)

### Building

* Clone the repository
* Create a build directory
* Run `cmake -S [source dir] -B [build dir]`
* Run `cmake --build [name of build directory]`

### Usage
 #### Packing
 ```bash
./packr -p -l [directory path]
```
 #### Unpacking
 ```bash
 ./packr -u -l [.packr file path]
 ```

 #### Help
 ```bash
 ./packr --help
 ```

## Notice
* You will probably find some bugs as the tool is in its early stages
* For now, the program skips any symlinks

## Goals
- [ ] Simplify main()
- [ ] Add support for symlinks
- [ ] Add much more tests
- [ ] Corruption detection with hashes
- [ ] Self-extraction ability
- [ ] Incremental packing
- [ ] Compression
- [ ] Benchmark mode
- [ ] Encryption
- [ ] Multi-threaded execution
- [ ] Archive mounting

## License
This project is licensed under the GPL v3.0 License - see the LICENSE file for details.

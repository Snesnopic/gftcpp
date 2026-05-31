#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstdint>
#include <cstring>
#include <array>

namespace fs = std::filesystem;

// tencent gft header magic: "TGF\0\0\0\0\0"
constexpr std::array<uint8_t, 8> GFT_MAGIC = { 0x54, 0x47, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00 };
constexpr uint32_t GFT_HEADER_SIZE = 0x14;

void print_usage() {
    std::cout << "usage: gftcpp <--pack|--unpack> <input> <output>\n"
              << "  --pack:   wraps an image into a .gft container\n"
              << "  --unpack: extracts the image from a .gft container\n";
}

bool pack(const fs::path& input, const fs::path& output) {
    std::ifstream in(input, std::ios::binary);
    if (!in) {
        std::cerr << "cannot open input file: " << input << "\n";
        return false;
    }

    std::vector<uint8_t> payload((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::ofstream out(output, std::ios::binary);
    if (!out) {
        std::cerr << "cannot open output file: " << output << "\n";
        return false;
    }

    // write magic (8 bytes)
    out.write(reinterpret_cast<const char*>(GFT_MAGIC.data()), GFT_MAGIC.size());
    
    // padding (8 bytes)
    std::array<uint8_t, 8> padding = {0};
    out.write(reinterpret_cast<const char*>(padding.data()), padding.size());

    // header size (4 bytes, little endian) - fixed at 20 bytes (0x14)
    out.write(reinterpret_cast<const char*>(&GFT_HEADER_SIZE), sizeof(GFT_HEADER_SIZE));

    // write payload
    out.write(reinterpret_cast<const char*>(payload.data()), payload.size());
    out.close();

    std::cout << "successfully packed " << input << " into " << output << "\n";
    return true;
}

bool unpack(const fs::path& input, const fs::path& output) {
    std::ifstream in(input, std::ios::binary);
    if (!in) {
        std::cerr << "cannot open input file: " << input << "\n";
        return false;
    }

    std::vector<uint8_t> data((std::istreambuf_iterator(in)), std::istreambuf_iterator<char>());
    in.close();

    if (data.size() < GFT_HEADER_SIZE) {
        std::cerr << "file too small to be a gft\n";
        return false;
    }

    if (std::memcmp(data.data(), GFT_MAGIC.data(), GFT_MAGIC.size()) != 0) {
        std::cerr << "invalid gft magic\n";
        return false;
    }

    uint32_t header_size = *reinterpret_cast<uint32_t*>(data.data() + 0x10);
    if (data.size() <= header_size) {
        std::cerr << "gft file truncated or empty\n";
        return false;
    }

    std::ofstream out(output, std::ios::binary);
    if (!out) {
        std::cerr << "cannot open output file: " << output << "\n";
        return false;
    }

    out.write(reinterpret_cast<const char*>(data.data() + header_size), data.size() - header_size);
    out.close();

    std::cout << "successfully unpacked " << input << " into " << output << "\n";
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage();
        return 1;
    }

    const std::string command = argv[1];
    const fs::path input = argv[2];
    const fs::path output = argv[3];
    bool success;
    if (command == "--pack") {
        success = pack(input, output);
    } else if (command == "--unpack") {
        success = unpack(input, output);
    } else {
        print_usage();
        success = false;
    }

    if (success) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}

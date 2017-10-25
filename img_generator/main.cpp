#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include <dirent.h>

#include "CImg.h"

namespace generator {
namespace detail {

using pixel_t = unsigned char;

const int noise_amplification = 123;

std::vector<std::string> strings;
int height = 53;
int noise = 0;

std::string output;
bool output_specified = false;

void printUsage() {
    std::cout << "Usage:\n\n"
                 "  generate [options] {<string>}\n\n";
}

void printHelp() {
    std::cout
        << "Options:\n\n"
           "  --help                       Display this message.\n"
           "  -o, --output                 Specify output file or directory. Default = image.bmp.\n"
           "  -a, --all_chars              Generate all symbols from 'A' to 'Z'.\n"
           "  -h, --height <int>           Set the height of text in pixels. Default = 53.\n"
           "  -n, --noise [0..10]          Set the noise level to be applied. Default = 0.\n\n";
}

void applyNoise(cimg_library::CImg<pixel_t> &img) {
    for (int x = 0; x < img.width(); x++)
        for (int y = 0; y < img.height(); y++) {
            int delta = rand() % noise - noise / 2;
            img.atXY(x, y) = std::max(0, std::min(img.atXY(x, y) + delta, 255));
        }
}

void drawString(const std::string &str) {
    cimg_library::CImg<pixel_t> img;

    pixel_t black = 0;
    pixel_t white = 255;

    img.draw_text(0, 0, str.c_str(), &black, &white, 1, height);

    if (noise > 0)
        applyNoise(img);

    std::string file_name;

    DIR *dir = opendir(output.c_str());

    if (strings.size() > 1) {
        if (!dir)
            throw std::runtime_error("invalid directory specified for output");

        if (output.back() != '/')
            output += '/';

        if (output_specified) {
            file_name = output + str + ".bmp";
        } else
            file_name = str + ".bmp";
    } else {
        if (output_specified) {
            if (dir) {
                if (output.back() != '/')
                    output += '/';

                file_name = output + "image.bmp";
            } else
                file_name = output;
        } else
            file_name = "image.bmp";
    }

    closedir(dir);

    img.save_bmp(file_name.c_str());
}

} // namespace detail

void parseArgs(int argc, char **argv) {
    if (argc < 2) {
        detail::printUsage();
        std::cout << "Run 'generate --help' for more information.\n\n";
        return;
    }

    int i = 1;
    std::string arg = argv[i++];

    auto next_arg = [&]() { arg = i++ < argc ? argv[i - 1] : ""; };

    while (arg.substr(0, 1) == "-") {
        if (arg == "--help") {
            detail::printUsage();
            detail::printHelp();
            return;
        } else if (arg == "--output" || arg == "-o") {
            next_arg();

            if (i > argc)
                throw std::runtime_error("argument expected after --output option");

            detail::output = arg.c_str();
            detail::output_specified = true;
        } else if (arg == "--all_chars" || arg == "-a") {
            for (const char *pc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; *pc; pc++)
                detail::strings.push_back(std::string(1, *pc));
        } else if (arg == "--height" || arg == "-h") {
            next_arg();

            if (i > argc)
                throw std::runtime_error("argument expected after --height option");

            try {
                detail::height = std::stoi(arg);
            } catch (...) {
                throw std::runtime_error("invalid image height specified");
            }

            if (detail::height <= 0)
                throw std::runtime_error("invalid image height specified");
        } else if (arg == "--noise" || arg == "-n") {
            next_arg();

            if (i > argc)
                throw std::runtime_error("argument expected after --noise option");

            try {
                detail::noise = std::stoi(arg);
            } catch (...) {
                throw std::runtime_error("invalid noise level specified");
            }

            if (detail::noise < 0 || detail::noise > 10)
                throw std::runtime_error("invalid noise level specified");
        } else
            throw std::runtime_error("unknown command '" + arg + "'");

        next_arg();
    }

    if (i <= argc)
        detail::strings.push_back(arg);
    else if (detail::strings.empty())
        throw std::runtime_error("no string specified");
}

void run() {
    detail::noise *= detail::noise_amplification;
    std::for_each(detail::strings.begin(), detail::strings.end(), detail::drawString);
}

} // namespace generator

int main(int argc, char **argv) {
    srand(time(nullptr));

    try {
        generator::parseArgs(argc, argv);
        generator::run();
    } catch (const std::runtime_error &e) {
        std::cout << "error: " << e.what() << std::endl;
    }

    return 0;
}

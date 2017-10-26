#include <iostream>
#include <string>
#include <vector>

#include "CImg.h"

#include "valuewrapper.hpp"

namespace recognizer {

using namespace label;
using namespace cimg_library;

using pixel_t = unsigned char;
using image_t = cimg_library::CImg<pixel_t>;

namespace detail {

const char *dataPath = "data/";
const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

using value_t = ValueWrapper<uint64_t, oper::min, oper::plus>;

std::string file_name;
std::vector<image_t> chars;
std::vector<std::vector<value_t>> weights;
std::vector<int> path;
int img_width;

void printUsage() {
    std::cout << "Usage:\n\n"
                 "  recognize [options] <path_to_image>\n\n";
}

void printHelp() {
    std::cout << "Options:\n\n"
                 "  -h, --help                   Display this message.\n\n";
}

void loadChars() {
    std::cout << "Loading characters..." << std::endl;

    for (const char *pc = alphabet; *(pc + 1); pc++)
        chars.emplace_back(image_t::get_load_bmp((std::string(dataPath) + *pc + ".bmp").c_str()));

    image_t space(1, chars[0].height(), { 255 });
    chars.emplace_back(space);
}

value_t calcWeight(const image_t &img, const image_t &char_img, int pos) {
    uint32_t w = 0;

    for (int x = 0; x < char_img.width(); x++)
        for (int y = 0; y < char_img.height(); y++) {
            int32_t a = img.atXY(pos + x, y);
            int32_t b = char_img.atXY(x, y);

            w += (a - b) * (a - b);
        }

    return w;
}

void calcWeights(const image_t &img) {
    std::cout << "Building graph..." << std::endl;

    for (auto &char_img : chars) {
        std::vector<value_t> w;

        for (int i = 0; i < img.width() - char_img.width() + 1; i++)
            w.emplace_back(calcWeight(img, char_img, i));

        weights.emplace_back(std::move(w));
    }
}

void findPath() {
    std::cout << "Finding the shortest path..." << std::endl;

    std::vector<std::vector<value_t>> w_vert;
    w_vert.resize(img_width);

    for (auto &v : w_vert)
        v.resize(chars.size());

    for (int i = 0; i < img_width; i++) {
        for (size_t j = 0; j < chars.size(); j++) {
            if (i < chars[j].width() - 1)
                continue;

            int prev_i = i - chars[j].width();

            if (prev_i > 0)
                for (size_t k = 0; k < chars.size(); k++)
                    w_vert[i][j] += w_vert[prev_i][k] * weights[j][prev_i + 1];
            else
                w_vert[i][j] = weights[j][prev_i + 1];
        }

        path.emplace_back(value_t::argplus(w_vert[i]));
    }
}

std::string buildString() {
    std::string str;

    for (int i = int(path.size()) - 1; i >= 0;) {
        if (alphabet[path[i]] != ' ' || str.back() != ' ')
            str += alphabet[path[i]];

        i -= chars[path[i]].width();
    }

    return std::string(str.rbegin(), str.rend());
}

std::string recognize(const image_t &img) {
    img_width = img.width();

    loadChars();
    calcWeights(img);
    findPath();

    return buildString();
}

} // namespace detail

bool parseArgs(int argc, char **argv) {
    if (argc < 2) {
        detail::printUsage();
        std::cout << "Run 'recognize --help' for more information.\n\n";
        return false;
    }

    int i = 1;
    std::string arg = argv[i++];

    auto next_arg = [&]() { arg = i++ < argc ? argv[i - 1] : ""; };

    while (arg.substr(0, 1) == "-") {
        if (arg == "--help") {
            detail::printUsage();
            detail::printHelp();
            return false;
        } else
            throw std::runtime_error("unknown command '" + arg + "'");

        next_arg();
    }

    if (i <= argc)
        detail::file_name = arg;
    else
        throw std::runtime_error("no string specified");

    return true;
}

std::string recognize() {
    recognizer::image_t img = image_t::get_load_bmp(detail::file_name.c_str());
    return detail::recognize(img);
}

} // namespace recognizer

int main(int argc, char **argv) {
    try {
        if (recognizer::parseArgs(argc, argv))
            std::cout << "\n\"" << recognizer::recognize() << "\"\n" << std::endl;
    } catch (const std::runtime_error &e) {
        std::cout << "error: " << e.what() << std::endl;
    }

    return 0;
}

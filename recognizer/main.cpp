#include <iostream>
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

std::vector<image_t> chars;
std::vector<std::vector<value_t>> weights;
std::vector<int> path;
int img_width;

void loadChars() {
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
    for (auto &char_img : chars) {
        std::vector<value_t> w;

        for (int i = 0; i < img.width() - char_img.width() + 1; i++)
            w.emplace_back(calcWeight(img, char_img, i));

        weights.emplace_back(std::move(w));
    }
}

void findPath() {
    std::vector<std::vector<value_t>> w_vert;
    w_vert.resize(img_width);

    for (auto &v : w_vert) {
        v.reserve(chars.size());

        for (size_t j = 0; j < chars.size(); j++)
            v.emplace_back(value_t::zero());
    }

    for (size_t j = 0; j < chars.size(); j++)
        w_vert[0][j] = weights[j][0];

    for (int i = 1; i < img_width; i++) {
        for (size_t j = 0; j < chars.size(); j++) {
            if (i < chars[j].width())
                continue;

            size_t prev_i = i - chars[j].width();

            for (size_t k = 0; k < chars.size(); k++)
                w_vert[i][j] += w_vert[prev_i][k] * weights[j][prev_i];
        }

        path.emplace_back(value_t::argplus(w_vert[i]));
    }
}

std::string buildString() {
    std::string str;

    for (int k = 10; k >= 1; k--) {
        str = "";

        for (int i = int(path.size()) - k; i >= 0;) {
            str += alphabet[path[i]];
            i -= chars[path[i]].width();
        }

        std::cout << "\"" << std::string(str.rbegin(), str.rend()) << "\"" << std::endl;
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

std::string recognize(const image_t &img) {
    return detail::recognize(img);
}

} // namespace recognizer

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    recognizer::image_t img = recognizer::image_t::get_load_bmp("img.bmp");
    std::cout << '"' << recognizer::recognize(img) << '"' << std::endl;

    return 0;
}

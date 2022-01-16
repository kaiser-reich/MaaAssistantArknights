#include "HashImageAnalyzer.h"

#include "AsstUtils.hpp"

bool asst::HashImageAnalyzer::analyze()
{
    m_hash_result.clear();
    m_hamming_result.clear();
    m_result.clear();

    cv::Mat roi = m_image(utils::make_rect<cv::Rect>(m_roi));

    if (m_mask_range.first != 0 || m_mask_range.second != 0) {
        cv::Mat bin;
        cv::inRange(roi, m_mask_range.first, m_mask_range.second, bin);
        roi = bin;
    }

    std::vector<cv::Mat> to_hash_vector;
    if (m_need_split) {
        to_hash_vector = split_bin(roi);
    }
    else {
        to_hash_vector.emplace_back(roi);
    }

    for (auto&& to_hash : to_hash_vector) {
        if (m_need_bound) {
            to_hash = bound_bin(to_hash);
        }
        std::string hash_result = shash(to_hash);

        decltype(m_hamming_result)::value_type cur_hamming;
        std::string min_hm_name;

        int min_hamming = INT_MAX;
        for (auto&& [name, templ] : m_hash_templates) {
            int hm = hamming(hash_result, templ);
            cur_hamming.emplace(name, hm);
            if (hm < min_hamming) {
                min_hm_name = name;
                min_hamming = hm;
            }
        }
        m_result.emplace_back(min_hm_name);
        m_hamming_result.emplace_back(std::move(cur_hamming));

        m_hash_result.emplace_back(std::move(hash_result));
    }

    return true;
}

void asst::HashImageAnalyzer::set_mask_range(int lower, int upper) noexcept
{
    m_mask_range = std::make_pair(lower, upper);
}

void asst::HashImageAnalyzer::set_mask_range(std::pair<int, int> mask_range) noexcept
{
    m_mask_range = std::move(mask_range);
}

void asst::HashImageAnalyzer::set_hash_templates(std::unordered_map<std::string, std::string> hash_templates) noexcept
{
    m_hash_templates = std::move(hash_templates);
}

void asst::HashImageAnalyzer::set_need_split(bool need_split) noexcept
{
    m_need_split = need_split;
}

void asst::HashImageAnalyzer::set_need_bound(bool need_bound) noexcept
{
    m_need_bound = need_bound;
}

const std::vector<std::string>& asst::HashImageAnalyzer::get_result() const noexcept
{
    return m_result;
}

std::string asst::HashImageAnalyzer::shash(const cv::Mat& gray)
{
    constexpr static int HashKernelSize = 16;
    cv::Mat resized;
    cv::resize(gray, resized, cv::Size(HashKernelSize, HashKernelSize));
    std::stringstream hash_value;
    uchar* pix = resized.data;
    int tmp_dec = 0;
    for (int ro = 0; ro < 256; ro++) {
        tmp_dec = tmp_dec << 1;
        if (*pix > 127)
            tmp_dec++;
        if (ro % 4 == 3) {
            hash_value << std::hex << tmp_dec;
            tmp_dec = 0;
        }
        pix++;
    }
    return hash_value.str();
}

std::vector<cv::Mat> asst::HashImageAnalyzer::split_bin(const cv::Mat& bin)
{
    std::vector<cv::Mat> result;

    int range_start = 0;
    bool started = false;
    for (int i = 0; i != bin.cols; ++i) {
        bool line_without_true = true;
        for (int j = 0; j != bin.rows; ++j) {
            cv::uint8_t value = bin.at<cv::uint8_t>(j, i);

            if (value) {
                line_without_true = false;
                if (!started) {
                    started = true;
                    range_start = i;
                }
                break;
            }
        }
        if (started && line_without_true) {
            started = false;
            cv::Mat range_img = bin(cv::Range::all(), cv::Range(range_start, i));
            result.emplace_back(range_img);
        }
    }
    if (started) {
        started = false;
        cv::Mat range_img = bin(cv::Range::all(), cv::Range(range_start, bin.cols));
        result.emplace_back(range_img);
    }

    return result;
}

cv::Mat asst::HashImageAnalyzer::bound_bin(const cv::Mat& bin)
{
    return bin(cv::boundingRect(bin));
}

int asst::HashImageAnalyzer::hamming(std::string hash1, std::string hash2)
{
    constexpr static int HammingFlags = 64;

    hash1.insert(hash1.begin(), HammingFlags - hash1.size(), '0');
    hash2.insert(hash2.begin(), HammingFlags - hash2.size(), '0');
    int dist = 0;
    for (int i = 0; i < HammingFlags; i = i + 16) {
        unsigned long long x = strtoull(hash1.substr(i, 16).c_str(), nullptr, 16)
            ^ strtoull(hash2.substr(i, 16).c_str(), nullptr, 16);
        while (x) {
            dist++;
            x = x & (x - 1);
        }
    }
    return dist;
}

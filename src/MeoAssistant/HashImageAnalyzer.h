#pragma once
#include "AbstractImageAnalyzer.h"

#include <unordered_map>

namespace asst
{
    class HashImageAnalyzer : public AbstractImageAnalyzer
    {
    public:
        using AbstractImageAnalyzer::AbstractImageAnalyzer;
        virtual ~HashImageAnalyzer() = default;

        virtual bool analyze() override;

        void set_mask_range(int lower, int upper) noexcept;
        void set_mask_range(std::pair<int, int> mask_range) noexcept;
        void set_hash_templates(std::unordered_map<std::string, std::string> hash_templates) noexcept;
        void set_need_split(bool need_split) noexcept;
        void set_need_bound(bool need_bound) noexcept;

        const std::vector<std::string>& get_result() const noexcept;

    protected:
        static std::string shash(const cv::Mat& gray);
        static int hamming(std::string hash1, std::string hash2);
        static std::vector<cv::Mat> split_bin(const cv::Mat& bin);
        static cv::Mat bound_bin(const cv::Mat& bin);

        std::pair<int, int> m_mask_range;
        std::unordered_map<std::string, std::string> m_hash_templates;
        bool m_need_split = false;
        bool m_need_bound = false;

        std::vector<std::string> m_hash_result;
        std::vector<std::unordered_map<std::string, int>> m_hamming_result;
        std::vector<std::string> m_result;
    };
}

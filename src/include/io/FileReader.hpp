#pragma once

#include "check.hpp"

#include <vector>
#include <filesystem>
#include <fstream>
#include <format>
#include <span>

namespace io {

    class FileReader {
    public:
        FileReader(const std::filesystem::path &path)
                : m_fin(path, std::ios::binary | std::ios::ate) {
            if (!std::filesystem::exists(path)) {
                THROW(std::format("invalid file path: {}", path.string()))
            }
            if (!m_fin.good()) {
                THROW(std::format("unable to open path: {}", path.string()));
            }
            m_size = m_fin.tellg();
            m_fin.seekg(std::ios::beg);
        }

        template<typename T = char>
        std::span<T> readFully() {
            m_buffer.resize(m_size);
            m_fin.read(m_buffer.data(), m_size);
            T *ptr = reinterpret_cast<T *>(m_buffer.data());
            return {ptr, m_size / sizeof(T)};
        }

        template<typename T = char>
        [[nodiscard]] int64_t size() const {
            return m_size / sizeof(T);
        }

    private:
        std::vector<char> m_buffer;
        std::ifstream m_fin;
        int64_t m_size;
    };
}
#pragma once
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace chq {
using PackBytes = std::vector<std::uint8_t>;

class ContentPack {
public:
    void set(std::string name, PackBytes data);
    const PackBytes* find(const std::string& name) const;
    bool save(const std::filesystem::path& path, std::string* error = nullptr) const;
    bool load(const std::filesystem::path& path, std::string* error = nullptr);
    std::size_t entry_count() const { return entries_.size(); }
    const std::map<std::string, PackBytes>& entries() const { return entries_; }
private:
    std::map<std::string, PackBytes> entries_;
};

// Overlay is a sparse CHQPACK1 container. Only named replacement regions are applied.
// Fail closed for unknown names and mismatched region sizes.
bool apply_content_overlay(ContentPack& base, const ContentPack& overlay, std::string* error = nullptr);
std::uint32_t content_crc32(const PackBytes& bytes);
}

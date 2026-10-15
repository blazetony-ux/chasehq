#include "content_pack.h"
#include <array>
#include <fstream>
#include <limits>

namespace chq {
namespace {
constexpr std::array<char,8> kMagic{{'C','H','Q','P','A','C','K','1'}};
template<class T> void write_le(std::ostream& out, T v) {
    for (std::size_t i=0;i<sizeof(T);++i) out.put(static_cast<char>((v >> (i*8)) & 0xff));
}
template<class T> bool read_le(std::istream& in, T& v) {
    v=0; for(std::size_t i=0;i<sizeof(T);++i){ int c=in.get(); if(c==EOF) return false; v |= static_cast<T>(static_cast<unsigned char>(c)) << (i*8); } return true;
}
}
std::uint32_t content_crc32(const PackBytes& bytes) {
    std::uint32_t crc=0xffffffffu;
    for(auto b:bytes){ crc^=b; for(int i=0;i<8;++i) crc=(crc>>1)^((crc&1u)?0xedb88320u:0u); }
    return ~crc;
}
void ContentPack::set(std::string name, PackBytes data){ entries_[std::move(name)]=std::move(data); }
const PackBytes* ContentPack::find(const std::string& name) const { auto it=entries_.find(name); return it==entries_.end()?nullptr:&it->second; }
bool ContentPack::save(const std::filesystem::path& path, std::string* error) const {
    std::ofstream out(path,std::ios::binary|std::ios::trunc); if(!out){if(error)*error="cannot create pack";return false;}
    out.write(kMagic.data(),kMagic.size()); write_le<std::uint32_t>(out,1); write_le<std::uint32_t>(out,static_cast<std::uint32_t>(entries_.size()));
    for(const auto& [name,data]:entries_){ if(name.size()>65535){if(error)*error="entry name too long";return false;} write_le<std::uint16_t>(out,static_cast<std::uint16_t>(name.size())); out.write(name.data(),static_cast<std::streamsize>(name.size())); write_le<std::uint64_t>(out,static_cast<std::uint64_t>(data.size())); write_le<std::uint32_t>(out,content_crc32(data)); if(!data.empty()) out.write(reinterpret_cast<const char*>(data.data()),static_cast<std::streamsize>(data.size())); }
    if(!out){if(error)*error="pack write failed";return false;} return true;
}
bool ContentPack::load(const std::filesystem::path& path, std::string* error) {
    entries_.clear(); std::ifstream in(path,std::ios::binary); if(!in){if(error)*error="cannot open pack: "+path.string();return false;}
    std::array<char,8> magic{}; in.read(magic.data(),magic.size()); std::uint32_t version=0,count=0;
    if(!in || magic!=kMagic || !read_le(in,version) || version!=1 || !read_le(in,count) || count>1024){if(error)*error="invalid CHQPACK1 header";return false;}
    for(std::uint32_t i=0;i<count;++i){ std::uint16_t nl=0; std::uint64_t sz=0; std::uint32_t crc=0; if(!read_le(in,nl)||nl==0){if(error)*error="invalid pack entry";return false;} std::string name(nl,'\0'); in.read(name.data(),nl); if(!in||!read_le(in,sz)||!read_le(in,crc)||sz>(1ull<<31)){if(error)*error="invalid pack entry metadata";return false;} PackBytes data(static_cast<std::size_t>(sz)); if(sz) in.read(reinterpret_cast<char*>(data.data()),static_cast<std::streamsize>(sz)); if(!in||content_crc32(data)!=crc){if(error)*error="pack CRC failure: "+name;return false;} if(!entries_.emplace(std::move(name),std::move(data)).second){if(error)*error="duplicate pack entry";return false;} }
    return true;
}
bool apply_content_overlay(ContentPack& base, const ContentPack& overlay, std::string* error) {
    if (overlay.entry_count() == 0) { if(error)*error="empty overlay"; return false; }
    for (const auto& [name, bytes] : overlay.entries()) {
        if (name.rfind("graphics/", 0) != 0 && name.rfind("prom/", 0) != 0) {
            if(error)*error="unsupported overlay region: " + name; return false;
        }
        const auto* original = base.find(name);
        if (!original || original->size() != bytes.size()) {
            if(error)*error="missing or size-mismatched overlay region: " + name; return false;
        }
    }
    for (const auto& [name, bytes] : overlay.entries()) base.set(name, bytes);
    return true;
}

}

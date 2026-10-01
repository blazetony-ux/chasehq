#include "machine.h"
#include "version.h"
#include "video.h"
#include "runtime.h"
#include "debug_api.h"
#include "sprite_gfx.h"
#include <chrono>
#include <iostream>
#include <thread>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <map>
#include <cmath>
#include <array>
#include <cstdint>
#include <vector>
#include <set>
#include <map>
#include <cstring>
#include <cctype>
#include <ctime>



static std::vector<std::string> split_command_words(const std::string& line) {
    std::vector<std::string> out; std::string cur; bool quote=false; char qchar=0;
    for(char c:line){
        if(quote){ if(c==qchar){quote=false;} else cur+=c; continue; }
        if(c=='\''||c=='"'){ quote=true; qchar=c; continue; }
        if(std::isspace(static_cast<unsigned char>(c))){ if(!cur.empty()){out.push_back(cur);cur.clear();} }
        else cur+=c;
    }
    if(!cur.empty()) out.push_back(cur); return out;
}
static bool parse_api_u32(const std::string& text, std::uint32_t& out){
    try { std::size_t n=0; int base=10; std::string t=text;
        if(t.size()>2&&t[0]=='0'&&(t[1]=='x'||t[1]=='X')) base=16;
        else if(t.find_first_of("abcdefABCDEF")!=std::string::npos) base=16;
        auto v=std::stoull(t,&n,base); if(n!=t.size()||v>0xffffffffull) return false; out=static_cast<std::uint32_t>(v); return true;
    } catch(...){ return false; }
}
static bool parse_api_hex_u32(const std::string& text, std::uint32_t& out){
    try { std::string t=text; if(t.size()>2&&t[0]=='0'&&(t[1]=='x'||t[1]=='X')) t=t.substr(2); std::size_t n=0; auto v=std::stoull(t,&n,16); if(n!=t.size()||v>0xffffffffull) return false; out=static_cast<std::uint32_t>(v); return true; } catch(...){ return false; }
}
static bool parse_api_width(const std::string& text, unsigned& bytes){
    std::uint32_t v=0; if(!parse_api_u32(text,v)) return false;
    if(v==8||v==1){bytes=1;return true;} if(v==16||v==2){bytes=2;return true;} if(v==32||v==4){bytes=4;return true;} return false;
}
static bool parse_api_space(const std::string& text, chq::BusSpace& space){
    if(text=="A"||text=="a"||text=="main"||text=="MAIN"){space=chq::BusSpace::Main;return true;}
    if(text=="B"||text=="b"||text=="sub"||text=="SUB"){space=chq::BusSpace::Sub;return true;} return false;
}
static bool parse_api_range(const std::string& text, std::uint32_t& start, std::uint32_t& end){
    const auto p=text.find(':');
    if(p==std::string::npos){ if(!parse_api_hex_u32(text,start)) return false; end=start; return true; }
    if(!parse_api_hex_u32(text.substr(0,p),start)||!parse_api_hex_u32(text.substr(p+1),end)) return false;
    start&=0xffffffu; end&=0xffffffu; if(end<start) std::swap(start,end); return true;
}


static std::uint32_t crc32_bytes(const std::uint8_t* data, std::size_t n);
static std::string json_escape(const std::string& s);
static void put_le16(std::ostream& o, std::uint16_t v){ char b[2]={(char)(v&255),(char)((v>>8)&255)}; o.write(b,2); }
static void put_le32(std::ostream& o, std::uint32_t v){ char b[4]={(char)(v&255),(char)((v>>8)&255),(char)((v>>16)&255),(char)((v>>24)&255)}; o.write(b,4); }
struct ZipCentralEntry { std::string name; std::uint32_t crc=0,size=0,offset=0; };
static bool zip_directory_store(const std::filesystem::path& source, const std::filesystem::path& zip_path) {
    std::error_code mkec;
    if (!zip_path.parent_path().empty()) std::filesystem::create_directories(zip_path.parent_path(), mkec);
    std::ofstream out(zip_path,std::ios::binary); if(!out) return false;
    std::vector<ZipCentralEntry> entries;
    std::error_code ec;
    for(auto it=std::filesystem::recursive_directory_iterator(source,ec);!ec&&it!=std::filesystem::recursive_directory_iterator();it.increment(ec)){
        if(!it->is_regular_file()) continue;
        std::error_code eqec; if(std::filesystem::equivalent(it->path(),zip_path,eqec) && !eqec) continue;
        auto rel=std::filesystem::relative(it->path(),source,ec).generic_string(); if(ec){ec.clear();continue;}
        std::ifstream in(it->path(),std::ios::binary); if(!in) continue;
        std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(in)),{});
        ZipCentralEntry e; e.name=rel; e.size=(std::uint32_t)data.size(); e.crc=crc32_bytes(data.data(),data.size()); e.offset=(std::uint32_t)out.tellp();
        put_le32(out,0x04034b50);put_le16(out,20);put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le32(out,e.crc);put_le32(out,e.size);put_le32(out,e.size);put_le16(out,(std::uint16_t)e.name.size());put_le16(out,0);out.write(e.name.data(),(std::streamsize)e.name.size());if(!data.empty())out.write((char*)data.data(),(std::streamsize)data.size());entries.push_back(e);
    }
    const auto central_start=(std::uint32_t)out.tellp();
    for(const auto&e:entries){put_le32(out,0x02014b50);put_le16(out,20);put_le16(out,20);put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le32(out,e.crc);put_le32(out,e.size);put_le32(out,e.size);put_le16(out,(std::uint16_t)e.name.size());put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le16(out,0);put_le32(out,0);put_le32(out,e.offset);out.write(e.name.data(),(std::streamsize)e.name.size());}
    const auto central_end=(std::uint32_t)out.tellp(); put_le32(out,0x06054b50);put_le16(out,0);put_le16(out,0);put_le16(out,(std::uint16_t)entries.size());put_le16(out,(std::uint16_t)entries.size());put_le32(out,central_end-central_start);put_le32(out,central_start);put_le16(out,0); return (bool)out;
}
static std::string ps_quote(std::string s){std::string o;for(char c:s){if(c=='\'')o+="''";else o+=c;}return o;}
static bool zip_directory_best(const std::filesystem::path& source,const std::filesystem::path& zip_path){
    // v0.47: use the in-process portable ZIP writer directly.  This avoids
    // Windows sharing/PowerShell Compress-Archive failures when a diagnostic
    // stream has only just been closed.  Reliability is more important than
    // compression ratio for forensic bundles.
    std::error_code ec; std::filesystem::remove(zip_path,ec);
    return zip_directory_store(source,zip_path);
}
static void write_artifact_validation(const std::filesystem::path& root){
    std::ofstream out(root/"artifact_validation.txt");
    out<<"Chase H.Q. Native artifact validation\n";
    std::error_code ec; std::size_t files=0,zero=0;
    for(auto it=std::filesystem::recursive_directory_iterator(root,ec);!ec&&it!=std::filesystem::recursive_directory_iterator();it.increment(ec)){
        if(!it->is_regular_file()) continue; ++files;
        const auto n=it->file_size(ec); if(ec){ec.clear();continue;}
        const auto rel=std::filesystem::relative(it->path(),root,ec).generic_string(); if(ec){ec.clear();continue;}
        if(rel=="artifact_validation.txt") continue; // report is still open while inventory is built
        out<<rel<<","<<n<<(n==0?",ZERO":" ,OK")<<"\n"; if(n==0)++zero;
    }
    out<<"files="<<files<<"\nzero_byte_files="<<zero<<"\n";
}
static std::size_t count_matching_files(const std::filesystem::path& root, const std::string& prefix, const std::string& extension) {
    std::error_code ec; std::size_t count=0;
    if(!std::filesystem::exists(root,ec)) return 0;
    for(auto it=std::filesystem::recursive_directory_iterator(root,ec);!ec&&it!=std::filesystem::recursive_directory_iterator();it.increment(ec)){
        if(!it->is_regular_file()) continue;
        const auto name=it->path().filename().string();
        if(name.rfind(prefix,0)==0 && it->path().extension()==extension) ++count;
    }
    return count;
}
static void write_run_status(const std::filesystem::path& root,unsigned start_frame,unsigned final_frame,unsigned target_frame,bool bounded,bool completed,const std::string& fault,std::size_t sprite_csvs){
    std::filesystem::create_directories(root); std::ofstream f(root/"run_status.json"); if(!f)return;
    f<<"{\n  \"build\": \""<<chq::kNativeVersion<<"\",\n"
     <<"  \"start_frame\": "<<start_frame<<",\n  \"final_frame\": "<<final_frame<<",\n"
     <<"  \"target_frame\": "<<(bounded?target_frame:0)<<",\n  \"bounded\": "<<(bounded?"true":"false")<<",\n"
     <<"  \"completed\": "<<(completed?"true":"false")<<",\n  \"sprite_evidence_files\": "<<sprite_csvs<<",\n"
     <<"  \"fault\": \""<<json_escape(fault)<<"\"\n}\n";
}
static std::string safe_name(std::string s){for(char&c:s)if(!std::isalnum((unsigned char)c)&&c!='-'&&c!='_')c='-';return s;}
static std::string timestamp_name(){std::time_t t=std::time(nullptr);std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm,&t);
#else
    localtime_r(&t,&tm);
#endif
    std::ostringstream o;o<<std::put_time(&tm,"%Y%m%d-%H%M%S");return o.str();}
static std::string console_stamp(const std::chrono::steady_clock::time_point& start){
    using namespace std::chrono; auto now=system_clock::now(); auto tt=system_clock::to_time_t(now); std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm,&tt);
#else
    localtime_r(&tt,&tm);
#endif
    auto ms=duration_cast<milliseconds>(now.time_since_epoch())%1000; auto elapsed=duration_cast<milliseconds>(steady_clock::now()-start).count();
    std::ostringstream o;o<<'['<<std::put_time(&tm,"%H:%M:%S")<<'.'<<std::setw(3)<<std::setfill('0')<<ms.count()<<" | +"<<std::fixed<<std::setprecision(3)<<(elapsed/1000.0)<<"] ";return o.str();
}
static std::filesystem::path default_bundle_name(const chq::Options&o){std::ostringstream n;n<<"ChaseHQ_v"<<chq::kNativeVersion<<'_'<<(o.scenario.empty()?"manual":safe_name(o.scenario));if(o.graphics_trace.pixel_provenance_pair)n<<"_pixel-pair"<<o.graphics_trace.pixel_provenance_pair->first<<'-'<<o.graphics_trace.pixel_provenance_pair->second;else if(!o.provenance.follow_ranges.empty())n<<"_follow-"<<std::hex<<o.provenance.follow_ranges.front().start;else if(o.graphics_trace.enabled())n<<"_graphics";n<<"_f"<<std::dec<<o.frames<<'_'<<timestamp_name()<<".zip";return o.logs.parent_path()/n.str();}
static int run_batch_jobs(const std::filesystem::path& file,const char* exe){
    std::ifstream in(file); if(!in){std::cerr<<"Cannot open batch file: "<<file.string()<<"\n";return 1;}
    const auto root=std::filesystem::path(std::string("batch_")+timestamp_name()); std::filesystem::create_directories(root); std::ofstream summary(root/"batch_summary.csv");summary<<"job,name,exit_code,bundle\n";
    std::string line;unsigned n=0,failed=0;
    while(std::getline(in,line)){auto hash=line.find('#');if(hash!=std::string::npos)line.resize(hash);auto first=line.find_first_not_of(" \t\r\n");if(first==std::string::npos)continue;line=line.substr(first);auto bar=line.find('|');std::string name,args;if(bar==std::string::npos){name="job"+std::to_string(n+1);args=line;}else{name=safe_name(line.substr(0,bar));args=line.substr(bar+1);}++n;if(name.empty())name="job"+std::to_string(n);auto jobdir=root/(std::to_string(n)+"_"+name);auto bundle=root/(std::to_string(n)+"_"+name+".zip");std::ostringstream cmd;cmd<<'"'<<exe<<'"'<<' '<<args<<" --logs \""<<jobdir.string()<<"\" --auto-zip-logs --zip-name \""<<bundle.string()<<"\"";std::cout<<"[BATCH "<<n<<"] "<<name<<"\n";int rc=std::system(cmd.str().c_str());if(rc!=0)++failed;summary<<n<<','<<name<<','<<rc<<','<<bundle.string()<<"\n";summary.flush();}
    std::cout<<"[BATCH COMPLETE] jobs="<<n<<" failed="<<failed<<" -> "<<root.string()<<"\n";return failed?2:0;
}


struct DriverProfilePoint {
    std::uint32_t course_position=0;
    unsigned bank=0, record=0;
    int steering=0;
    bool accel=true, brake=false;
    int speed_target=390;
};
static std::vector<std::string> split_csv_simple(const std::string& line){
    std::vector<std::string> out; std::string cur;
    for(char c:line){ if(c==','){out.push_back(cur);cur.clear();}else cur.push_back(c); } out.push_back(cur); return out;
}
static std::vector<DriverProfilePoint> load_driver_profile(const std::filesystem::path& path){
    std::ifstream in(path); if(!in) throw std::runtime_error("Cannot open course profile: "+path.string());
    std::vector<DriverProfilePoint> out; std::string line; bool first=true;
    while(std::getline(in,line)){
        if(first){first=false; if(line.rfind("frame,",0)==0) continue;}
        if(line.empty()) continue; auto c=split_csv_simple(line); if(c.size()<17) continue;
        try{ DriverProfilePoint p; p.course_position=static_cast<std::uint32_t>(std::stoul(c[1],nullptr,0)); p.bank=static_cast<unsigned>(std::stoul(c[2],nullptr,0)); p.record=static_cast<unsigned>(std::stoul(c[3],nullptr,0)); p.steering=std::stoi(c[13]); p.accel=std::stoi(c[14])!=0; p.brake=std::stoi(c[15])!=0; p.speed_target=std::stoi(c[16]); out.push_back(p); }catch(...){ }
    }
    std::sort(out.begin(),out.end(),[](const auto&a,const auto&b){return a.course_position<b.course_position;});
    if(out.empty()) throw std::runtime_error("Course profile contains no usable rows: "+path.string());
    return out;
}
static const DriverProfilePoint* nearest_profile_point(const std::vector<DriverProfilePoint>& v,std::uint32_t cp){
    if(v.empty()) return nullptr; auto it=std::lower_bound(v.begin(),v.end(),cp,[](const auto&p,std::uint32_t x){return p.course_position<x;});
    if(it==v.begin()) return &*it; if(it==v.end()) return &v.back(); const auto& a=*(it-1); const auto& b=*it;
    return (cp-a.course_position <= b.course_position-cp)?&a:&b;
}

static const chq::Region* find_region_named(const chq::Bus& bus, const char* name) {
    for (const auto& region : bus.regions) if (std::string(region.name) == name) return &region;
    return nullptr;
}
static std::uint16_t be16_at(const std::vector<std::uint8_t>& b, std::size_t o) {
    return o + 1 < b.size() ? static_cast<std::uint16_t>((b[o] << 8) | b[o+1]) : 0;
}


static std::uint32_t crc32_bytes(const std::uint8_t* data, std::size_t n) {
    std::uint32_t c=0xffffffffu; for(std::size_t i=0;i<n;++i){c^=data[i]; for(int k=0;k<8;++k)c=(c>>1)^(0xedb88320u & (0u-(c&1u)));} return c^0xffffffffu;
}
static std::uint32_t adler32_bytes(const std::vector<std::uint8_t>& d) {
    std::uint32_t a=1,b=0; for(auto v:d){a=(a+v)%65521u;b=(b+a)%65521u;} return (b<<16)|a;
}
static void put_be32(std::vector<std::uint8_t>& o,std::uint32_t v){o.push_back(v>>24);o.push_back(v>>16);o.push_back(v>>8);o.push_back(v);}
static void png_chunk(std::vector<std::uint8_t>& o,const char t[4],const std::vector<std::uint8_t>& d){put_be32(o,(std::uint32_t)d.size());std::size_t st=o.size();o.insert(o.end(),t,t+4);o.insert(o.end(),d.begin(),d.end());put_be32(o,crc32_bytes(o.data()+st,o.size()-st));}
static bool save_png_rgba(const std::filesystem::path& path,int w,int h,const std::vector<std::uint32_t>& argb) {
    if(w<=0||h<=0||argb.size()<(std::size_t)w*h)return false; std::vector<std::uint8_t> raw; raw.reserve((std::size_t)h*(1+w*4));
    for(int y=0;y<h;++y){raw.push_back(0);for(int x=0;x<w;++x){auto c=argb[(std::size_t)y*w+x];raw.push_back((c>>16)&255);raw.push_back((c>>8)&255);raw.push_back(c&255);raw.push_back((c>>24)&255);}}
    std::vector<std::uint8_t> z{0x78,0x01}; std::size_t pos=0; while(pos<raw.size()){std::size_t n=std::min<std::size_t>(65535,raw.size()-pos);bool last=pos+n==raw.size();z.push_back(last?1:0);std::uint16_t nn=(std::uint16_t)n,nc=(std::uint16_t)~nn;z.push_back(nn);z.push_back(nn>>8);z.push_back(nc);z.push_back(nc>>8);z.insert(z.end(),raw.begin()+pos,raw.begin()+pos+n);pos+=n;} put_be32(z,adler32_bytes(raw));
    std::vector<std::uint8_t> out{137,80,78,71,13,10,26,10}, ih; put_be32(ih,w);put_be32(ih,h);ih.insert(ih.end(),{8,6,0,0,0});png_chunk(out,"IHDR",ih);png_chunk(out,"IDAT",z);png_chunk(out,"IEND",{});std::ofstream f(path,std::ios::binary);f.write((char*)out.data(),(std::streamsize)out.size());return (bool)f;
}
static std::uint8_t export_sprite_pixel(const std::vector<std::uint8_t>& r,std::size_t tile,int x,int y){return chq::decode_sprite_pixel_4bpp(r,tile,x,y);}
static std::uint32_t export_palette(const chq::Bus& bus,std::uint8_t bank,std::uint8_t pen){if(!pen)return 0;auto raw=bus.palette[((bank&255)<<4)|(pen&15)];auto e=[](unsigned v){v&=31;return (std::uint8_t)((v<<3)|(v>>2));};auto r=e(raw),g=e(raw>>5),b=e(raw>>10);return 0xff000000u|((std::uint32_t)r<<16)|((std::uint32_t)g<<8)|b;}
struct ExportSprite { unsigned slot=0; std::uint16_t w[4]{}; int x=0,y=0,zx=0,zy=0,cols=0; std::size_t base=0; const std::vector<std::uint8_t>* gfx=nullptr; const char* gfxname=""; std::uint8_t color=0; bool fx=false,fy=false; };
static void blit_tile(std::vector<std::uint32_t>& dst,int dw,int dh,int ox,int oy,int tw,int th,const std::vector<std::uint8_t>& gfx,std::uint16_t code,bool fx,bool fy,const chq::Bus& bus,std::uint8_t color){for(int y=0;y<th;++y)for(int x=0;x<tw;++x){int sx=std::min(15,x*16/std::max(1,tw)),sy=std::min(15,y*16/std::max(1,th));if(fx)sx=15-sx;if(fy)sy=15-sy;auto pen=export_sprite_pixel(gfx,code,sx,sy);int dx=ox+x,dy=oy+y;if(pen&&dx>=0&&dy>=0&&dx<dw&&dy<dh)dst[(std::size_t)dy*dw+dx]=export_palette(bus,color,pen);}}
static std::uint32_t export_raw_pen(std::uint8_t pen){if(!pen)return 0;const unsigned v=static_cast<unsigned>(pen)*17u;return 0xff000000u|(v<<16)|(v<<8)|v;}
static void blit_tile_raw_pen(std::vector<std::uint32_t>& dst,int dw,int dh,int ox,int oy,int tw,int th,const std::vector<std::uint8_t>& gfx,std::uint16_t code,bool fx,bool fy){for(int y=0;y<th;++y)for(int x=0;x<tw;++x){int sx=std::min(15,x*16/std::max(1,tw)),sy=std::min(15,y*16/std::max(1,th));if(fx)sx=15-sx;if(fy)sy=15-sy;auto pen=export_sprite_pixel(gfx,code,sx,sy);int dx=ox+x,dy=oy+y;if(pen&&dx>=0&&dy>=0&&dx<dw&&dy<dh)dst[(std::size_t)dy*dw+dx]=export_raw_pen(pen);}}

static void export_tile_atlas(const std::filesystem::path& path,const std::vector<std::uint8_t>& gfx,const chq::Bus& bus,const char* label){
    const std::size_t tiles=(gfx.size()*8)/1024; if(!tiles)return; const int cols=64, cell=16; const int rows=(int)((tiles+cols-1)/cols);
    std::vector<std::uint32_t> img((std::size_t)cols*cell*rows*cell,0);
    for(std::size_t t=0;t<tiles;++t){int ox=(int)(t%cols)*cell,oy=(int)(t/cols)*cell; for(int y=0;y<16;++y)for(int x=0;x<16;++x){auto pen=export_sprite_pixel(gfx,t,x,y); if(pen) img[(std::size_t)(oy+y)*cols*cell+ox+x]=export_palette(bus,0,pen);}}
    save_png_rgba(path,cols*cell,rows*cell,img); std::ofstream meta(path.string()+".txt"); meta<<label<<" tiles="<<tiles<<" grid="<<cols<<"x"<<rows<<" tile=16x16\n";
}
static void blit_variant(std::vector<std::uint32_t>& dst,int dw,int dh,int ox,int oy,int tw,int th,const std::vector<std::uint8_t>& gfx,std::uint16_t code,const chq::Bus& bus,std::uint8_t color){blit_tile(dst,dw,dh,ox,oy,tw,th,gfx,code,false,false,bus,color);}
static void export_sprite_assets(const chq::Machine& machine,const chq::Runtime& runtime,const chq::Options& options,unsigned frame){
    const auto* sr=find_region_named(runtime.bus,"sprites");if(!sr)return;auto root=options.logs/("sprite_export_frame_"+std::to_string(frame));std::filesystem::create_directories(root);std::vector<ExportSprite> all;
    for(std::size_t off=0;off+7<sr->bytes.size();off+=8){ExportSprite q;q.slot=(unsigned)(off/8);for(int i=0;i<4;++i)q.w[i]=be16_at(sr->bytes,off+i*2);int y=q.w[0]&0x1ff;if((q.w[3]&0x7ff)==0)continue;q.zy=((q.w[0]>>9)&0x7f)+1;q.zx=(q.w[1]&0x7f)+1;q.x=q.w[2]&0x1ff;if(q.x>0x140)q.x-=0x200;q.y=y+7;if(q.y>0x140)q.y-=0x200;q.color=(q.w[1]&0x7f80)>>7;q.fx=q.w[2]&0x4000;q.fy=q.w[2]&0x8000;auto fmt=chq::Machine::format_for_zoomx(q.zx);if(fmt==chq::SpriteFormat::Invalid)continue;if(fmt==chq::SpriteFormat::Obj128x128){q.cols=8;q.base=(std::size_t)(q.w[3]&0x7ff)<<6;q.gfx=&machine.sprites_a();q.gfxname="A";}else if(fmt==chq::SpriteFormat::Obj64x128){q.cols=4;q.base=((std::size_t)(q.w[3]&0x7ff)<<5)+0x20000;q.gfx=&machine.sprites_b();q.gfxname="B";}else{q.cols=2;q.base=((std::size_t)(q.w[3]&0x7ff)<<4)+0x30000;q.gfx=&machine.sprites_b();q.gfxname="B";}if(options.sprite_debug_large_only&&q.cols<4)continue;if(options.sprite_debug_index>=0&&options.sprite_debug_index!=(int)q.slot)continue;all.push_back(q);}
    std::ofstream csv(root/"sprites.csv");csv<<"slot,w0,w1,w2,w3,x,y,zoom_x,zoom_y,cols,color,priority,flip_x,flip_y,map_base,gfx\n";
    std::ofstream mapcsv,zoomcsv,summary;
    if(options.sprite_export_analysis){mapcsv.open(root/"map_entries.csv");mapcsv<<"slot,j,k,map_word,map_byte,code_raw,code_masked,code_be_raw,code_be_masked,code_highbits,gfx,tile_count,masked_in_range,is_ffff,is_zero,repeated_prev\n";zoomcsv.open(root/"zoom_boundaries.csv");zoomcsv<<"slot,axis,index,start,end,size,total,parts,zero_size\n";summary.open(root/"analysis.txt");summary<<"v"<<chq::kNativeVersion<<" sprite-pipeline analysis frame="<<frame<<"\n";}
    if(options.sprite_export_atlas){export_tile_atlas(root/"OBJ_A_tile_atlas.png",machine.sprites_a(),runtime.bus,"OBJ A");export_tile_atlas(root/"OBJ_B_tile_atlas.png",machine.sprites_b(),runtime.bus,"OBJ B");}
    std::set<std::pair<char,std::uint16_t>> exported_tiles;std::vector<std::vector<std::uint32_t>> thumbs,raw_thumbs;std::vector<std::pair<int,int>> dims;
    for(auto&q:all){std::ostringstream dn;dn<<"sprite_"<<std::setfill('0')<<std::setw(3)<<q.slot;auto dir=root/dn.str();std::filesystem::create_directories(dir);csv<<q.slot<<','<<std::hex<<q.w[0]<<','<<q.w[1]<<','<<q.w[2]<<','<<q.w[3]<<std::dec<<','<<q.x<<','<<q.y<<','<<q.zx<<','<<q.zy<<','<<q.cols<<','<<(int)q.color<<','<<((q.w[1]&0x8000)?1:0)<<','<<q.fx<<','<<q.fy<<","<<q.base<<','<<q.gfxname<<"\n";
        int uw=q.cols*16,uh=128;std::vector<std::uint32_t> un((std::size_t)uw*uh,0),rend((std::size_t)q.zx*q.zy,0),unraw((std::size_t)uw*uh,0),rendraw((std::size_t)q.zx*q.zy,0),alt_swap,alt_gfx; if(options.sprite_export_alternates){alt_swap.assign((std::size_t)uw*uh,0);alt_gfx.assign((std::size_t)uw*uh,0);}
        std::ofstream man(dir/"manifest.txt");man<<"frame="<<frame<<"\nslot="<<q.slot<<"\nraw="<<std::hex<<std::setfill('0')<<std::setw(4)<<q.w[0]<<' '<<std::setw(4)<<q.w[1]<<' '<<std::setw(4)<<q.w[2]<<' '<<std::setw(4)<<q.w[3]<<std::dec<<"\npos="<<q.x<<','<<q.y<<"\nzoom="<<q.zx<<','<<q.zy<<"\ncols="<<q.cols<<" rows=8\npalette="<<(int)q.color<<"\npriority="<<((q.w[1]&0x8000)?1:0)<<"\nflip="<<q.fx<<','<<q.fy<<"\nmap_base_word=0x"<<std::hex<<q.base<<" map_base_byte=0x"<<(q.base*2)<<std::dec<<" gfx="<<q.gfxname<<"\n";
        if(options.sprite_export_analysis){for(int i=0;i<q.cols;++i){int a=i*q.zx/q.cols,b=(i+1)*q.zx/q.cols;zoomcsv<<q.slot<<",x,"<<i<<','<<a<<','<<b<<','<<(b-a)<<','<<q.zx<<','<<q.cols<<','<<(a==b)<<"\n";}for(int i=0;i<8;++i){int a=i*q.zy/8,b=(i+1)*q.zy/8;zoomcsv<<q.slot<<",y,"<<i<<','<<a<<','<<b<<','<<(b-a)<<','<<q.zy<<",8,"<<(a==b)<<"\n";}}
        std::uint16_t prev_code=0xffff; unsigned bad_codes=0,zero_chunks=0,ffff_chunks=0;
        for(int j=0;j<8;++j)for(int k=0;k<q.cols;++k){int mx=q.fx?q.cols-1-k:k,my=q.fy?7-j:j;auto mi=q.base+(std::size_t)(mx+my*q.cols);auto code=machine.spritemap_word(mi);auto masked=static_cast<std::uint16_t>(code&0x3fff);auto sw=static_cast<std::uint16_t>((code>>8)|(code<<8));auto swmasked=static_cast<std::uint16_t>(sw&0x3fff);const auto tile_count=(q.gfx->size()*8)/1024;bool inrange=masked<tile_count,repeat=(code==prev_code);man<<"chunk "<<j<<','<<k<<" map="<<mx<<','<<my<<" word=0x"<<std::hex<<mi<<" byte=0x"<<(mi*2)<<" codeRaw=0x"<<code<<" codeMasked=0x"<<masked<<" codeBEraw=0x"<<sw<<" codeBEmasked=0x"<<swmasked<<" highBits=0x"<<(code&0xc000)<<std::dec<<" masked_in_range="<<inrange<<" repeat_prev="<<repeat<<"\n";if(options.sprite_export_analysis)mapcsv<<q.slot<<','<<j<<','<<k<<","<<mi<<","<<(mi*2)<<","<<code<<","<<masked<<","<<sw<<","<<swmasked<<","<<(code&0xc000)<<','<<q.gfxname<<','<<tile_count<<','<<inrange<<','<<(code==0xffff)<<','<<(code==0)<<','<<repeat<<"\n";prev_code=code;if(code==0xffff)++ffff_chunks;if(!inrange){++bad_codes;continue;}blit_tile(un,uw,uh,k*16,j*16,16,16,*q.gfx,masked,q.fx,q.fy,runtime.bus,q.color);blit_tile_raw_pen(unraw,uw,uh,k*16,j*16,16,16,*q.gfx,masked,q.fx,q.fy);int x0=k*q.zx/q.cols,x1=(k+1)*q.zx/q.cols,y0=j*q.zy/8,y1=(j+1)*q.zy/8;if(x0==x1||y0==y1)++zero_chunks;blit_tile(rend,q.zx,q.zy,x0,y0,x1-x0,y1-y0,*q.gfx,masked,q.fx,q.fy,runtime.bus,q.color);blit_tile_raw_pen(rendraw,q.zx,q.zy,x0,y0,x1-x0,y1-y0,*q.gfx,masked,q.fx,q.fy);if(options.sprite_export_alternates){if(swmasked<tile_count)blit_variant(alt_swap,uw,uh,k*16,j*16,16,16,*q.gfx,swmasked,runtime.bus,q.color);const auto& other=(q.gfxname[0]=='A'?machine.sprites_b():machine.sprites_a());if(masked<((other.size()*8)/1024))blit_variant(alt_gfx,uw,uh,k*16,j*16,16,16,other,masked,runtime.bus,q.color);}if(options.sprite_export_tiles&&exported_tiles.insert({q.gfxname[0],masked}).second){std::vector<std::uint32_t> t(256,0);blit_tile(t,16,16,0,0,16,16,*q.gfx,masked,false,false,runtime.bus,q.color);std::ostringstream n;n<<"tile_"<<q.gfxname<<'_'<<std::hex<<std::setfill('0')<<std::setw(4)<<masked<<".png";save_png_rgba(root/n.str(),16,16,t);}}
        man<<"validation bad_tile_codes="<<bad_codes<<" zero_sized_zoom_chunks="<<zero_chunks<<" ffff_sentinel_chunks="<<ffff_chunks<<"\n";if(options.sprite_export_alternates){save_png_rgba(dir/"alternate_byteswapped_codes.png",uw,uh,alt_swap);save_png_rgba(dir/"alternate_other_gfx_bank.png",uw,uh,alt_gfx);std::vector<std::uint32_t> cmp((std::size_t)uw*3*uh,0);for(int y=0;y<uh;++y)for(int x=0;x<uw;++x){cmp[(std::size_t)y*uw*3+x]=un[(std::size_t)y*uw+x];cmp[(std::size_t)y*uw*3+uw+x]=alt_swap[(std::size_t)y*uw+x];cmp[(std::size_t)y*uw*3+2*uw+x]=alt_gfx[(std::size_t)y*uw+x];}save_png_rgba(dir/"comparison_current_byteswap_otherbank.png",uw*3,uh,cmp);}
        if(options.sprite_export_analysis)summary<<"slot="<<q.slot<<" gfx="<<q.gfxname<<" map_base=0x"<<std::hex<<q.base<<std::dec<<" bad_codes="<<bad_codes<<" zero_zoom_chunks="<<zero_chunks<<" ffff="<<ffff_chunks<<"\n";
        std::vector<std::uint32_t> mask((std::size_t)uw*uh,0);for(std::size_t pi=0;pi<unraw.size();++pi)if((unraw[pi]>>24)!=0)mask[pi]=0xffffffffu;save_png_rgba(dir/"assembled_unscaled.png",uw,uh,un);save_png_rgba(dir/"rendered_zoomed.png",q.zx,q.zy,rend);save_png_rgba(dir/"assembled_raw_pen.png",uw,uh,unraw);save_png_rgba(dir/"rendered_raw_pen.png",q.zx,q.zy,rendraw);save_png_rgba(dir/"mask.png",uw,uh,mask);thumbs.push_back(std::move(un));raw_thumbs.push_back(std::move(unraw));dims.push_back({uw,uh});}
    if(!thumbs.empty()){int cellw=140,cellh=148,cc=4,rr=((int)thumbs.size()+cc-1)/cc;std::vector<std::uint32_t> sheet((std::size_t)cellw*cc*cellh*rr,0),raw_sheet((std::size_t)cellw*cc*cellh*rr,0);for(std::size_t i=0;i<thumbs.size();++i){int ox=(i%cc)*cellw,oy=(i/cc)*cellh;auto [w,h]=dims[i];double sc=std::min(1.0,std::min(128.0/w,128.0/h));int sw=std::max(1,(int)(w*sc)),sh=std::max(1,(int)(h*sc));for(int y=0;y<sh;++y)for(int x=0;x<sw;++x){auto si=(std::size_t)(y*h/sh)*w+(x*w/sw);auto c=thumbs[i][si];if(c)sheet[(std::size_t)(oy+y)*cellw*cc+ox+x]=c;auto rc=raw_thumbs[i][si];if(rc)raw_sheet[(std::size_t)(oy+y)*cellw*cc+ox+x]=rc;}}save_png_rgba(root/"contact_sheet.png",cellw*cc,cellh*rr,sheet);save_png_rgba(root/"contact_sheet_raw_pen.png",cellw*cc,cellh*rr,raw_sheet);}
    std::ofstream readme(root/"README.txt");readme<<"Chase H.Q. Native sprite export frame "<<frame<<"\nassembled_unscaled.png = spritemap reconstruction before screen zoom.\nrendered_zoomed.png = same logical sprite after current zoom/chunk distribution.\nsprites.csv + manifest.txt preserve raw RAM and mapping metadata.\nTransparent PNG background; palette sampled from runtime TC0110PCR RAM.\nassembled_raw_pen.png / rendered_raw_pen.png use pen-index greyscale independent of TC0110PCR so zero/black runtime palettes cannot hide valid graphics. mask.png is an exact binary opaque-pixel silhouette derived from the raw-pen reconstruction. contact_sheet_raw_pen.png provides the same diagnostic view across all exported slots.\nIf unscaled is wrong: investigate ROM/spritemap/bank/chunk order. If unscaled is right but zoomed wrong: investigate zoom/flip/chunk placement. If both are right but screen is wrong: investigate clipping/priority/mixer/position.\nanalysis mode adds map_entries.csv, zoom_boundaries.csv and analysis.txt. alternate mode adds byte-swapped-code and opposite-gfx-bank comparison PNGs. atlas mode adds complete OBJ-A/OBJ-B tile atlases.\n";std::cout<<"[SPRITE EXPORT] frame="<<frame<<" wrote "<<root.string()<<" sprites="<<all.size()<<"\n";
}

static void dump_sprite_diagnostics(const chq::Machine& machine, const chq::Runtime& runtime,
                                    const chq::Options& options, unsigned frame) {
    const auto* sr = find_region_named(runtime.bus, "sprites");
    if (!sr) return;
    std::filesystem::create_directories(options.logs);
    const auto path = options.logs / ("sprites_frame_" + std::to_string(frame) + ".log");
    std::ofstream out(path);
    if (!out) return;
    out << "# Chase H.Q. Native v" << chq::kNativeVersion << " sprite decode snapshot\n# frame=" << frame
        << " sprite_ram_base=d00000 bytes=" << sr->bytes.size() << "\n";
    out << "# Logs BOTH spritemap LE (current renderer) and byte-swapped BE candidate for diagnosis.\n";
    out << "# slot raw[w0 w1 w2 w3] x y zoomx zoomy format tile color pri flip map_base chunks\n";
    unsigned active=0, invalid=0, chunks_total=0, f128=0, f64=0, f32=0;
    for (std::size_t off=0; off+7<sr->bytes.size(); off+=8) {
        unsigned slot=static_cast<unsigned>(off/8);
        auto w0=be16_at(sr->bytes,off), w1=be16_at(sr->bytes,off+2), w2=be16_at(sr->bytes,off+4), w3=be16_at(sr->bytes,off+6);
        int y=w0&0x1ff; if((w3&0x7ff)==0) continue;
        int zy=((w0>>9)&0x7f)+1, zx=(w1&0x7f)+1, x=w2&0x1ff; if(x>0x140)x-=0x200;
        y+=3; if(y>0x140)y-=0x200;
        auto fmt=chq::Machine::format_for_zoomx(zx);
        if(fmt==chq::SpriteFormat::Invalid) { ++invalid; if(options.sprite_debug_index<0 || options.sprite_debug_index==(int)slot) out<<"INVALID slot="<<slot<<" raw="<<std::hex<<w0<<' '<<w1<<' '<<w2<<' '<<w3<<std::dec<<" zoomx="<<zx<<"\n"; continue; }
        int cols=0; std::size_t base=0; const char* region="";
        if(fmt==chq::SpriteFormat::Obj128x128){cols=8;base=(std::size_t)(w3&0x7ff)<<6;region="A";++f128;}
        else if(fmt==chq::SpriteFormat::Obj64x128){cols=4;base=((std::size_t)(w3&0x7ff)<<5)+0x20000;region="B";++f64;}
        else {cols=2;base=((std::size_t)(w3&0x7ff)<<4)+0x30000;region="B";++f32;}
        ++active;
        if(options.sprite_debug_large_only && cols<4) continue;
        if(options.sprite_debug_index>=0 && options.sprite_debug_index!=(int)slot) continue;
        out << "SPR slot="<<slot<<" raw="<<std::hex<<std::setfill('0')<<std::setw(4)<<w0<<' '<<std::setw(4)<<w1<<' '<<std::setw(4)<<w2<<' '<<std::setw(4)<<w3<<std::dec
            <<" x="<<x<<" y="<<y<<" zx="<<zx<<" zy="<<zy<<" fmt="<<chq::Machine::format_name(fmt)
            <<" tile="<<(w3&0x7ff)<<" color="<<((w1&0x7f80)>>7)<<" pri="<<((w1&0x8000)?1:0)
            <<" flipx="<<((w2&0x4000)?1:0)<<" flipy="<<((w2&0x8000)?1:0)
            <<" map_base=0x"<<std::hex<<base<<std::dec<<" gfx="<<region<<" cols="<<cols<<" rows=8\n";
        int by=y+(128-zy); unsigned valid=0, empty=0;
        for(int j=0;j<8;++j) for(int k=0;k<cols;++k) {
            int mx=(w2&0x4000)?cols-1-k:k, my=(w2&0x8000)?7-j:j;
            std::size_t mi=base+(std::size_t)(mx+my*cols); auto le=machine.spritemap_word(mi); auto sw=static_cast<std::uint16_t>((le>>8)|(le<<8));
            int cx=x+(k*zx)/cols, cy=by+(j*zy)/8, nx=x+((k+1)*zx)/cols, ny=by+((j+1)*zy)/8;
            if(le==0xffff) ++empty; else ++valid;
            out << "  CHUNK j="<<j<<" k="<<k<<" map_xy="<<mx<<','<<my<<" map_word=0x"<<std::hex<<mi
                <<" codeLE=0x"<<std::setw(4)<<le<<" codeBE=0x"<<std::setw(4)<<sw<<std::dec
                <<" dst="<<cx<<','<<cy<<" size="<<(nx-cx)<<'x'<<(ny-cy)
                <<" onscreen="<<((nx>0&&cx<320&&ny>0&&cy<240)?1:0)<<"\n";
        }
        chunks_total += valid;
        out << "  SUMMARY valid_chunks="<<valid<<" empty_chunks="<<empty<<"\n";
    }
    out << "# TOTAL active="<<active<<" invalid="<<invalid<<" fmt128="<<f128<<" fmt64="<<f64<<" fmt32="<<f32<<" logged_valid_chunks="<<chunks_total<<"\n";
    if(options.sprite_dump_ram) {
        auto bin=options.logs/("sprite_ram_frame_"+std::to_string(frame)+".bin"); std::ofstream b(bin,std::ios::binary); b.write((const char*)sr->bytes.data(),(std::streamsize)sr->bytes.size());
    }
    std::cout << "[SPRITE DEBUG] frame="<<frame<<" wrote "<<path.string()<<" active="<<active<<" invalid="<<invalid<<" formats="<<f128<<'/'<<f64<<'/'<<f32<<"\n";
}


static std::string json_escape(const std::string& s) {
    std::ostringstream o; for(char c:s){ switch(c){case '\\':o<<"\\\\";break;case '"':o<<"\\\"";break;case '\n':o<<"\\n";break;case '\r':o<<"\\r";break;case '\t':o<<"\\t";break;default:o<<c;}} return o.str();
}
static const char* mixer_name(chq::Options::MixerChoice m){ return m==chq::Options::MixerChoice::Reference?"reference":m==chq::Options::MixerChoice::Prom?"prom":"legacy"; }
static const char* diagnostic_background_name(chq::Options::DiagnosticBackground b){
    switch(b){case chq::Options::DiagnosticBackground::Black:return "black";case chq::Options::DiagnosticBackground::White:return "white";case chq::Options::DiagnosticBackground::Checkerboard:return "checkerboard";default:return "none";}
}
static void write_int_array(std::ostream& f,const std::vector<int>& v){ f<<'['; for(size_t i=0;i<v.size();++i){if(i)f<<',';f<<v[i];} f<<']'; }
static void write_selector_json(std::ostream& f,const chq::Options::SpriteSelector& s){
    f<<"{\"slots\":";write_int_array(f,s.slots);f<<",\"maps\":";write_int_array(f,s.maps);f<<",\"palettes\":";write_int_array(f,s.palettes);f<<",\"priorities\":";write_int_array(f,s.priorities);f<<'}';
}
static void write_quad_json(std::ostream& f,const chq::Options::SpriteQuadCandidate& q){
    f<<"{\"name\":\""<<json_escape(q.name)<<"\",\"tl\":"<<q.tl<<",\"tr\":"<<q.tr<<",\"bl\":"<<q.bl<<",\"br\":"<<q.br<<",\"palette\":"<<q.palette<<",\"priority\":"<<q.priority<<'}';
}
static void write_run_manifest(const chq::Options& o,int argc,char** argv,unsigned effective_frames){
    if(!o.write_run_manifest)return; std::filesystem::create_directories(o.logs); std::ofstream f(o.logs/"run_manifest.json"); if(!f)return;
    f<<"{\n  \"format_version\": 5,\n  \"build\": \""<<chq::kNativeVersion<<"\",\n  \"scenario\": \""<<json_escape(o.scenario)<<"\",\n  \"frames_requested\": "<<o.frames<<",\n  \"frames_effective\": "<<effective_frames<<",\n  \"mixer\": \""<<mixer_name(o.mixer)<<"\",\n  \"roms\": \""<<json_escape(o.roms.string())<<"\",\n  \"debug_everything\": "<<(o.debug_everything?"true":"false")<<",\n  \"input_trace\": "<<(o.input_trace?"true":"false")<<",\n  \"fast_forward_to\": "<<o.fast_forward_to<<",\n  \"investigate_profile\": \""<<json_escape(o.investigate_profile)<<"\",\n  \"memory_watch_count\": "<<o.memory_watches.size()<<",\n  \"sprite_mask_matrix_count\": "<<o.graphics_trace.experiment_sprite_mask_matrix.size()<<",\n  \"provenance_enabled\": "<<(o.provenance.enabled()?"true":"false")<<",\n  \"provenance_from_frame\": "<<o.provenance.from_frame<<",\n  \"provenance_to_frame\": "<<o.provenance.to_frame<<",\n  \"argv\": [";
    for(int i=0;i<argc;++i){if(i)f<<',';f<<"\""<<json_escape(argv[i])<<"\"";}
    f<<"],\n  \"capture_frames\": [";for(size_t i=0;i<o.capture_frames.size();++i){if(i)f<<',';f<<o.capture_frames[i];}
    f<<"],\n  \"checkpoint\": {\"load\":\""<<json_escape(o.load_checkpoint.string())<<"\",\"dir\":\""<<json_escape(o.checkpoint_dir.string())<<"\",\"slot\":"<<o.checkpoint_slot<<"},";
    f<<"\n  \"sprite_evidence\": {\"every\":"<<o.sprite_evidence_every<<",\"from\":"<<o.sprite_evidence_from<<",\"to\":"<<o.sprite_evidence_to<<"},";
    f<<"\n  \"diagnostic_background\": \""<<diagnostic_background_name(o.diagnostic_background)<<"\",";
    f<<"\n  \"sprite_solo\": [";for(size_t i=0;i<o.sprite_solo.size();++i){if(i)f<<',';write_selector_json(f,o.sprite_solo[i]);}
    f<<"],\n  \"sprite_hide\": [";for(size_t i=0;i<o.sprite_hide.size();++i){if(i)f<<',';write_selector_json(f,o.sprite_hide[i]);}
    f<<"],\n  \"sprite_quad_candidates\": [";for(size_t i=0;i<o.sprite_quad_candidates.size();++i){if(i)f<<',';write_quad_json(f,o.sprite_quad_candidates[i]);}
    f<<"],\n  \"object_evidence\": [";for(size_t i=0;i<o.object_evidence.size();++i){if(i)f<<',';f<<"\""<<json_escape(o.object_evidence[i])<<"\"";}
    f<<"],\n  \"object_track\": [";for(size_t i=0;i<o.object_track.size();++i){if(i)f<<',';f<<"\""<<json_escape(o.object_track[i])<<"\"";}
    f<<"],\n  \"layer_offsets\": {\"bg0\":["<<o.layer_offset_bg0.x<<','<<o.layer_offset_bg0.y<<"],\"bg1\":["<<o.layer_offset_bg1.x<<','<<o.layer_offset_bg1.y<<"],\"text\":["<<o.layer_offset_text.x<<','<<o.layer_offset_text.y<<"],\"sprites\":["<<o.layer_offset_sprites.x<<','<<o.layer_offset_sprites.y<<"],\"road\":["<<o.layer_offset_road.x<<','<<o.layer_offset_road.y<<"]},";
    f<<"\n  \"sprite_tie_break\": \""<<(o.sprite_tie_break==chq::Options::SpriteTieBreak::HigherSlot?"higher-slot":"lower-slot")<<"\",";
    f<<"\n  \"event_traces\": [";for(size_t i=0;i<o.event_trace.events.size();++i){if(i)f<<',';f<<"\""<<json_escape(o.event_trace.events[i])<<"\"";}f<<"]\n}\n";
    f.flush(); f.close();
    std::error_code ec; std::filesystem::copy_file(o.logs/"run_manifest.json", o.logs/"run_metadata.json", std::filesystem::copy_options::overwrite_existing, ec);
}
static void capture_hardware_state(const chq::Runtime& runtime,const chq::Options& o,unsigned frame){
    auto root=o.logs/("capture_frame_"+std::to_string(frame));std::filesystem::create_directories(root);std::ofstream meta(root/"capture_manifest.txt");meta<<"Chase H.Q. Native v"<<chq::kNativeVersion<<" hardware-state capture\nframe="<<frame<<"\nscenario="<<o.scenario<<"\nNOTE: renderer/device-state evidence capture, not yet a resumable CPU save-state.\n";
    for(const auto&r:runtime.bus.regions){if(r.readonly)continue;std::string name=r.name?r.name:"region";std::ofstream out(root/(name+".bin"),std::ios::binary);out.write((const char*)r.bytes.data(),(std::streamsize)r.bytes.size());meta<<name<<" base=0x"<<std::hex<<r.base<<std::dec<<" size="<<r.bytes.size()<<" reads="<<r.reads<<" writes="<<r.writes<<"\n";}
    std::ofstream pal(root/"palette_u16le.bin",std::ios::binary);for(auto v:runtime.bus.palette){unsigned char b[2]={(unsigned char)(v&255),(unsigned char)(v>>8)};pal.write((char*)b,2);}std::ofstream sum(root/"runtime_summary.txt");runtime.summary(sum);
}
static void print_scenario_list(){std::cout<<"Built-in scenarios:\n  boot\n  stage1-gameplay\n  stage1-driving\n  service-mode\n  crash-test\n";}
static void print_scenario_description(const std::string& n){
    if(n=="stage1-gameplay"||n=="stage1-driving")std::cout<<n<<": proven credit/start/script IOC pulse sequence through Stage 1 gameplay; minimum 1800 frames. stage1-driving currently aliases gameplay until native controls are wired.\n";
    else if(n=="boot")std::cout<<"boot: IRQ4 normal boot without injected controls; minimum 600 frames.\n";
    else if(n=="service-mode")std::cout<<"service-mode: reserved reproducible service-state scenario; currently avoids undocumented input injection and runs 900 frames.\n";
    else if(n=="crash-test")std::cout<<"crash-test: Stage 1 gameplay setup with extended 2100-frame observation window; collision injection is reserved for future control work.\n";
    else std::cout<<"Unknown scenario: "<<n<<"\n";
}

int main(int argc, char* argv[]) {
    std::filesystem::path emergency_logs, emergency_zip;
    bool emergency_bundle = false;
    try {
        const auto process_start = std::chrono::steady_clock::now();
        auto options = chq::parse_options(argc, argv);
        std::vector<DriverProfilePoint> course_profile;
        if(!options.course_profile_file.empty()) { course_profile=load_driver_profile(options.course_profile_file); std::cout<<"[COURSE PROFILE] loaded "<<course_profile.size()<<" rows from "<<options.course_profile_file.string()<<"\n"; }
        if (options.help) { chq::print_help(); return 0; }
        if (!options.batch_file.empty()) return run_batch_jobs(options.batch_file, argv[0]);
        if (options.scenario_list) { print_scenario_list(); return 0; }
        if (!options.scenario_describe.empty()) { print_scenario_description(options.scenario_describe); return 0; }

        // v0.49.1: every evidence-bundled run gets an isolated directory by default.
        // A friendly evidence name produces one attachable ZIP without stale files.
        if (options.auto_zip_logs && options.isolated_log_run && !options.logs_explicit) {
            const auto stamp = timestamp_name();
            const auto base = options.evidence_name.empty() ? std::string("run_")+stamp : safe_name(options.evidence_name)+"_"+stamp;
            options.logs = std::filesystem::path("evidence") / base;
            if (options.zip_name.empty()) options.zip_name = (std::filesystem::path("evidence") / (base+".zip")).string();
        } else if (options.auto_zip_logs && !options.evidence_name.empty() && options.zip_name.empty()) {
            options.zip_name = (options.logs.parent_path() / (safe_name(options.evidence_name)+".zip")).string();
        }

        emergency_logs = options.logs;
        emergency_bundle = options.auto_zip_logs;
        emergency_zip = options.zip_name.empty() ? default_bundle_name(options) : std::filesystem::path(options.zip_name);
        if (emergency_zip.extension() != ".zip") emergency_zip += ".zip";

        std::cout << console_stamp(process_start) << chq::kWindowProduct << " " << chq::kNativeVersion << " - " << chq::kNativePlatform << "\n";

        std::unique_ptr<chq::Runtime> runtime;
        unsigned loaded_checkpoint_frame = 0;
        bool checkpoint_loaded = false;
        if (!options.scene_only) {
            runtime = std::make_unique<chq::Runtime>(
                chq::load_main_rom(options.roms),
                chq::load_sub_rom(options.roms));
            std::filesystem::create_directories(options.logs);
            runtime->bus.open_log(options.logs / "bus.log", 20000, options.all);
            runtime->enable_boot_debug(options.boot_debug, options.loop_samples);
            if (options.boot_debug)
                runtime->open_boot_debug_log(options.logs / "boot_loop.log", 20000);
            runtime->enable_sub_debug(options.sub_debug, options.sub_samples);
            if (options.sub_debug)
                runtime->open_sub_debug_log(options.logs / "cpu_b_loop.log", 40000);
            if (options.handshake_debug)
                runtime->open_handshake_log(options.logs / "handshake.log", 60000);
            runtime->enable_road_focus_debug(options.road_focus_debug, options.road_samples);
            if (options.road_focus_debug)
                runtime->open_road_focus_log(options.logs / "road_write_focus.log", 80000);
            if (options.road_flow_debug)
                runtime->open_road_data_flow_log(options.logs / "road_data_flow.log", options.flow_samples);
            runtime->enable_road_state_debug(options.road_state_debug, options.state_samples);
            runtime->set_force_road_flag(options.force_road_flag);
            runtime->bus.set_suppress_collision_shove(options.no_collisions);
            runtime->configure_handling_override(options.cornering_scale, options.cornering_speed_retain);
            runtime->set_target_one_hit(options.target_one_hit);
            if (options.road_state_debug)
                runtime->open_road_state_log(options.logs / "road_state.log", options.state_samples);
            if (options.road_pending_debug)
                runtime->bus.open_road_pending_log(options.logs / "road_pending.log", options.pending_samples);
            if (options.trace.enabled())
                runtime->configure_generic_trace(options.trace, options.logs / "trace.log");
            if (options.provenance.enabled())
                runtime->configure_provenance(options.provenance, options.logs);
            runtime->reset();
            // v0.49.4: CLI checkpoint restore remains after machine/video frontend initialisation;
            // Runtime::load_checkpoint now also rebinds Musashi host-process pointers so states
            // saved in an earlier process are safe to resume after ASLR relocation.
            if (!options.break_frames.empty()) runtime->configure_forensic_history(options.break_history);
        }

        chq::Machine machine;
        if (!machine.load_roms(options.roms)) return 1;

        chq::Video video;
        video.configure_proms(machine);
        if (!video.initialise(options.logs)) return 1;
        std::vector<unsigned> graphics_frames = options.graphics_debug_frames;
        for (auto f : options.break_frames) if (std::find(graphics_frames.begin(), graphics_frames.end(), f) == graphics_frames.end()) graphics_frames.push_back(f);
        if (graphics_frames.empty() && options.graphics_debug_all) {
            if (!options.sprite_debug_frames.empty()) graphics_frames = options.sprite_debug_frames;
            else graphics_frames.push_back(options.frames);
        }
        if (options.graphics_trace.enabled() && graphics_frames.empty()) {
            const unsigned capture = options.graphics_trace.to_frame != 0xffffffffu ? options.graphics_trace.to_frame : options.frames;
            graphics_frames.push_back(capture);
        }
        video.configure_diagnostics(graphics_frames, options.graphics_debug_all || options.graphics_trace.enabled() || !options.break_frames.empty(), options.graphics_export_raw_maps);
        video.configure_forensics(options.graphics_trace);
        video.configure_layer_offsets(options);
        video.configure_sprite_tie_break(options);
        video.configure_tc0100scn_trace(options);
        video.configure_sprite_presentation(options.sprite_solo, options.sprite_hide, options.sprite_quad_candidates, options.diagnostic_background);

        auto write_run_phase = [&](const std::string& phase, unsigned phase_frame) {
            std::filesystem::create_directories(options.logs);
            std::ofstream pf(options.logs / "run_phase.txt", std::ios::trunc);
            if (pf) { pf << "build=" << chq::kNativeVersion << "\nphase=" << phase << "\nframe=" << phase_frame << "\n"; pf.flush(); }
        };
        write_run_phase("frontend_initialised", 0);
        if (runtime && !options.load_checkpoint.empty()) {
            write_run_phase("checkpoint_load_begin", 0);
            loaded_checkpoint_frame = runtime->load_checkpoint(options.load_checkpoint);
            checkpoint_loaded = true;
            write_run_phase("checkpoint_load_complete", loaded_checkpoint_frame);
            std::cout << console_stamp(process_start) << "[CHECKPOINT LOAD] frame=" << loaded_checkpoint_frame << " <- " << options.load_checkpoint.string() << "\n";
            std::cout << console_stamp(process_start) << "[CHECKPOINT REPAIR] Musashi cycle-table/callback pointers rebound for this process\n";
        }

        std::cout << "v" << chq::kNativeVersion << " " << chq::kNativePlatform << ":\n"
                     "  R = toggle live road rendering\n"
                     "  T = toggle live sprite RAM view\n"
                     "  Y = toggle TC0100SCN background layers\n"
                     "  U = toggle TC0100SCN text layer\n"
                     "  I = show per-pixel layer-mask view\n"
                     "  O = show PROM address/output trace view\n"
                     "  P = cycle PROM-MASK / LEGACY / REFERENCE mixer\n"
                     "  V = TC0150ROD raw-signal view\n"
                     "  H = TC0150ROD RAM activity heatmap\n"
                     "  [ / ] = select raw RAM scanline shown in title\n"
                     "  F1 = interactive debug overlay\n"
                     "  F11 / Shift+F11 = next / previous debug page\n"
                     "  F12 = live Research Workbench (auto-pauses + snapshots baseline)\n"
                     "  Esc = quit (or close Workbench while it has focus)\n"
                     "v0.60.0: reversible live RAM interventions, watched writes, frame/instruction stepping, pause-point rewind and sprite-change provenance.\n"
                     "Layout results are written to logs/road_ram_layout.log and logs/road_ram_final.bin.\n";

        chq::SceneState scene;
        if (options.mixer == chq::Options::MixerChoice::Reference) { scene.use_reference_mixer = true; scene.use_prom_mixer = false; }
        else if (options.mixer == chq::Options::MixerChoice::Prom) { scene.use_reference_mixer = false; scene.use_prom_mixer = true; }
        else { scene.use_reference_mixer = false; scene.use_prom_mixer = false; }
        bool running = true, changed = true, saved = false;
        unsigned frame = checkpoint_loaded ? loaded_checkpoint_frame : 0;
        unsigned effective_frames = (options.frames_explicit || !options.scenario.empty()) ? options.frames : 0;
        for (auto f : options.sprite_debug_frames) effective_frames = std::max(effective_frames, f + 1);
        for (auto f : graphics_frames) effective_frames = std::max(effective_frames, f + 1);
        for (auto f : options.capture_frames) effective_frames = std::max(effective_frames, f + 1);
        for (auto f : options.break_frames) effective_frames = std::max(effective_frames, f + 1);
        for (const auto& q : options.save_checkpoints) effective_frames = std::max(effective_frames, q.frame + 1);
        const bool bounded_run = effective_frames != 0;
        bool debug_paused = false;
        unsigned debug_step_remaining = 0;
        bool timer_frozen = false;
        std::uint8_t frozen_timer_value = 0;
        std::uint8_t frozen_timer_fraction = 0;
        std::filesystem::create_directories(options.checkpoint_dir);
        video.set_checkpoint_slot(options.checkpoint_slot);
        video.set_course_survey_assists(options.course_follow, options.infinite_time, options.unlimited_turbo, options.auto_turbo);
        auto ui_state_path = [&]() { return options.checkpoint_dir / (std::string("slot_") + std::to_string(video.checkpoint_slot()) + ".chqstate"); };
        const auto workbench_dir = options.logs / "workbench";
        const auto workbench_pause_point = workbench_dir / "pause_baseline.chqstate";
        bool workbench_have_pause_point = false;
        std::map<std::string,std::filesystem::path> named_snapshots;
        const unsigned run_start_frame = frame;
        const std::uint8_t survey_timer_bcd = runtime ? static_cast<std::uint8_t>(runtime->bus.peek(0x100200,1,chq::BusSpace::Main)) : 0x59;
        int course_follow_target = 0;
        int course_follow_curve_now = 0, course_follow_curve_ahead = 0, course_follow_curve_predict = 0;
        int course_follow_feedforward = 0, course_follow_lateral_correction = 0;
        int course_follow_p_term = 0, course_follow_d_term = 0, course_follow_error_rate = 0;
        int course_follow_speed_target = 390;
        bool course_follow_accel_cmd = true, course_follow_brake_cmd = false;
        bool course_follow_have_prev_error = false;
        std::int16_t course_follow_prev_error = 0;
        int course_follow_output = 0;
        int course_follow_profile_feedforward = 0;
        std::int64_t course_follow_profile_delta = 0;
        std::uint16_t road_left_bound=0, road_right_bound=0, road_centre=0, car_lateral=0;
        int road_width=0; double lateral_error_norm=0.0;
        std::int16_t lateral_error=0;
        const char* lateral_side="CENTRE";
        const char* course_follow_mode = "IDLE";
        bool excursion_active=false; unsigned excursion_id=0, excursion_start_frame=0; std::uint32_t excursion_start_cp=0; unsigned excursion_start_bank=0, excursion_start_record=0; int excursion_start_lateral_error=0, excursion_max_abs_lateral_error=0;
        std::ofstream excursion_log;
        if(runtime && options.course_survey){ excursion_log.open(options.logs/"offroad_excursions.csv"); excursion_log<<"excursion,start_frame,end_frame,duration,start_course_position,end_course_position,start_bank,start_record,end_bank,end_record,max_speed_kmh,start_lateral_error,max_abs_lateral_error\n"; }
        std::ofstream pursuit_probe, pursuit_events; std::uint32_t last_pursuit_score=0xffffffffu;
        if(runtime && options.course_survey){ pursuit_probe.open(options.logs/"pursuit_probe.csv"); pursuit_probe<<"frame,course_position,bank,record,score_bcd,palette_crc32\n"; pursuit_events.open(options.logs/"pursuit_events.csv"); pursuit_events<<"frame,event,old_value,new_value,course_position,bank,record,palette_crc32\n"; }
        int excursion_max_speed=0; unsigned interior_frames=0, edge_frames=0, offroad_frames=0;
        const auto progress_start = std::chrono::steady_clock::now();
        std::cout << console_stamp(process_start) << "[RUN MODE] " << (bounded_run ? (std::string("bounded to absolute frame ") + std::to_string(effective_frames)) : "continuous (F1 debug UI, Esc quits)") << "\n";
        if (bounded_run) {
            const unsigned remaining = frame < effective_frames ? (effective_frames - frame) : 0;
            std::cout << console_stamp(process_start) << "[RUN TARGET] start=" << frame << " target=" << effective_frames << " remaining=" << remaining << "\n";
            if (frame >= effective_frames) {
                std::cout << console_stamp(process_start) << "[RUN TARGET] checkpoint is already at/past target; finalising without emulation\n";
                running = false;
            }
        }
        write_run_manifest(options, argc, argv, effective_frames);
        write_run_phase("run_loop_ready", frame);
        std::ofstream input_trace;
        if (runtime && options.input_trace) { input_trace.open(options.logs / "input_trace.csv"); input_trace << "frame,port02,port03,steer_lo,steer_hi,reads02,reads03,reads0c,reads0d\n"; }
        std::ofstream gameplay_state;
        if(runtime && options.gameplay_state_log){ gameplay_state.open(options.logs/"gameplay_state.csv"); gameplay_state<<"frame,speed_bcd,speed_kmh,speed_internal,distance_raw,distance,timer_bcd,score_bcd,steering_injected_raw,steering_injected_signed,steering_game_raw12,steering_game_signed,steering_processed_signed,autopilot_target_signed,autopilot_curve_now,autopilot_curve_ahead,autopilot_curve_predict,autopilot_feedforward,autopilot_p_term,autopilot_d_term,autopilot_error_rate,autopilot_lateral_correction,autopilot_speed_target,autopilot_accel_cmd,autopilot_brake_cmd,autopilot_controller,autopilot_mode,road_left_bound,road_centre,road_right_bound,road_width,car_lateral,lateral_error,lateral_error_normalized,lateral_side,accel,brake,turbo_active,turbos_left,brake_lamp,brake_lamp_pal64_pen13,brake_lamp_pal65_pen13,road_state,road_zone,road_flags,grade_state,grade_profile_signed\n"; }
        std::ofstream handling_state, driver_profile;
        if(runtime && options.course_survey){
            handling_state.open(options.logs/"handling_state.csv");
            handling_state<<"frame,turn_state,table_index,speed_internal,forward_coeff,forward_coeff_ratio,lateral_coeff,lateral_coeff_ratio,applied_forward_coeff,applied_forward_ratio,applied_lateral_coeff,applied_lateral_ratio,forward_component,lateral_component,override_active,samples\n";
            driver_profile.open(options.logs/"driver_profile.csv");
            driver_profile<<"frame,course_position,bank,record,curve_now,curve_predict,road_left,road_centre,road_right,road_width,car_lateral,lateral_error,lateral_error_normalized,steering_target,accel_cmd,brake_cmd,speed_target,controller,mode,profile_feedforward,profile_position_delta\n";
        }
        std::ofstream target_state, collision_events;
        bool target_contact_known=false, last_target_contact=false; std::uint16_t last_collision_timer=0; std::uint64_t last_suppressed_shoves=0, last_suppressed_lateral=0, last_suppressed_speed=0;
        if(runtime && options.target_state_log){
            target_state.open(options.logs/"target_state.csv");
            target_state<<"frame,player_longitudinal,target_longitudinal,separation_signed,target_status,target_contact_candidate,target_speed_10a092,target_motion_control_10a096,defeat_flags_10018d,defeat_mode_candidate,defeat_timer_1002d0,interaction_type,interaction_timer,interaction_counter,suppressed_collision_responses,suppressed_lateral,suppressed_speed,stage1_sprite_maps,stage1_sprite_palette\n";
            collision_events.open(options.logs/"collision_events.csv");
            collision_events<<"frame,event,target_status,interaction_type,interaction_timer,interaction_counter,separation_signed,suppressed_collision_responses,suppressed_lateral,suppressed_speed\n";
        }
        std::ofstream course_state;
        if(runtime && options.course_data_log){
            std::ofstream raw(options.logs/"course_raw.csv");
            raw<<"bank,record,address,ch0_curve_signed,ch0_raw,ch1_raw,ch2_raw,ch3_raw,ch4_raw,ch5_raw,ch6_raw,ch7_raw\n";
            for(unsigned bank=0;bank<16;++bank) for(unsigned rec=0;rec<32;++rec){
                const auto a=0x109000u+bank*0x100u+rec*8u; raw<<bank<<','<<rec<<",0x"<<std::hex<<a<<std::dec;
                const auto c0=static_cast<std::uint8_t>(runtime->bus.peek(a,1,chq::BusSpace::Sub)); raw<<','<<int(static_cast<std::int8_t>(c0));
                for(unsigned c=0;c<8;++c) raw<<",0x"<<std::hex<<std::setfill('0')<<std::setw(2)<<runtime->bus.peek(a+c,1,chq::BusSpace::Sub)<<std::dec; raw<<'\n';
            }
            std::ofstream links(options.logs/"course_banks.csv"); links<<"bank,next_bank,selector_rom_address,selector_raw\n";
            for(unsigned bank=0;bank<16;++bank){const auto v=runtime->bus.peek(0x00f5a4u+bank,1,chq::BusSpace::Sub); links<<bank<<','<<(v&0x0f)<<",0x"<<std::hex<<(0x00f5a4u+bank)<<",0x"<<v<<std::dec<<'\n';}
            course_state.open(options.logs/"course_state.csv"); course_state<<"frame,course_position,bank,record,record_address,ch0_curve_signed,ch0_raw,ch1_profile_signed,ch1_raw,ch3_raw,road_flags,road_zone\n";
        }

        // Keep debugger input injection identical between ChaseHQRuntime and
        // the SDL frontend.  Earlier frontends parsed --pulse-ioc but never
        // applied the resulting IOC XOR masks, so the CPUs advanced while the
        // visible game remained at the RAM test.
        struct PulseState {
            bool armed = false;
            bool observed = false;
            unsigned active_from = 0;
            std::uint64_t reads_before = 0;
        };
        std::vector<PulseState> pulse_state(options.input_pulses.size());
        std::array<std::uint8_t, 16> active_ioc_xor{};
        if (runtime && (!options.input_pulses.empty() || !options.periodic_input_pulses.empty()))
            runtime->bus.open_ioc_pulse_log(options.logs / "ioc_pulse.log");
        std::ofstream patch_log;
        if (runtime && !options.memory_patches.empty()) {
            patch_log.open(options.logs / "memory_patches.csv");
            patch_log << "frame,cpu,width,address,old_value,new_value,kind\n";
        }
        std::ofstream watch_log;
        if (runtime && !options.memory_watches.empty()) {
            watch_log.open(options.logs / "memory_watch.csv");
            watch_log << "frame,cpu,width,address,label,old_value,new_value\n";
        }
        std::ofstream timeline_log;
        if (runtime && (!options.transition_triggers.empty() || !options.memory_patches.empty() || !options.save_checkpoints.empty())) {
            timeline_log.open(options.logs / "event_timeline.csv");
            timeline_log << "frame,event,detail\n";
        }
        std::set<unsigned> scheduled_capture_frames(options.capture_frames.begin(), options.capture_frames.end());
        std::set<unsigned> scheduled_screenshot_frames(options.screenshot_at.begin(), options.screenshot_at.end());
        unsigned screenshots_written = 0;
        std::vector<std::filesystem::path> captured_screenshots;
        bool patch_relative_scheduled = false;
        if(runtime && !options.load_checkpoint.empty()) {
            video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
            if(scheduled_screenshot_frames.erase(frame) && screenshots_written < options.screenshot_limit) {
                const auto dir=options.screenshot_dir.empty()?options.logs/"screenshots":options.screenshot_dir;
                std::ostringstream fn; fn<<"frame_"<<std::setfill('0')<<std::setw(8)<<frame<<".png";
                if(!video.save_current_screenshot(dir/fn.str())) throw std::runtime_error("Initial checkpoint screenshot failed");
                captured_screenshots.push_back(dir/fn.str());
                ++screenshots_written;
            }
        }


        auto save = [&] {
            if (!runtime || saved) return;
            runtime->summary(std::cout);
            std::ofstream summary(options.logs / "summary.txt");
            runtime->summary(summary);
            if (!summary) throw std::runtime_error("Cannot write summary");
            runtime->bus.dump(options.logs / "ram");
            runtime->write_provenance_reports();
            saved = true;
        };

        chq::TraceConfig api_legacy_trace = options.trace;
        std::filesystem::path api_legacy_trace_path = options.logs / "trace_live.log";
        chq::ProvenanceConfig api_provenance = options.provenance;
        std::filesystem::path api_provenance_dir = options.logs / "provenance_live";
        // v0.61.0 deterministic live IOC pulses.  Pulse expiry is keyed to
        // emulated frames, never wall-clock time, so experiments remain reproducible.
        struct LiveIocPulse { std::uint8_t port=0, mask=0, prior=0; unsigned end_frame=0; };
        std::vector<LiveIocPulse> live_ioc_pulses;
        // v0.62.0 research-only palette freezes. These are renderer-visible TC0110PCR
        // entries reapplied at the frame boundary; clear restores authentic game control.
        std::map<std::uint16_t,std::uint16_t> live_palette_overrides;
        std::map<std::uint16_t,std::uint16_t> live_palette_originals;
        // v0.66.8.0 Forensic Timeline v1. A recording is a directory of
        // deterministic per-frame CHQSTATE checkpoints plus an exact bus-write stream.
        // The deliberately simple v1 storage prioritises fidelity and offline seekability;
        // later revisions can delta-compress without changing the script/API contract.
        bool forensic_timeline_recording=false, forensic_timeline_loaded=false;
        std::filesystem::path forensic_timeline_root, forensic_timeline_return_state;
        std::vector<unsigned> forensic_timeline_frames;
        std::size_t forensic_timeline_index=0;
        std::ofstream forensic_timeline_index_stream, forensic_timeline_write_stream;
        unsigned forensic_timeline_start_frame=0, forensic_timeline_end_frame=0;
        std::uint64_t forensic_timeline_writes=0;
        auto forensic_frame_path=[&](unsigned f){std::ostringstream n;n<<"frame_"<<std::setfill('0')<<std::setw(8)<<f<<".chqstate";return forensic_timeline_root/"frames"/n.str();};
        auto forensic_write_manifest=[&](const char* state){
            if(forensic_timeline_root.empty())return;
            std::ofstream m(forensic_timeline_root/"manifest.json"); if(!m)return;
            m<<"{\n  \"schema\": \"chq-forensic-timeline-v1\",\n  \"build\": \""<<chq::kNativeVersion<<"\",\n"
             <<"  \"state\": \""<<state<<"\",\n  \"startFrame\": "<<forensic_timeline_start_frame<<",\n  \"endFrame\": "<<forensic_timeline_end_frame<<",\n  \"frameCount\": "<<forensic_timeline_frames.size()<<",\n  \"writeCount\": "<<forensic_timeline_writes<<",\n"
             <<"  \"storage\": \"per-frame-chqstate\",\n  \"seekable\": true,\n  \"forkLive\": true,\n"
             <<"  \"captures\": {\"cpuA\":true,\"cpuB\":true,\"mutableMemory\":true,\"palette\":true,\"ioc\":true,\"spritesRaw\":true,\"tilemapRaw\":true,\"roadRaw\":true,\"writeProvenance\":true,\"audioRuntime\":false,\"audioStub\":true},\n"
             <<"  \"notes\": [\"Sprite/object RAM, tilemap, palette and road state are checkpointed every frame and can be inspected through the normal live APIs after timeline.seek.\",\"The current native runtime has only sound_stub; real sound CPU/chip/PCM capture is not yet available and is explicitly not claimed by this timeline version.\",\"timeline fork keeps the selected checkpoint loaded and returns control to the live emulator for counterfactual experiments.\"]\n}\n";
        };
        auto forensic_capture_frame=[&](unsigned f){
            if(!forensic_timeline_recording||!runtime)return;
            std::filesystem::create_directories(forensic_timeline_root/"frames");
            runtime->save_checkpoint(forensic_frame_path(f),f);
            if(forensic_timeline_frames.empty()||forensic_timeline_frames.back()!=f)forensic_timeline_frames.push_back(f);
            forensic_timeline_end_frame=f;
            if(forensic_timeline_index_stream){forensic_timeline_index_stream<<f<<",frames/frame_"<<std::setfill('0')<<std::setw(8)<<f<<".chqstate\n";forensic_timeline_index_stream.flush();}
        };
        auto forensic_stop=[&](){
            if(!forensic_timeline_recording)return;
            forensic_timeline_recording=false;
            runtime->bus.clear_timeline_write_sink();
            if(forensic_timeline_index_stream)forensic_timeline_index_stream.close();
            if(forensic_timeline_write_stream)forensic_timeline_write_stream.close();
            forensic_write_manifest("complete");
        };
        auto forensic_start=[&](const std::filesystem::path& root){
            if(forensic_timeline_recording)throw std::runtime_error("timeline recording already active");
            if(forensic_timeline_loaded)throw std::runtime_error("cannot record while a timeline is loaded; fork or unload first");
            forensic_timeline_root=root;forensic_timeline_frames.clear();forensic_timeline_index=0;forensic_timeline_writes=0;
            forensic_timeline_start_frame=frame;forensic_timeline_end_frame=frame;
            std::filesystem::create_directories(forensic_timeline_root/"frames");
            forensic_timeline_index_stream.open(forensic_timeline_root/"frames.csv");
            forensic_timeline_write_stream.open(forensic_timeline_root/"writes.csv");
            if(!forensic_timeline_index_stream||!forensic_timeline_write_stream)throw std::runtime_error("cannot create timeline files");
            forensic_timeline_index_stream<<"frame,checkpoint\n";
            forensic_timeline_write_stream<<"sequence,frame,cpu,pc,address,width,old_value,new_value,changed\n";
            forensic_timeline_recording=true;
            runtime->bus.set_timeline_write_sink([&](const chq::MemoryWriteTraceEvent& e){
                if(!forensic_timeline_recording||!forensic_timeline_write_stream)return;
                ++forensic_timeline_writes;
                forensic_timeline_write_stream<<e.write_count<<','<<e.frame<<','<<(e.space==chq::BusSpace::Main?'A':'B')<<",0x"<<std::hex<<e.pc<<",0x"<<e.address<<std::dec<<','<<(e.size*8)<<",0x"<<std::hex<<e.old_value<<",0x"<<e.new_value<<std::dec<<','<<(e.changed?1:0)<<'\n';
            });
            forensic_capture_frame(frame);forensic_write_manifest("recording");
        };
        auto forensic_scan_frames=[&](const std::filesystem::path& root){
            std::vector<unsigned> v;const auto dir=root/"frames";if(!std::filesystem::exists(dir))return v;
            for(const auto& de:std::filesystem::directory_iterator(dir)){if(!de.is_regular_file())continue;const auto n=de.path().filename().string();if(n.size()!=23||n.rfind("frame_",0)!=0||n.substr(n.size()-9)!=".chqstate")continue;try{v.push_back(static_cast<unsigned>(std::stoul(n.substr(6,8))));}catch(...){}}
            std::sort(v.begin(),v.end());v.erase(std::unique(v.begin(),v.end()),v.end());return v;
        };
        auto forensic_seek=[&](unsigned wanted)->unsigned{
            if(!forensic_timeline_loaded)throw std::runtime_error("no timeline loaded");
            auto it=std::lower_bound(forensic_timeline_frames.begin(),forensic_timeline_frames.end(),wanted);
            if(it==forensic_timeline_frames.end()||*it!=wanted)throw std::runtime_error("timeline frame not found: "+std::to_string(wanted));
            forensic_timeline_index=static_cast<std::size_t>(it-forensic_timeline_frames.begin());
            frame=runtime->load_checkpoint(forensic_frame_path(wanted));debug_paused=true;debug_step_remaining=0;changed=true;return frame;
        };
        chq::DebugApiServer debug_api;
        if(runtime && options.debug_api){
            std::string api_error;
            if(debug_api.start(static_cast<std::uint16_t>(options.debug_api_port),api_error))
                std::cout<<console_stamp(process_start)<<"[DEBUG API] listening on 127.0.0.1:"<<debug_api.port()<<" (chqctl.exe)\n";
            else
                std::cout<<console_stamp(process_start)<<"[DEBUG API] disabled: "<<api_error<<"\n";
        }
        bool first_loop_iteration = true;
        const unsigned fast_forward_start_frame = frame;
        const auto fast_forward_wall_start = std::chrono::steady_clock::now();
        bool fast_forward_reported = false;
        while (running) {
            if (first_loop_iteration) write_run_phase("run_loop_entered", frame);
            const auto tick = std::chrono::steady_clock::now();
            const bool fast_forwarding = runtime && frame < options.fast_forward_to;
            if (runtime) runtime->set_quiet_fast_forward(fast_forwarding);
            if (runtime && options.fast_forward_to && !fast_forwarding && !fast_forward_reported) {
                const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now()-fast_forward_wall_start).count();
                const unsigned ff_frames = frame > fast_forward_start_frame ? frame-fast_forward_start_frame : 0;
                const double fps = seconds > 0.0 ? double(ff_frames)/seconds : 0.0;
                const double realtime = fps/60.0;
                std::cout << console_stamp(process_start) << "[FAST-FORWARD COMPLETE] " << fast_forward_start_frame << " -> " << frame
                          << " frames=" << ff_frames << " wall=" << std::fixed << std::setprecision(3) << seconds
                          << "s rate=" << std::setprecision(1) << fps << " f/s realtime=" << std::setprecision(1) << realtime << "x\n" << std::defaultfloat;
                std::ofstream ff(options.logs / "fast_forward_summary.txt");
                if(ff) ff << "build=" << chq::kNativeVersion << "\nstart_frame=" << fast_forward_start_frame << "\ntarget_frame=" << options.fast_forward_to
                          << "\nreached_frame=" << frame << "\nframes=" << ff_frames << "\nwall_seconds=" << seconds << "\nframes_per_second=" << fps << "\nrealtime_multiplier=" << realtime << "\n";
                fast_forward_reported = true;
            }
            chq::DebugUiActions ui_actions{};
            if (!fast_forwarding || (options.fast_forward_event_every && (frame % options.fast_forward_event_every)==0))
                video.process_events(running, scene, changed, ui_actions);

            // v0.66.3.1 native gameplay controls: route keyboard state through the same
            // IOC values used by the arcade code and debugger injection.
            if(runtime){
                if(ui_actions.gameplay_steering_set) runtime->bus.set_ioc_steering(ui_actions.gameplay_steering);
                auto set_ioc_bit=[&](std::uint8_t port,std::uint8_t bit,bool on){
                    auto mask=runtime->bus.ioc_input_xor_mask(port);
                    if(on) mask=static_cast<std::uint8_t>(mask|bit); else mask=static_cast<std::uint8_t>(mask&~bit);
                    runtime->bus.set_ioc_input_xor_mask(port,mask);
                };
                if(ui_actions.gameplay_accel_set) set_ioc_bit(3,0x20,ui_actions.gameplay_accel);
                if(ui_actions.gameplay_brake_set) set_ioc_bit(2,0x20,ui_actions.gameplay_brake);
                if(ui_actions.gameplay_turbo_set) set_ioc_bit(3,0x01,ui_actions.gameplay_turbo);
            }

            if(runtime && !live_ioc_pulses.empty()){
                for(auto it=live_ioc_pulses.begin();it!=live_ioc_pulses.end();){
                    if(frame>=it->end_frame){ runtime->bus.set_ioc_input_xor_mask(it->port,it->prior); it=live_ioc_pulses.erase(it); }
                    else ++it;
                }
            }
            if(runtime && debug_api.running()){
                debug_api.poll([&](const std::string& line)->std::string{
                    const auto a=split_command_words(line); if(a.empty()) return "ERR empty command";
                    auto lower=[](std::string x){ for(char&c:x)c=static_cast<char>(std::tolower(static_cast<unsigned char>(c))); return x; };
                    const auto c0=lower(a[0]); std::ostringstream o;
                    auto need=[&](std::size_t n)->bool{if(a.size()<n){o<<"ERR insufficient arguments";return false;}return true;};
                    auto fmt_hex=[](std::uint32_t v){std::ostringstream x;x<<"0x"<<std::hex<<std::uppercase<<v;return x.str();};
                    if(c0=="help") return "OK commands: version | capabilities | status | pause | resume | timer status|freeze|resume | window status|top|fullscreen|scale|show|hide|minimize|restore ... | baseline save|restore | snapshot save|restore|list|remove NAME | run N | step frame [N] | step instr A|B | disasm A|B ADDR [COUNT] | read A|B ADDR WIDTH | readrange A|B ADDR LENGTH | write A|B ADDR WIDTH VALUE | regs A|B [REG] | reg read|write A|B REG [VALUE] | patch freeze|replace|suppress|list|remove|clear ... | watch add|remove|list|clear ... | trace start|stop|add|remove|list|clear ... | events tail|clear|save|limit ... | why mem A|B ADDR | changed A|B START:END [since FRAME] | input steering|xor|pulse|ports ... | palette trace start|status|tail|stop|clear ... | memory trace start|status|tail|stop|clear ... | target one-hit status|on|off | sprites tail|save ... | checkpoint save|load PATH | timeline start|stop|status|load|seek|next|prev|fork|unload PATH | screenshot [PATH] | graphics snapshot PATH | legacytrace status|reset|mem|pc|trigger|cpu|window|context|max|output|start|stop ... | provenance status|reset|follow|access|window|max|output|start|stop ...";
                    if(c0=="version") return std::string("OK build=")+chq::kNativeVersion+" product=ChaseHQ-Native api=0.66.9.0-RC2.9 transport=localhost-tcp";
                    if(c0=="capabilities") return "OK execution.pause execution.resume execution.step_frame execution.step_instruction snapshot.named baseline.save baseline.restore memory.read memory.read_range memory.write register.read register.write patch.freeze patch.replace patch.suppress patch.remove watch.memory_range watch.remove events.tail events.save events.limit query.why_memory query.changed_memory input.steering input.xor input.pulse input.ports cpu.disassemble tuning.handling palette.inspect palette.freeze palette.restore palette.trace memory.trace sprites.tail sprites.save sprites.list sprites.inspect sprites.override sprites.override.clear sprites.visual sprites.visual.clear checkpoint.save checkpoint.load timeline.record timeline.load timeline.seek timeline.fork timeline.write_history screenshot graphics.layered_snapshot timer.freeze graphics.layer_offsets graphics.tc0100_compare window.always_on_top window.fullscreen window.scale window.visibility window.minimize_restore legacytrace.dynamic provenance.dynamic";
                    if(c0=="status"){
                        o<<"OK build="<<chq::kNativeVersion<<" frame="<<frame<<" paused="<<(debug_paused?1:0)<<" step_remaining="<<debug_step_remaining<<" research="<<(runtime->bus.research_enabled()?1:0)
                         <<" events="<<runtime->bus.research_events().size()<<" watches="<<runtime->bus.research_watches().size()<<" patches="<<runtime->bus.research_patches().size()
                         <<" baseline="<<(workbench_have_pause_point?1:0)<<" source="<<(forensic_timeline_loaded?"timeline":"live")<<" timeline_recording="<<(forensic_timeline_recording?1:0)<<" timeline_loaded="<<(forensic_timeline_loaded?1:0)<<" timer_frozen="<<(timer_frozen?1:0)<<" always_on_top="<<(video.always_on_top()?1:0)<<" fullscreen="<<(video.fullscreen()?1:0)<<" window_visible="<<(video.window_visible()?1:0)<<" window_minimized="<<(video.window_minimized()?1:0)<<" window_scale="<<video.window_scale()<<" handling_override="<<((std::abs(runtime->handling_cornering_scale()-1.0)>1e-12||std::abs(runtime->handling_speed_retain()-1.0)>1e-12)?1:0)<<" target_one_hit="<<(runtime->target_one_hit()?1:0)<<" palette_overrides="<<live_palette_overrides.size()<<" fault="<<(runtime->fault.empty()?"none":runtime->fault); return o.str();
                    }
                    if(c0=="timer") {
                        const auto op=a.size()>1?lower(a[1]):"status";
                        if(op=="status"||op=="get") return std::string("OK timer_frozen=")+(timer_frozen?"1":"0");
                        if(op=="freeze"||op=="on"){if(!timer_frozen){frozen_timer_value=static_cast<std::uint8_t>(runtime->bus.peek(0x100200,1,chq::BusSpace::Main));frozen_timer_fraction=static_cast<std::uint8_t>(runtime->bus.peek(0x100201,1,chq::BusSpace::Main));}timer_frozen=true;changed=true;o<<"OK timer_frozen=1 value="<<fmt_hex(frozen_timer_value)<<" fraction="<<fmt_hex(frozen_timer_fraction);return o.str();}
                        if(op=="resume"||op=="off"||op=="unfreeze"){timer_frozen=false;changed=true;return "OK timer_frozen=0";}
                        if(op=="toggle"){if(!timer_frozen){frozen_timer_value=static_cast<std::uint8_t>(runtime->bus.peek(0x100200,1,chq::BusSpace::Main));frozen_timer_fraction=static_cast<std::uint8_t>(runtime->bus.peek(0x100201,1,chq::BusSpace::Main));timer_frozen=true;}else timer_frozen=false;changed=true;return std::string("OK timer_frozen=")+(timer_frozen?"1":"0");}
                        return "ERR timer expects status|freeze|resume|toggle";
                    }
                    if(c0=="layer-offset") {
                        const auto op=a.size()>1?lower(a[1]):"get";
                        if(op=="get") return video.layer_offsets_status();
                        if(op=="set" && a.size()==5) { try { if(!video.set_layer_offset(lower(a[2]),std::stoi(a[3]),std::stoi(a[4]))) return "ERR invalid layer or offset (range -4096..4096)"; } catch(...) { return "ERR offset must be integer"; } }
                        else if(op=="reset" && a.size()<=3) { if(!video.set_layer_offset(a.size()>2?lower(a[2]):"all",0,0)) return "ERR invalid layer"; }
                        else return "ERR layer-offset get|set LAYER X Y|reset [LAYER]";
                        changed=true; return video.layer_offsets_status();
                    }
                    if(c0=="tc0100-mode") {
                        if(a.size()>1) { const auto mode=lower(a[1]); if(mode!="legacy" && mode!="corrected") return "ERR tc0100-mode legacy|corrected"; video.set_tc0100scn_legacy_x(mode=="legacy"); changed=true; }
                        return std::string("OK tc0100_mode=")+(video.tc0100scn_legacy_x()?"legacy":"corrected");
                    }
                    if(c0=="window") {
                        const auto op=a.size()>1?lower(a[1]):"status";
                        if(op=="status"||op=="get"){o<<"OK always_on_top="<<(video.always_on_top()?1:0)<<" fullscreen="<<(video.fullscreen()?1:0)<<" visible="<<(video.window_visible()?1:0)<<" minimized="<<(video.window_minimized()?1:0)<<" scale="<<video.window_scale();return o.str();}
                        if(op=="top"||op=="always-on-top"){const auto sub=a.size()>2?lower(a[2]):"get";if(sub=="get"||sub=="status")return std::string("OK always_on_top=")+(video.always_on_top()?"1":"0");if(sub=="on"||sub=="1"||sub=="true"){if(!video.set_always_on_top(true))return "ERR failed to enable always-on-top";return "OK always_on_top=1";}if(sub=="off"||sub=="0"||sub=="false"){if(!video.set_always_on_top(false))return "ERR failed to disable always-on-top";return "OK always_on_top=0";}return "ERR window top expects get|on|off";}
                        if(op=="fullscreen"){const auto sub=a.size()>2?lower(a[2]):"get";if(sub=="get"||sub=="status")return std::string("OK fullscreen=")+(video.fullscreen()?"1":"0");bool on=(sub=="on"||sub=="1"||sub=="true");if(!(on||sub=="off"||sub=="0"||sub=="false"))return "ERR window fullscreen expects get|on|off";if(!video.set_fullscreen(on))return "ERR failed to change fullscreen";return std::string("OK fullscreen=")+(on?"1":"0");}
                        if(op=="scale"){if(a.size()<3)return "ERR window scale expects 1..8";int n=std::stoi(a[2]);if(!video.set_window_scale(n))return "ERR failed to set scale (1..8)";return "OK scale="+std::to_string(video.window_scale());}
                        if(op=="show"){if(!video.show_window(true))return "ERR failed to show window";return "OK visible=1";}
                        if(op=="hide"){if(!video.show_window(false))return "ERR failed to hide window";return "OK visible=0";}
                        if(op=="minimize"){if(!video.minimize_window())return "ERR failed to minimize window";return "OK minimized=1";}
                        if(op=="restore"){if(!video.restore_window())return "ERR failed to restore window";return "OK minimized=0 visible=1";}
                        return "ERR window expects status|top|fullscreen|scale|show|hide|minimize|restore";
                    }
                    if(c0=="pause"){debug_paused=true;debug_step_remaining=0;changed=true;return "OK paused frame="+std::to_string(frame);}
                    if(c0=="resume"){if(forensic_timeline_loaded)return "ERR timeline source is read-only; use timeline fork for live execution";debug_paused=false;debug_step_remaining=0;changed=true;return "OK running frame="+std::to_string(frame);}
                    if(c0=="baseline"){
                        const std::string op=a.size()>1?lower(a[1]):"save";
                        if(op=="save"||op=="create") { debug_paused=true;debug_step_remaining=0;std::filesystem::create_directories(workbench_dir);runtime->save_checkpoint(workbench_pause_point,frame);workbench_have_pause_point=true;return "OK baseline saved frame="+std::to_string(frame); }
                        if(op=="restore") { if(!workbench_have_pause_point)return "ERR no baseline";runtime->bus.research_clear_patches();frame=runtime->load_checkpoint(workbench_pause_point);debug_paused=true;debug_step_remaining=0;changed=true;return "OK baseline restored frame="+std::to_string(frame); }
                        return "ERR baseline expects save|restore";
                    }
                    if(c0=="snapshot"){
                        if(a.size()<2)return "ERR snapshot save|restore|list|remove NAME"; const auto op=lower(a[1]);
                        if(op=="list"){o<<"OK snapshots="<<named_snapshots.size();for(const auto&[name,path]:named_snapshots)o<<"\n"<<name<<" "<<path.string();return o.str();}
                        if(a.size()<3)return "ERR snapshot "+op+" requires NAME"; const auto name=a[2]; const auto path=workbench_dir/(std::string("snapshot_")+name+".chqstate");
                        if(op=="save"||op=="create"){debug_paused=true;debug_step_remaining=0;std::filesystem::create_directories(workbench_dir);runtime->save_checkpoint(path,frame);named_snapshots[name]=path;return "OK snapshot saved name="+name+" frame="+std::to_string(frame);}
                        if(op=="restore"){auto it=named_snapshots.find(name);if(it==named_snapshots.end()&&!std::filesystem::exists(path))return "ERR unknown snapshot "+name;runtime->bus.research_clear_patches();frame=runtime->load_checkpoint(path);debug_paused=true;debug_step_remaining=0;changed=true;named_snapshots[name]=path;return "OK snapshot restored name="+name+" frame="+std::to_string(frame);}
                        if(op=="remove"||op=="delete"){std::error_code ec;std::filesystem::remove(path,ec);const auto n=named_snapshots.erase(name);return n||!ec?"OK snapshot removed "+name:"ERR snapshot remove failed";}
                        return "ERR snapshot expects save|restore|list|remove";
                    }
                    if(c0=="run") { if(!need(2))return o.str(); std::uint32_t n=0;if(!parse_api_u32(a[1],n)||n<1)return "ERR run expects positive frame count";if(forensic_timeline_loaded){const auto remaining=forensic_timeline_frames.size()-1-forensic_timeline_index;const auto advance=std::min<std::size_t>(n,remaining);forensic_timeline_index+=advance;frame=runtime->load_checkpoint(forensic_frame_path(forensic_timeline_frames[forensic_timeline_index]));debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline advanced="<<advance<<" frame="<<frame<<" index="<<forensic_timeline_index;return o.str();}debug_paused=true;debug_step_remaining+=n;return "OK running "+std::to_string(n)+" frame(s) from "+std::to_string(frame); }
                    if(c0=="disasm"||c0=="disassemble") {
                        if(a.size()<3)return "ERR disasm A|B ADDR [COUNT]";
                        chq::BusSpace sp{};std::uint32_t pc=0,count=16;
                        if(!parse_api_space(a[1],sp)||!parse_api_hex_u32(a[2],pc))return "ERR bad disasm arguments";
                        if(a.size()>3&&(!parse_api_u32(a[3],count)||count<1||count>256))return "ERR disasm count 1..256";
                        o<<"OK cpu="<<(sp==chq::BusSpace::Main?'A':'B')<<" start="<<fmt_hex(pc)<<" count="<<count;
                        for(std::uint32_t i=0;i<count;++i){unsigned size=0;const auto text=runtime->disassemble(sp,pc,&size);if(size==0)size=2;o<<"\n"<<fmt_hex(pc)<<"  "<<text;pc+=size;}
                        return o.str();
                    }
                    if(c0=="step"){
                        if(!need(2))return o.str(); const auto kind=lower(a[1]);
                        if(kind=="frame"||kind=="frames"){std::uint32_t n=1;if(a.size()>2&&(!parse_api_u32(a[2],n)||n<1))return "ERR bad frame count";if(forensic_timeline_loaded){const auto remaining=forensic_timeline_frames.size()-1-forensic_timeline_index;const auto advance=std::min<std::size_t>(n,remaining);forensic_timeline_index+=advance;frame=runtime->load_checkpoint(forensic_frame_path(forensic_timeline_frames[forensic_timeline_index]));debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline step frames="<<advance<<" frame="<<frame<<" index="<<forensic_timeline_index;return o.str();}debug_paused=true;debug_step_remaining+=n;return "OK step frames="+std::to_string(n);}
                        if(kind=="instr"||kind=="instruction"){if(forensic_timeline_loaded)return "ERR instruction stepping is unavailable for an imported timeline; use timeline fork for live execution";if(!need(3))return o.str();chq::BusSpace sp;if(!parse_api_space(a[2],sp))return "ERR CPU must be A or B";debug_paused=true;debug_step_remaining=0;const int used=runtime->step_instruction(sp);changed=true;o<<"OK CPU="<<(sp==chq::BusSpace::Main?'A':'B')<<" cycles="<<used<<" frame="<<frame;return o.str();}
                        return "ERR step expects frame [N] | instr A|B";
                    }
                    if(c0=="memory"&&a.size()>1&&lower(a[1])=="trace") {
                        const auto sub=a.size()>2?lower(a[2]):"status";
                        if(sub=="status"){
                            o<<"OK enabled="<<(runtime->bus.memory_trace_enabled()?1:0)
                             <<" events="<<runtime->bus.memory_trace_events().size()
                             <<" limit="<<runtime->bus.memory_trace_limit()
                             <<" cpu="<<(runtime->bus.memory_trace_space()==chq::BusSpace::Main?'A':'B')
                             <<" address="<<fmt_hex(runtime->bus.memory_trace_address())
                             <<" length="<<runtime->bus.memory_trace_length()
                             <<" width="<<(runtime->bus.memory_trace_width()?std::to_string(runtime->bus.memory_trace_width()*8):std::string("any"));
                            return o.str();
                        }
                        if(sub=="clear"){runtime->bus.memory_trace_clear();return "OK memory trace events cleared";}
                        if(sub=="stop"){runtime->bus.memory_trace_stop();return "OK memory trace stopped events="+std::to_string(runtime->bus.memory_trace_events().size());}
                        if(sub=="start"){
                            if(a.size()<5)return "ERR memory trace start A|B ADDRESS [LENGTH] [WIDTH|any] [LIMIT]";
                            chq::BusSpace sp{}; std::uint32_t addr=0,len=1,lim=4096; unsigned width=0;
                            if(!parse_api_space(a[3],sp)||!parse_api_hex_u32(a[4],addr))return "ERR bad memory trace CPU/address";
                            if(a.size()>5&&(!parse_api_u32(a[5],len)||len<1||len>0x1000000u))return "ERR memory trace length 1..16777216";
                            if(a.size()>6&&lower(a[6])!="any"&&a[6]!="0"&&!parse_api_width(a[6],width))return "ERR memory trace width 8|16|32|any";
                            if(a.size()>7&&(!parse_api_u32(a[7],lim)||lim<64||lim>1000000))return "ERR memory trace limit 64..1000000";
                            runtime->bus.memory_trace_clear();runtime->bus.memory_trace_start(sp,addr,len,width,lim);runtime->bus.research_enable(true);
                            o<<"OK memory trace started cpu="<<(sp==chq::BusSpace::Main?'A':'B')<<" address="<<fmt_hex(addr)<<" length="<<len<<" width="<<(width?std::to_string(width*8):std::string("any"))<<" limit="<<lim;return o.str();
                        }
                        if(sub=="tail"){
                            std::uint32_t n=20;if(a.size()>3&&!parse_api_u32(a[3],n))return "ERR bad count";const auto&e=runtime->bus.memory_trace_events();const auto start=e.size()>n?e.size()-n:0;
                            o<<"OK memory_events="<<(e.size()-start);for(std::size_t i=start;i<e.size();++i){const auto&x=e[i];o<<"\nframe="<<x.frame<<" CPU="<<(x.space==chq::BusSpace::Main?'A':'B')<<" pc="<<fmt_hex(x.pc)<<" address="<<fmt_hex(x.address)<<" width="<<(x.size*8)<<" old="<<fmt_hex(x.old_value)<<" new="<<fmt_hex(x.new_value)<<" changed="<<(x.changed?1:0)<<" write_count="<<x.write_count;}return o.str();
                        }
                        return "ERR memory trace expects start|status|tail|stop|clear";
                    }
                    if(c0=="read"||(c0=="mem"&&a.size()>1&&lower(a[1])=="read")){
                        const std::size_t k=c0=="mem"?2:1; if(a.size()<k+3)return "ERR read A|B ADDR WIDTH"; chq::BusSpace sp;std::uint32_t addr;unsigned sz;
                        if(!parse_api_space(a[k],sp)||!parse_api_hex_u32(a[k+1],addr)||!parse_api_width(a[k+2],sz))return "ERR bad read arguments";const auto v=runtime->bus.peek(addr,sz,sp);o<<"OK cpu="<<(sp==chq::BusSpace::Main?'A':'B')<<" addr="<<fmt_hex(addr)<<" width="<<(sz*8)<<" value="<<fmt_hex(v);return o.str();
                    }
                    if(c0=="readrange"||(c0=="mem"&&a.size()>1&&lower(a[1])=="readrange")){
                        const std::size_t k=c0=="mem"?2:1; if(a.size()<k+3)return "ERR readrange A|B ADDR LENGTH"; chq::BusSpace sp;std::uint32_t addr,len;
                        if(!parse_api_space(a[k],sp)||!parse_api_hex_u32(a[k+1],addr)||!parse_api_u32(a[k+2],len)||len<1||len>65536)return "ERR bad readrange arguments (length 1..65536)";
                        o<<"OK cpu="<<(sp==chq::BusSpace::Main?'A':'B')<<" addr="<<fmt_hex(addr)<<" length="<<len;
                        for(std::uint32_t base=0;base<len;base+=16){o<<"\n";o<<std::hex<<std::uppercase<<std::setw(8)<<std::setfill('0')<<(addr+base)<<":";const auto take=std::min<std::uint32_t>(16,len-base);for(std::uint32_t i=0;i<take;++i){const auto v=runtime->bus.peek(addr+base+i,1,sp);o<<" "<<std::setw(2)<<std::setfill('0')<<(v&0xFFu);}o<<std::dec;}
                        return o.str();
                    }
                    if(c0=="write"||(c0=="mem"&&a.size()>1&&lower(a[1])=="write")){
                        if(forensic_timeline_loaded)return "ERR timeline source is read-only; use timeline fork before memory writes";
                        const std::size_t k=c0=="mem"?2:1;if(a.size()<k+4)return "ERR write A|B ADDR WIDTH VALUE";chq::BusSpace sp;std::uint32_t addr,val;unsigned sz;
                        if(!parse_api_space(a[k],sp)||!parse_api_hex_u32(a[k+1],addr)||!parse_api_width(a[k+2],sz)||!parse_api_hex_u32(a[k+3],val))return "ERR bad write arguments";debug_paused=true;debug_step_remaining=0;const auto old=runtime->bus.peek(addr,sz,sp);runtime->bus.debug_write(addr,val,sz,sp);changed=true;o<<"OK "<<fmt_hex(addr)<<" "<<fmt_hex(old)<<" -> "<<fmt_hex(val);return o.str();
                    }
                    if(c0=="regs"||c0=="reg"){
                        std::size_t k=1; std::string op="read"; if(c0=="reg"&&a.size()>1&&(lower(a[1])=="read"||lower(a[1])=="write")){op=lower(a[1]);k=2;}
                        if(a.size()<=k)return "ERR regs A|B [REG] | reg write A|B REG VALUE"; chq::BusSpace sp;if(!parse_api_space(a[k],sp))return "ERR CPU must be A or B";
                        if(op=="write"){if(forensic_timeline_loaded)return "ERR timeline source is read-only; use timeline fork before register writes";if(a.size()<k+3)return "ERR reg write A|B REG VALUE";std::uint32_t v;if(!parse_api_hex_u32(a[k+2],v))return "ERR bad register value";debug_paused=true;debug_step_remaining=0;if(!runtime->debug_set_register(sp,a[k+1],v))return "ERR unknown register";o<<"OK CPU="<<(sp==chq::BusSpace::Main?'A':'B')<<" "<<a[k+1]<<"="<<fmt_hex(runtime->debug_get_register(sp,a[k+1]));return o.str();}
                        if(a.size()>k+1){try{o<<"OK CPU="<<(sp==chq::BusSpace::Main?'A':'B')<<" "<<a[k+1]<<"="<<fmt_hex(runtime->debug_get_register(sp,a[k+1]));return o.str();}catch(const std::exception&e){return std::string("ERR ")+e.what();}}
                        o<<"OK CPU="<<(sp==chq::BusSpace::Main?'A':'B'); for(int i=0;i<8;++i)o<<" D"<<i<<"="<<fmt_hex(runtime->debug_get_register(sp,"D"+std::to_string(i)));for(int i=0;i<8;++i)o<<" A"<<i<<"="<<fmt_hex(runtime->debug_get_register(sp,"A"+std::to_string(i)));o<<" PC="<<fmt_hex(runtime->debug_get_register(sp,"PC"))<<" SR="<<fmt_hex(runtime->debug_get_register(sp,"SR"))<<" SP="<<fmt_hex(runtime->debug_get_register(sp,"SP"));return o.str();
                    }
                    if(c0=="patch"){
                        if(!need(2))return o.str();const auto op=lower(a[1]);if(forensic_timeline_loaded&&op!="list")return "ERR timeline source is read-only; use timeline fork before patching";
                        if(op=="clear"){runtime->bus.research_clear_patches();return "OK patches cleared";}
                        if(op=="remove"||op=="delete"){if(a.size()<3)return "ERR patch remove ID";std::uint32_t id;if(!parse_api_u32(a[2],id))return "ERR bad patch id";return runtime->bus.research_remove_patch(id)?"OK patch removed id="+std::to_string(id):"ERR patch id not found";}
                        if(op=="list"){o<<"OK patches="<<runtime->bus.research_patches().size();for(const auto&p:runtime->bus.research_patches())o<<"\n"<<p.id<<" CPU="<<(p.space==chq::BusSpace::Main?'A':'B')<<" addr="<<fmt_hex(p.address)<<" width="<<(p.size*8)<<" value="<<fmt_hex(p.value)<<" mode="<<(p.mode==chq::LivePatchMode::Freeze?"freeze":p.mode==chq::LivePatchMode::Replace?"replace":"suppress")<<" hits="<<p.interceptions<<" last_pc="<<fmt_hex(p.last_pc);return o.str();}
                        if(op=="freeze"){if(a.size()<6)return "ERR patch freeze A|B ADDR WIDTH VALUE";chq::BusSpace sp;std::uint32_t addr,val;unsigned sz;if(!parse_api_space(a[2],sp)||!parse_api_hex_u32(a[3],addr)||!parse_api_width(a[4],sz)||!parse_api_hex_u32(a[5],val))return "ERR bad patch arguments";debug_paused=true;runtime->bus.debug_write(addr,val,sz,sp);auto id=runtime->bus.research_add_patch(sp,addr,sz,val,chq::LivePatchMode::Freeze);o<<"OK patch id="<<id;return o.str();}
                        if(op=="replace"){if(a.size()<6)return "ERR patch replace A|B ADDR WIDTH VALUE";chq::BusSpace sp;std::uint32_t addr,val;unsigned sz;if(!parse_api_space(a[2],sp)||!parse_api_hex_u32(a[3],addr)||!parse_api_width(a[4],sz)||!parse_api_hex_u32(a[5],val))return "ERR bad patch arguments";auto id=runtime->bus.research_add_patch(sp,addr,sz,val,chq::LivePatchMode::Replace);o<<"OK patch id="<<id<<" replace_on_write="<<fmt_hex(val);return o.str();}
                        if(op=="suppress"){if(a.size()<5)return "ERR patch suppress A|B ADDR WIDTH";chq::BusSpace sp;std::uint32_t addr;unsigned sz;if(!parse_api_space(a[2],sp)||!parse_api_hex_u32(a[3],addr)||!parse_api_width(a[4],sz))return "ERR bad patch arguments";const auto keep=runtime->bus.peek(addr,sz,sp);auto id=runtime->bus.research_add_patch(sp,addr,sz,keep,chq::LivePatchMode::Suppress);o<<"OK patch id="<<id<<" preserve="<<fmt_hex(keep);return o.str();}
                        return "ERR patch expects freeze|replace|suppress|list|remove|clear";
                    }
                    if(c0=="watch"||c0=="trace"){
                        if(!need(2))return o.str();const bool trace_alias=c0=="trace";const auto op=lower(a[1]);
                        if(trace_alias&&op=="start"){runtime->bus.research_enable(true);return "OK research trace enabled";}
                        if(trace_alias&&op=="stop"){runtime->bus.research_enable(false);return "OK research trace disabled";}
                        if(op=="clear"){runtime->bus.research_clear_watches();return "OK watches cleared";}
                        if(op=="remove"||op=="delete"){if(a.size()<3)return "ERR watch remove ID";std::uint32_t id;if(!parse_api_u32(a[2],id))return "ERR bad watch id";return runtime->bus.research_remove_watch(id)?"OK watch removed id="+std::to_string(id):"ERR watch id not found";}
                        if(op=="list"){o<<"OK watches="<<runtime->bus.research_watches().size();for(const auto&w:runtime->bus.research_watches())o<<"\n"<<w.id<<" CPU="<<(w.space==chq::BusSpace::Main?'A':'B')<<" "<<fmt_hex(w.start)<<":"<<fmt_hex(w.end)<<" r="<<w.reads<<" w="<<w.writes<<" change="<<w.change_only<<" break="<<w.break_on_match;return o.str();}
                        std::size_t k=2;if(trace_alias&&op=="add"&&a.size()>2&&lower(a[2])=="mem")k=3;else if(op!="add")return "ERR watch/trace expects add|list|clear (trace also start|stop)";
                        if(a.size()<k+2)return "ERR watch add A|B ADDR[:END] [WIDTH] [read|write|rw] [change] [break]";chq::BusSpace sp;std::uint32_t start,end;if(!parse_api_space(a[k],sp)||!parse_api_range(a[k+1],start,end))return "ERR bad watch arguments";
                        std::size_t opt=k+2; if(start==end && a.size()>opt){unsigned sz=0;if(parse_api_width(a[opt],sz)){end=start+sz-1;++opt;}}
                        chq::ResearchWatch w{};w.space=sp;w.start=start;w.end=end;w.writes=true;for(std::size_t j=opt;j<a.size();++j){auto q=lower(a[j]);if(q=="read"||q=="r"){w.reads=true;w.writes=false;}else if(q=="rw"){w.reads=w.writes=true;}else if(q=="write"||q=="w")w.writes=true;else if(q=="change")w.change_only=true;else if(q=="break")w.break_on_match=true;}
                        runtime->bus.research_enable(true);const auto id=runtime->bus.research_add_watch(w);o<<"OK watch id="<<id<<" CPU="<<(sp==chq::BusSpace::Main?'A':'B')<<" "<<fmt_hex(w.start)<<":"<<fmt_hex(w.end);return o.str();
                    }
                    if(c0=="legacytrace"){
                        if(a.size()<2)return "ERR legacytrace status|reset|mem|pc|trigger|cpu|window|context|max|output|start|stop";const auto op=lower(a[1]);
                        if(op=="status"){o<<"OK enabled="<<(api_legacy_trace.enabled()?1:0)<<" mem="<<api_legacy_trace.mem_ranges.size()<<" pc="<<api_legacy_trace.pc_ranges.size()<<" triggers="<<api_legacy_trace.triggers.size()<<" cpu_mask="<<api_legacy_trace.cpu_mask<<" frames="<<api_legacy_trace.from_frame<<":"<<api_legacy_trace.to_frame<<" before="<<api_legacy_trace.before<<" after="<<api_legacy_trace.after<<" max="<<api_legacy_trace.max_lines<<" output="<<api_legacy_trace_path.string();return o.str();}
                        if(op=="reset"){api_legacy_trace=chq::TraceConfig{};runtime->configure_generic_trace(api_legacy_trace,api_legacy_trace_path);return "OK legacy trace reset/stopped";}
                        if(op=="stop"){chq::TraceConfig off{};runtime->configure_generic_trace(off,api_legacy_trace_path);return "OK legacy trace stopped configuration retained";}
                        if(op=="start"){if(a.size()>2)api_legacy_trace_path=a[2];runtime->configure_generic_trace(api_legacy_trace,api_legacy_trace_path);return "OK legacy trace started output="+api_legacy_trace_path.string();}
                        if(op=="output"){if(a.size()<3)return "ERR legacytrace output PATH";api_legacy_trace_path=a[2];return "OK legacy trace output="+api_legacy_trace_path.string();}
                        if(op=="cpu"){if(a.size()<3)return "ERR legacytrace cpu A|B|both";const auto q=lower(a[2]);if(q=="a"||q=="main")api_legacy_trace.cpu_mask=1;else if(q=="b"||q=="sub")api_legacy_trace.cpu_mask=2;else if(q=="both"||q=="ab")api_legacy_trace.cpu_mask=3;else return "ERR bad cpu";return "OK legacy trace cpu_mask="+std::to_string(api_legacy_trace.cpu_mask);}
                        if(op=="window"){if(a.size()<4)return "ERR legacytrace window FROM TO";std::uint32_t f,t;if(!parse_api_u32(a[2],f)||!parse_api_u32(a[3],t))return "ERR bad frame window";api_legacy_trace.from_frame=f;api_legacy_trace.to_frame=t;return "OK legacy trace window set";}
                        if(op=="context"){if(a.size()<4)return "ERR legacytrace context BEFORE AFTER";std::uint32_t b,af;if(!parse_api_u32(a[2],b)||!parse_api_u32(a[3],af))return "ERR bad context";api_legacy_trace.before=b;api_legacy_trace.after=af;return "OK legacy trace context set";}
                        if(op=="max"){if(a.size()<3)return "ERR legacytrace max LINES";std::uint32_t n;if(!parse_api_u32(a[2],n)||n<1)return "ERR bad max";api_legacy_trace.max_lines=n;return "OK legacy trace max="+std::to_string(n);}
                        if(op=="mem"){if(a.size()<3)return "ERR legacytrace mem START:END [r|w|rw] [change] [nonzero]";std::uint32_t st,en;if(!parse_api_range(a[2],st,en))return "ERR bad range";chq::TraceRange r{};r.start=st;r.end=en;r.reads=true;r.writes=true;for(std::size_t i=3;i<a.size();++i){auto q=lower(a[i]);if(q=="r"||q=="read"){r.reads=true;r.writes=false;}else if(q=="w"||q=="write"){r.reads=false;r.writes=true;}else if(q=="rw"){r.reads=r.writes=true;}else if(q=="change")r.change_only=true;else if(q=="nonzero")r.nonzero_only=true;}api_legacy_trace.mem_ranges.push_back(r);return "OK legacy trace mem added";}
                        if(op=="pc"){if(a.size()<3)return "ERR legacytrace pc START:END";std::uint32_t st,en;if(!parse_api_range(a[2],st,en))return "ERR bad range";api_legacy_trace.pc_ranges.push_back({st,en});return "OK legacy trace pc added";}
                        if(op=="trigger"){if(a.size()<3)return "ERR legacytrace trigger START:END [write|change|nonzero]";std::uint32_t st,en;if(!parse_api_range(a[2],st,en))return "ERR bad range";chq::TraceTrigger t{};t.start=st;t.end=en;t.kind=chq::TraceTriggerKind::Write;if(a.size()>3){auto q=lower(a[3]);if(q=="change")t.kind=chq::TraceTriggerKind::Change;else if(q=="nonzero")t.kind=chq::TraceTriggerKind::NonZeroWrite;}api_legacy_trace.triggers.push_back(t);return "OK legacy trace trigger added";}
                        return "ERR legacytrace expects status|reset|mem|pc|trigger|cpu|window|context|max|output|start|stop";
                    }
                    if(c0=="provenance"){
                        if(a.size()<2)return "ERR provenance status|reset|follow|access|window|max|output|start|stop";const auto op=lower(a[1]);
                        if(op=="status"){o<<"OK enabled="<<(api_provenance.enabled()?1:0)<<" follow="<<api_provenance.follow_ranges.size()<<" reads="<<api_provenance.reads<<" writes="<<api_provenance.writes<<" frames="<<api_provenance.from_frame<<":"<<api_provenance.to_frame<<" max="<<api_provenance.max_events<<" callstack="<<api_provenance.trace_callstack<<" callgraph="<<api_provenance.trace_callgraph<<" profile="<<api_provenance.profile_functions<<" output="<<api_provenance_dir.string();return o.str();}
                        if(op=="reset"){api_provenance=chq::ProvenanceConfig{};runtime->configure_provenance(api_provenance,api_provenance_dir);return "OK provenance reset/stopped";}
                        if(op=="stop"){chq::ProvenanceConfig off{};runtime->configure_provenance(off,api_provenance_dir);return "OK provenance stopped configuration retained";}
                        if(op=="start"){if(a.size()>2)api_provenance_dir=a[2];runtime->configure_provenance(api_provenance,api_provenance_dir);return "OK provenance started output="+api_provenance_dir.string();}
                        if(op=="output"){if(a.size()<3)return "ERR provenance output DIR";api_provenance_dir=a[2];return "OK provenance output="+api_provenance_dir.string();}
                        if(op=="follow"){if(a.size()<3)return "ERR provenance follow START:END";std::uint32_t st,en;if(!parse_api_range(a[2],st,en))return "ERR bad range";api_provenance.follow_ranges.push_back({st,en});return "OK provenance follow added";}
                        if(op=="access"){if(a.size()<3)return "ERR provenance access r|w|rw";auto q=lower(a[2]);api_provenance.access_filter_explicit=true;if(q=="r"||q=="read"){api_provenance.reads=true;api_provenance.writes=false;}else if(q=="w"||q=="write"){api_provenance.reads=false;api_provenance.writes=true;}else if(q=="rw"){api_provenance.reads=api_provenance.writes=true;}else return "ERR bad access";return "OK provenance access set";}
                        if(op=="window"){if(a.size()<4)return "ERR provenance window FROM TO";std::uint32_t f,t;if(!parse_api_u32(a[2],f)||!parse_api_u32(a[3],t))return "ERR bad window";api_provenance.from_frame=f;api_provenance.to_frame=t;return "OK provenance window set";}
                        if(op=="max"){if(a.size()<3)return "ERR provenance max EVENTS";std::uint32_t n;if(!parse_api_u32(a[2],n)||n<1)return "ERR bad max";api_provenance.max_events=n;return "OK provenance max="+std::to_string(n);}
                        if(op=="callstack"){api_provenance.trace_callstack=a.size()<3||lower(a[2])!="off";return std::string("OK provenance callstack=")+(api_provenance.trace_callstack?"1":"0");}
                        if(op=="callgraph"){api_provenance.trace_callgraph=a.size()<3||lower(a[2])!="off";return std::string("OK provenance callgraph=")+(api_provenance.trace_callgraph?"1":"0");}
                        if(op=="profile"){api_provenance.profile_functions=a.size()<3||lower(a[2])!="off";return std::string("OK provenance profile=")+(api_provenance.profile_functions?"1":"0");}
                        return "ERR provenance expects status|reset|follow|access|window|max|callstack|callgraph|profile|output|start|stop";
                    }
                    if(c0=="events"){
                        if(!need(2))return o.str();const auto op=lower(a[1]);
                        if(op=="clear"){runtime->bus.research_clear_events();return "OK events cleared";}
                        if(op=="limit"){if(a.size()<3){return "OK event_limit="+std::to_string(runtime->bus.research_event_limit());}std::uint32_t n;if(!parse_api_u32(a[2],n))return "ERR bad event limit";runtime->bus.research_set_event_limit(n);return "OK event_limit="+std::to_string(runtime->bus.research_event_limit());}
                        if(op=="tail"){std::uint32_t n=20;if(a.size()>2&&!parse_api_u32(a[2],n))return "ERR bad count";const auto&e=runtime->bus.research_events();const auto start=e.size()>n?e.size()-n:0;o<<"OK events="<<(e.size()-start);for(std::size_t i=start;i<e.size();++i){const auto&x=e[i];o<<"\nframe="<<x.frame<<" kind="<<(x.kind==chq::ResearchEventKind::Read?"READ":x.kind==chq::ResearchEventKind::Write?"WRITE":x.kind==chq::ResearchEventKind::Intervention?"PATCH":"SUPPRESS")<<" CPU="<<(x.space==chq::BusSpace::Main?'A':'B')<<" pc="<<fmt_hex(x.pc)<<" addr="<<fmt_hex(x.address)<<" width="<<(x.size*8)<<" old="<<fmt_hex(x.old_value)<<" new="<<fmt_hex(x.new_value)<<" changed="<<x.changed;}return o.str();}
                        if(op=="save"){if(a.size()<3)return "ERR events save PATH";std::ofstream f(a[2]);if(!f)return "ERR could not open output";f<<"frame,kind,cpu,pc,address,width,old_value,new_value,changed\n";for(const auto&e:runtime->bus.research_events())f<<e.frame<<','<<(e.kind==chq::ResearchEventKind::Read?"READ":e.kind==chq::ResearchEventKind::Write?"WRITE":e.kind==chq::ResearchEventKind::Intervention?"INTERVENTION":"SUPPRESSED_WRITE")<<','<<(e.space==chq::BusSpace::Main?'A':'B')<<",0x"<<std::hex<<e.pc<<",0x"<<e.address<<std::dec<<','<<(e.size*8)<<",0x"<<std::hex<<e.old_value<<",0x"<<e.new_value<<std::dec<<','<<(e.changed?1:0)<<'\n';return "OK events saved "+a[2];}
                        return "ERR events expects tail|clear|save|limit";
                    }
                    if(c0=="why"){
                        if(a.size()<4||lower(a[1])!="mem")return "ERR why mem A|B ADDR";chq::BusSpace sp;std::uint32_t addr;if(!parse_api_space(a[2],sp)||!parse_api_hex_u32(a[3],addr))return "ERR bad why arguments";
                        const chq::ResearchEvent* lastw=nullptr;std::vector<const chq::ResearchEvent*> reads;for(auto it=runtime->bus.research_events().rbegin();it!=runtime->bus.research_events().rend();++it){const auto& e=*it;if(e.space!=sp||addr<e.address||addr>=e.address+e.size)continue;if(!lastw&&(e.kind==chq::ResearchEventKind::Write||e.kind==chq::ResearchEventKind::Intervention||e.kind==chq::ResearchEventKind::SuppressedWrite))lastw=&e;if(e.kind==chq::ResearchEventKind::Read&&reads.size()<8)reads.push_back(&e);}
                        o<<"OK cpu="<<(sp==chq::BusSpace::Main?'A':'B')<<" addr="<<fmt_hex(addr)<<" current="<<fmt_hex(runtime->bus.peek(addr,1,sp));if(lastw)o<<"\nlast_write frame="<<lastw->frame<<" pc="<<fmt_hex(lastw->pc)<<" width="<<(lastw->size*8)<<" old="<<fmt_hex(lastw->old_value)<<" new="<<fmt_hex(lastw->new_value);else o<<"\nlast_write none_in_event_buffer";for(const auto*r:reads)o<<"\nrecent_read frame="<<r->frame<<" pc="<<fmt_hex(r->pc)<<" width="<<(r->size*8);return o.str();
                    }
                    if(c0=="changed"){
                        if(a.size()<3)return "ERR changed A|B START:END [since FRAME]";chq::BusSpace sp;std::uint32_t start,end;if(!parse_api_space(a[1],sp)||!parse_api_range(a[2],start,end))return "ERR bad changed arguments";unsigned since=0;if(a.size()>4&&lower(a[3])=="since"){std::uint32_t x;if(!parse_api_u32(a[4],x))return "ERR bad since frame";since=x;}
                        std::map<std::uint32_t,const chq::ResearchEvent*> latest;for(const auto&e:runtime->bus.research_events()){if(e.space!=sp||e.frame<since||!e.changed||e.address>end||(e.address+e.size-1)<start)continue;latest[e.address]=&e;}o<<"OK changed="<<latest.size();for(const auto&[addr,e]:latest)o<<"\nframe="<<e->frame<<" addr="<<fmt_hex(addr)<<" width="<<(e->size*8)<<" "<<fmt_hex(e->old_value)<<"->"<<fmt_hex(e->new_value)<<" pc="<<fmt_hex(e->pc);return o.str();
                    }
                    if(c0=="target"){
                        if(a.size()<2 || lower(a[1])!="one-hit") return "ERR target expects one-hit status|on|off";
                        const auto op=a.size()>2?lower(a[2]):"status";
                        if(op=="status"||op=="get") { o<<"OK target_one_hit="<<(runtime->target_one_hit()?1:0)<<" armed="<<runtime->target_one_hit_arms()<<" counter="<<fmt_hex(runtime->bus.peek(0x1002ae,2,chq::BusSpace::Main)); return o.str(); }
                        if(op=="on"||op=="1"||op=="true") { runtime->set_target_one_hit(true); return "OK target_one_hit=1"; }
                        if(op=="off"||op=="0"||op=="false") { runtime->set_target_one_hit(false); return "OK target_one_hit=0"; }
                        return "ERR target one-hit expects status|on|off";
                    }
                    if(c0=="collision"){
                        const auto op=a.size()>1?lower(a[1]):"state";
                        if(op=="state"||op=="status"||op=="get"){
                            o<<"OK response_enabled="<<(runtime->bus.suppress_collision_shove()?0:1)
                             <<" suppression_enabled="<<(runtime->bus.suppress_collision_shove()?1:0)
                             <<" suppressed_total="<<runtime->bus.suppressed_collision_shoves()
                             <<" suppressed_lateral="<<runtime->bus.suppressed_collision_lateral()
                             <<" suppressed_speed="<<runtime->bus.suppressed_collision_speed()
                             <<" scope=proven_response_writes detection_preserved=1";return o.str();
                        }
                        if(op=="response"){
                            const auto sub=a.size()>2?lower(a[2]):"get";
                            if(sub=="get"||sub=="status"){
                                o<<"OK enabled="<<(runtime->bus.suppress_collision_shove()?0:1)
                                 <<" suppression="<<(runtime->bus.suppress_collision_shove()?1:0)
                                 <<" suppressed_total="<<runtime->bus.suppressed_collision_shoves();return o.str();
                            }
                            if(sub=="set"){
                                if(a.size()<4)return "ERR collision response set 0|1";
                                const auto v=lower(a[3]); const bool enabled=(v=="1"||v=="true"||v=="on"||v=="enabled");
                                const bool disabled=(v=="0"||v=="false"||v=="off"||v=="disabled");
                                if(!enabled&&!disabled)return "ERR collision response set 0|1";
                                runtime->bus.set_suppress_collision_shove(!enabled);
                                return std::string("OK collision response enabled=")+(enabled?"1":"0")+" detection_preserved=1";
                            }
                            if(sub=="reset"||sub=="restore") {runtime->bus.set_suppress_collision_shove(false);return "OK collision response enabled=1 detection_preserved=1";}
                            return "ERR collision response expects get|set|reset";
                        }
                        if(op=="reset"||op=="restore"){runtime->bus.set_suppress_collision_shove(false);return "OK collision response enabled=1 detection_preserved=1";}
                        return "ERR collision expects state|response|reset";
                    }
                    if(c0=="handling"||c0=="tuning"){
                        const auto op=a.size()>1?lower(a[1]):"get";
                        if(op=="get"||op=="status"){const auto& h=runtime->handling_snapshot();o<<"OK cornering_scale="<<runtime->handling_cornering_scale()<<" speed_retain="<<runtime->handling_speed_retain()<<" override="<<((std::abs(runtime->handling_cornering_scale()-1.0)>1e-12||std::abs(runtime->handling_speed_retain()-1.0)>1e-12)?1:0)<<" valid="<<(h.valid?1:0)<<" turn="<<h.turn_state<<" table_index="<<h.table_index<<" speed="<<h.speed_internal<<" lateral_coeff="<<h.lateral_coeff<<" applied_lateral="<<h.applied_lateral_coeff;return o.str();}
                        if(op=="reset"){runtime->configure_handling_override(1.0,1.0);return "OK cornering_scale=1 speed_retain=1 override=0";}
                        if(op=="set"){if(a.size()<3)return "ERR handling set CORNERING [SPEED_RETAIN]";double cs=0,sr=runtime->handling_speed_retain();try{cs=std::stod(a[2]);if(a.size()>3)sr=std::stod(a[3]);}catch(...){return "ERR invalid handling value";}if(cs<0.0||cs>4.0||sr<0.0||sr>2.0)return "ERR handling range cornering=0..4 speed_retain=0..2";runtime->configure_handling_override(cs,sr);o<<"OK cornering_scale="<<cs<<" speed_retain="<<sr<<" override="<<((std::abs(cs-1.0)>1e-12||std::abs(sr-1.0)>1e-12)?1:0);return o.str();}
                        return "ERR handling expects get|set|reset";
                    }
                    if(c0=="palette"){
                        const auto op=a.size()>1?lower(a[1]):"status";
                        if(op=="status"){o<<"OK entries=4096 overrides="<<live_palette_overrides.size();return o.str();}
                        if(op=="bank"){if(a.size()<3)return "ERR palette bank BANK";std::uint32_t bank;if(!parse_api_u32(a[2],bank)||bank>255)return "ERR palette bank 0..255";o<<"OK bank="<<bank;for(unsigned pen=0;pen<16;++pen){auto idx=(bank<<4)|pen;o<<" pen"<<pen<<"="<<fmt_hex(runtime->bus.palette[idx]);}return o.str();}
                        if(op=="get"){if(a.size()<3)return "ERR palette get INDEX";std::uint32_t idx;if(!parse_api_u32(a[2],idx)||idx>4095)return "ERR palette index 0..4095";o<<"OK index="<<idx<<" bank="<<(idx>>4)<<" pen="<<(idx&15)<<" raw="<<fmt_hex(runtime->bus.palette[idx])<<" frozen="<<(live_palette_overrides.count((std::uint16_t)idx)?1:0);return o.str();}
                        if(op=="freeze"||op=="set"){if(a.size()<4)return "ERR palette freeze INDEX RAW16";std::uint32_t idx,val;if(!parse_api_u32(a[2],idx)||idx>4095||!parse_api_hex_u32(a[3],val)||val>0xffff)return "ERR bad palette arguments";const auto key=(std::uint16_t)idx;if(!live_palette_overrides.count(key))live_palette_originals[key]=runtime->bus.palette[idx];live_palette_overrides[key]=(std::uint16_t)val;runtime->bus.palette[idx]=(std::uint16_t)val;o<<"OK index="<<idx<<" raw="<<fmt_hex(val)<<" original="<<fmt_hex(live_palette_originals[key])<<" frozen=1";return o.str();}
                        if(op=="unfreeze"||op=="restore"){if(a.size()<3)return "ERR palette restore INDEX";std::uint32_t idx;if(!parse_api_u32(a[2],idx)||idx>4095)return "ERR palette index 0..4095";const auto key=(std::uint16_t)idx;auto it=live_palette_originals.find(key);if(it!=live_palette_originals.end())runtime->bus.palette[idx]=it->second;live_palette_overrides.erase(key);live_palette_originals.erase(key);return "OK palette entry restored index="+std::to_string(idx)+" raw="+fmt_hex(runtime->bus.palette[idx]);}
                        if(op=="clear"){for(const auto& q:live_palette_originals)runtime->bus.palette[q.first]=q.second;live_palette_overrides.clear();live_palette_originals.clear();return "OK palette overrides cleared; saved original values restored";}
                        if(op=="trace"){
                            const auto sub=a.size()>2?lower(a[2]):"status";
                            if(sub=="status"){
                                std::vector<unsigned> selected; const auto& f=runtime->bus.palette_trace_filter();
                                for(unsigned i=0;i<f.size();++i)if(f[i])selected.push_back(i);
                                o<<"OK enabled="<<(runtime->bus.palette_trace_enabled()?1:0)<<" events="<<runtime->bus.palette_trace_events().size()<<" limit="<<runtime->bus.palette_trace_limit()<<" indices=";
                                for(std::size_t i=0;i<selected.size();++i){if(i)o<<',';o<<selected[i];} return o.str();
                            }
                            if(sub=="clear"){runtime->bus.palette_trace_clear();return "OK palette trace events cleared";}
                            if(sub=="stop"){runtime->bus.palette_trace_stop();return "OK palette trace stopped events="+std::to_string(runtime->bus.palette_trace_events().size());}
                            if(sub=="start"){
                                if(a.size()<4)return "ERR palette trace start INDEX[,INDEX...] [LIMIT]";
                                std::vector<std::uint16_t> indices; std::stringstream ss(a[3]); std::string tok;
                                while(std::getline(ss,tok,',')){std::uint32_t idx=0;if(!parse_api_u32(tok,idx)||idx>4095)return "ERR palette trace index 0..4095";indices.push_back(static_cast<std::uint16_t>(idx));}
                                if(indices.empty())return "ERR palette trace requires at least one index"; std::uint32_t lim=4096;
                                if(a.size()>4&&(!parse_api_u32(a[4],lim)||lim<64||lim>1000000))return "ERR palette trace limit 64..1000000";
                                runtime->bus.palette_trace_clear(); runtime->bus.palette_trace_start(indices,lim); runtime->bus.research_enable(true);
                                o<<"OK palette trace started indices="<<a[3]<<" limit="<<lim;return o.str();
                            }
                            if(sub=="tail"){
                                std::uint32_t n=20;if(a.size()>3&&!parse_api_u32(a[3],n))return "ERR bad count";const auto&e=runtime->bus.palette_trace_events();const auto start=e.size()>n?e.size()-n:0;
                                o<<"OK palette_events="<<(e.size()-start);for(std::size_t i=start;i<e.size();++i){const auto&x=e[i];o<<"\nframe="<<x.frame<<" CPU="<<(x.space==chq::BusSpace::Main?'A':'B')<<" pc="<<fmt_hex(x.pc)<<" index="<<x.index<<" bank="<<(x.index>>4)<<" pen="<<(x.index&15)<<" old="<<fmt_hex(x.old_value)<<" new="<<fmt_hex(x.new_value)<<" changed="<<(x.changed?1:0)<<" write_count="<<x.write_count;}return o.str();
                            }
                            return "ERR palette trace expects start|status|tail|stop|clear";
                        }
                        return "ERR palette expects status|bank|get|freeze|restore|clear|trace";
                    }
                    if(c0=="input"){
                        if(a.size()<2)return "ERR input steering|xor|pulse|ports ...";const auto op=lower(a[1]);
                        if(op=="steering"){if(a.size()==2||lower(a[2])=="get"){o<<"OK steering="<<fmt_hex(runtime->bus.ioc_steering());return o.str();}std::size_t vi=lower(a[2])=="set"?3:2;if(a.size()<=vi)return "ERR input steering set VALUE";std::uint32_t v;if(!parse_api_hex_u32(a[vi],v)||v>0xffff)return "ERR bad steering value";runtime->bus.set_ioc_steering(static_cast<std::uint16_t>(v));return "OK steering="+fmt_hex(v);}
                        if(op=="ports"){o<<"OK";for(unsigned i=0;i<16;++i){const auto p=static_cast<std::uint8_t>(i);o<<" port"<<i<<"="<<fmt_hex(runtime->bus.ioc_port_value(p))<<" xor="<<fmt_hex(runtime->bus.ioc_input_xor_mask(p))<<" scenario="<<fmt_hex(runtime->bus.ioc_scenario_xor_mask(p))<<" effective_xor="<<fmt_hex(runtime->bus.ioc_effective_xor_mask(p))<<" reads="<<runtime->bus.ioc_read_count(p);}return o.str();}
                        if(op=="xor"){if(a.size()<3)return "ERR input xor get|set|clear ...";const auto sub=lower(a[2]);if(sub=="clear"){runtime->bus.clear_ioc_input_overrides();live_ioc_pulses.clear();return "OK input xor overrides cleared";}if(a.size()<4)return "ERR input xor get PORT | set PORT MASK";std::uint32_t port;if(!parse_api_u32(a[3],port)||port>15)return "ERR bad IOC port";if(sub=="get"){return "OK port="+std::to_string(port)+" xor="+fmt_hex(runtime->bus.ioc_input_xor_mask(static_cast<std::uint8_t>(port)));}if(sub=="set"){if(a.size()<5)return "ERR input xor set PORT MASK";std::uint32_t mask;if(!parse_api_hex_u32(a[4],mask)||mask>0xff)return "ERR bad xor mask";runtime->bus.set_ioc_input_xor_mask(static_cast<std::uint8_t>(port),static_cast<std::uint8_t>(mask));return "OK port="+std::to_string(port)+" xor="+fmt_hex(mask);}return "ERR input xor expects get|set|clear";}
                        if(op=="pulse"){if(a.size()<5)return "ERR input pulse PORT MASK FRAMES";std::uint32_t port,mask,n;if(!parse_api_u32(a[2],port)||port>15||!parse_api_hex_u32(a[3],mask)||mask>0xff||!parse_api_u32(a[4],n)||n<1)return "ERR bad pulse arguments";const auto pp=static_cast<std::uint8_t>(port);const auto prior=runtime->bus.ioc_input_xor_mask(pp);runtime->bus.set_ioc_input_xor_mask(pp,static_cast<std::uint8_t>(prior^mask));live_ioc_pulses.push_back({pp,static_cast<std::uint8_t>(mask),prior,frame+static_cast<unsigned>(n)});o<<"OK pulse port="<<port<<" mask="<<fmt_hex(mask)<<" start="<<frame<<" end="<<(frame+n);return o.str();}
                        return "ERR input expects steering|xor|pulse|ports";
                    }
                    if(c0=="sprites"){
                        if(!need(2))return o.str();const auto op=lower(a[1]);
                        if(op=="tail"){std::uint32_t n=20;if(a.size()>2&&!parse_api_u32(a[2],n))return "ERR bad count";bool semantic=false;for(const auto&q:a)if(lower(q)=="semantic")semantic=true;std::vector<const chq::SpriteChangeEvent*> v;for(const auto&e:video.sprite_change_events())if(!semantic||e.semantic)v.push_back(&e);const auto start=v.size()>n?v.size()-n:0;o<<"OK sprite_events="<<(v.size()-start);for(std::size_t i=start;i<v.size();++i){auto&e=*v[i];o<<"\nframe="<<e.frame<<" slot="<<e.slot<<" semantic="<<e.semantic<<" map="<<e.old_map<<"->"<<e.new_map<<" pal="<<unsigned(e.old_palette)<<"->"<<unsigned(e.new_palette)<<" pos="<<e.old_x<<','<<e.old_y<<"->"<<e.new_x<<','<<e.new_y<<" writer_pc="<<fmt_hex(e.writer_pc);}return o.str();}
                        if(op=="save"){if(a.size()<3)return "ERR sprites save PATH";std::ofstream f(a[2]);if(!f)return "ERR could not open output";f<<"frame,slot,semantic,old_map,new_map,old_palette,new_palette,old_x,old_y,new_x,new_y,old_visible,new_visible,writer_pc\n";for(const auto&e:video.sprite_change_events())f<<e.frame<<','<<e.slot<<','<<(e.semantic?1:0)<<','<<e.old_map<<','<<e.new_map<<','<<unsigned(e.old_palette)<<','<<unsigned(e.new_palette)<<','<<e.old_x<<','<<e.old_y<<','<<e.new_x<<','<<e.new_y<<','<<(e.old_visible?1:0)<<','<<(e.new_visible?1:0)<<",0x"<<std::hex<<e.writer_pc<<std::dec<<'\n';return "OK sprites saved "+a[2];}
                        if(op=="list"){
                            video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
                            const auto& v=video.debug_sprites();o<<"OK sprites="<<v.size();
                            for(const auto& q:v)o<<"\nslot="<<q.slot<<" map="<<q.map<<" palette="<<unsigned(q.palette)<<" x="<<q.x<<" y="<<q.y<<" width="<<q.width<<" height="<<q.height<<" priority="<<q.priority<<" visible_pixels="<<q.visible_pixels<<" visible_left="<<q.visible_left<<" visible_top="<<q.visible_top<<" visible_right="<<q.visible_right<<" visible_bottom="<<q.visible_bottom;
                            return o.str();
                        }
                        if(op=="inspect"){
                            if(a.size()<3)return "ERR sprites inspect SLOT";std::uint32_t slot=0;if(!parse_api_u32(a[2],slot))return "ERR bad slot";
                            video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
                            for(const auto& q:video.debug_sprites())if(q.slot==slot){o<<"OK slot="<<q.slot<<" map="<<q.map<<" palette="<<unsigned(q.palette)<<" x="<<q.x<<" y="<<q.y<<" width="<<q.width<<" height="<<q.height<<" priority="<<q.priority<<" visible_pixels="<<q.visible_pixels<<" visible_left="<<q.visible_left<<" visible_top="<<q.visible_top<<" visible_right="<<q.visible_right<<" visible_bottom="<<q.visible_bottom;return o.str();}
                            return "ERR sprite slot not active";
                        }
                        if(op=="order"){
                            video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
                            const bool higher=video.sprite_tie_break_higher();
                            o<<"OK tie_break="<<(higher?"higher-slot":"lower-slot")<<" traversal="<<(higher?"ascending":"descending")<<" ownership=first-nontransparent occupancy=includes-layer-blocked active="<<video.debug_sprites().size();
                            std::vector<const chq::DebugSpriteInfo*> ordered;ordered.reserve(video.debug_sprites().size());
                            for(const auto& q:video.debug_sprites())ordered.push_back(&q);
                            std::sort(ordered.begin(),ordered.end(),[&](auto* l,auto* r){return higher?l->slot<r->slot:l->slot>r->slot;});
                            unsigned draw=0;for(const auto* q:ordered)o<<"\ndraw="<<draw++<<" slot="<<q->slot<<" map="<<q->map<<" palette="<<unsigned(q->palette)<<" priority="<<q->priority<<" x="<<q->x<<" y="<<q->y<<" visible_pixels="<<q->visible_pixels;
                            return o.str();
                        }
                        if(op=="tie-break"||op=="tie_break"){
                            if(a.size()<3)return std::string("OK tie_break=")+(video.sprite_tie_break_higher()?"higher-slot":"lower-slot");
                            const auto mode=lower(a[2]);if(mode!="higher-slot"&&mode!="lower-slot")return "ERR sprites tie-break expects higher-slot|lower-slot";
                            video.set_sprite_tie_break_higher(mode=="higher-slot");changed=true;
                            return std::string("OK tie_break=")+(video.sprite_tie_break_higher()?"higher-slot":"lower-slot");
                        }
                        if(op=="map-inspect"){
                            if(a.size()<3)return "ERR sprites map-inspect SLOT";std::uint32_t slot=0;if(!parse_api_u32(a[2],slot))return "ERR bad slot";
                            const auto* sr=find_region_named(runtime->bus,"sprites"); if(!sr)return "ERR sprite RAM unavailable"; const std::size_t off=(std::size_t)slot*8u; if(off+7>=sr->bytes.size())return "ERR slot out of range";
                            const auto w0=be16_at(sr->bytes,off),w1=be16_at(sr->bytes,off+2),w2=be16_at(sr->bytes,off+4),w3=be16_at(sr->bytes,off+6);
                            const int zx=(w1&0x7f)+1; auto fmt=chq::Machine::format_for_zoomx(zx); int cols=0; std::size_t base=0; const std::vector<std::uint8_t>* gfx=nullptr; const char* gfxname="?";
                            if(fmt==chq::SpriteFormat::Obj128x128){cols=8;base=(std::size_t)(w3&0x7ff)<<6;gfx=&machine.sprites_a();gfxname="A";}
                            else if(fmt==chq::SpriteFormat::Obj64x128){cols=4;base=((std::size_t)(w3&0x7ff)<<5)+0x20000;gfx=&machine.sprites_b();gfxname="B";}
                            else if(fmt==chq::SpriteFormat::Obj32x128){cols=2;base=((std::size_t)(w3&0x7ff)<<4)+0x30000;gfx=&machine.sprites_b();gfxname="B";} else return "ERR unsupported sprite format";
                            std::array<std::uint64_t,16> hist{}; std::ostringstream chunks; const auto tiles=(gfx->size()*8)/1024;
                            for(int my=0;my<8;++my)for(int mx=0;mx<cols;++mx){auto mi=base+(std::size_t)(mx+my*cols);auto raw=machine.spritemap_word(mi);auto code=static_cast<std::uint16_t>(raw&0x3fff);chunks<<" chunk"<<(my*cols+mx)<<"="<<fmt_hex(mi*2)<<":"<<fmt_hex(raw)<<":"<<fmt_hex(code);if(code<tiles)for(int py=0;py<16;++py)for(int px=0;px<16;++px)++hist[export_sprite_pixel(*gfx,code,px,py)];}
                            o<<"OK slot="<<slot<<" map="<<(w3&0x7ff)<<" palette="<<unsigned((w1&0x7f80)>>7)<<" gfx="<<gfxname<<" cols="<<cols<<" rows=8 spritemap_word_base="<<fmt_hex(base)<<" spritemap_byte_base="<<fmt_hex(base*2)<<" tile_count="<<tiles;for(unsigned pen=0;pen<16;++pen)o<<" pen"<<pen<<"="<<hist[pen];o<<chunks.str();return o.str();
                        }
                        if(op=="override"){
                            if(a.size()<6)return "ERR sprites override SLOT MAP PAL VISIBLE (-1 keeps field)";std::uint32_t slot=0;if(!parse_api_u32(a[2],slot))return "ERR bad slot";
                            int map=std::stoi(a[3]),pal=std::stoi(a[4]),vis=std::stoi(a[5]);video.set_runtime_sprite_override(slot,map,pal,vis);changed=true;return "OK sprite override slot="+std::to_string(slot)+" map="+std::to_string(map)+" palette="+std::to_string(pal)+" visible="+std::to_string(vis);
                        }
                        if(op=="override-clear"||op=="clear-overrides"){video.clear_runtime_sprite_overrides();changed=true;return "OK sprite overrides cleared";}
                        if(op=="visual"){
                            if(a.size()<4)return "ERR sprites visual SLOT solo|hide|flash";std::uint32_t slot=0;if(!parse_api_u32(a[2],slot))return "ERR bad slot";auto mode=lower(a[3]);if(mode!="solo"&&mode!="hide"&&mode!="flash")return "ERR visual mode must be solo|hide|flash";video.set_runtime_sprite_visual(slot,mode);changed=true;return "OK sprite visual slot="+std::to_string(slot)+" mode="+mode;
                        }
                        if(op=="visual-clear"){video.clear_runtime_sprite_visual();changed=true;return "OK sprite visual cleared";}
                        return "ERR sprites expects tail|save|list|inspect|order|tie-break|map-inspect|override|override-clear|visual|visual-clear";
                    }
                    if(c0=="prompt"){
                        if(a.size()<2)return "ERR prompt show|status|clear [MESSAGE]"; const auto op=lower(a[1]);
                        if(op=="show"){std::string msg;for(std::size_t i=2;i<a.size();++i){if(i>2)msg+=' ';msg+=a[i];}if(msg.empty())msg="Continue when ready";video.show_script_prompt(msg);changed=true;return "OK prompt shown";}
                        if(op=="status"){o<<"OK active="<<(video.script_prompt_active()?1:0)<<" accepted="<<(video.script_prompt_accepted()?1:0)<<" cancelled="<<(video.script_prompt_cancelled()?1:0);return o.str();}
                        if(op=="clear"){video.clear_script_prompt();changed=true;return "OK prompt cleared";}
                        return "ERR prompt expects show|status|clear";
                    }
                    if(c0=="timeline"){
                        const auto op=a.size()>1?lower(a[1]):"status";
                        if(op=="status"){o<<"OK recording="<<(forensic_timeline_recording?1:0)<<" loaded="<<(forensic_timeline_loaded?1:0)<<" source="<<(forensic_timeline_loaded?"timeline":"live")<<" frame="<<frame<<" frames="<<forensic_timeline_frames.size()<<" index="<<forensic_timeline_index<<" writes="<<forensic_timeline_writes<<" path="<<forensic_timeline_root.string();return o.str();}
                        if(op=="start"||op=="record"){if(a.size()<3)return "ERR timeline start PATH";forensic_start(a[2]);o<<"OK timeline recording=1 frame="<<frame<<" path="<<forensic_timeline_root.string();return o.str();}
                        if(op=="stop"){if(!forensic_timeline_recording)return "ERR timeline recording not active";forensic_stop();o<<"OK timeline recording=0 frames="<<forensic_timeline_frames.size()<<" start="<<forensic_timeline_start_frame<<" end="<<forensic_timeline_end_frame<<" writes="<<forensic_timeline_writes<<" path="<<forensic_timeline_root.string();return o.str();}
                        if(op=="load"){if(a.size()<3)return "ERR timeline load PATH";if(forensic_timeline_recording)return "ERR stop timeline recording first";if(forensic_timeline_loaded)return "ERR timeline already loaded; unload or fork first";const std::filesystem::path root=a[2];auto frames=forensic_scan_frames(root);if(frames.empty())return "ERR timeline has no frame checkpoints";forensic_timeline_return_state=options.logs/"timeline_return_state.chqstate";runtime->save_checkpoint(forensic_timeline_return_state,frame);forensic_timeline_root=root;forensic_timeline_frames=std::move(frames);forensic_timeline_loaded=true;forensic_timeline_index=0;frame=runtime->load_checkpoint(forensic_frame_path(forensic_timeline_frames[0]));debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline loaded=1 frame="<<frame<<" frames="<<forensic_timeline_frames.size()<<" path="<<forensic_timeline_root.string();return o.str();}
                        if(op=="seek"){if(a.size()<3)return "ERR timeline seek FRAME";std::uint32_t f=0;if(!parse_api_u32(a[2],f))return "ERR bad timeline frame";const auto got=forensic_seek(f);o<<"OK timeline frame="<<got<<" index="<<forensic_timeline_index<<" frames="<<forensic_timeline_frames.size();return o.str();}
                        if(op=="next"||op=="prev"){if(!forensic_timeline_loaded)return "ERR no timeline loaded";if(op=="next"&&forensic_timeline_index+1<forensic_timeline_frames.size())++forensic_timeline_index;else if(op=="prev"&&forensic_timeline_index>0)--forensic_timeline_index;const auto f=forensic_timeline_frames[forensic_timeline_index];frame=runtime->load_checkpoint(forensic_frame_path(f));debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline frame="<<frame<<" index="<<forensic_timeline_index<<" frames="<<forensic_timeline_frames.size();return o.str();}
                        if(op=="frames"){if(!forensic_timeline_loaded&&forensic_timeline_frames.empty())return "ERR no timeline loaded";o<<"OK";for(auto f:forensic_timeline_frames)o<<' '<<f;return o.str();}
                        if(op=="fork"){if(!forensic_timeline_loaded)return "ERR no timeline loaded";forensic_timeline_loaded=false;forensic_timeline_return_state.clear();debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline forked_live frame="<<frame;return o.str();}
                        if(op=="unload"){if(!forensic_timeline_loaded)return "ERR no timeline loaded";if(forensic_timeline_return_state.empty()||!std::filesystem::exists(forensic_timeline_return_state))return "ERR timeline return checkpoint missing";frame=runtime->load_checkpoint(forensic_timeline_return_state);forensic_timeline_loaded=false;debug_paused=true;debug_step_remaining=0;changed=true;o<<"OK timeline unloaded frame="<<frame;return o.str();}
                        return "ERR timeline expects status|start|stop|load|seek|next|prev|frames|fork|unload";
                    }
                    if(c0=="checkpoint"){
                        if(a.size()<3)return "ERR checkpoint save|load PATH";const auto op=lower(a[1]);if(op=="save"){runtime->save_checkpoint(a[2],frame);return "OK checkpoint saved frame="+std::to_string(frame)+" path="+a[2];}if(op=="load"){frame=runtime->load_checkpoint(a[2]);debug_paused=true;debug_step_remaining=0;changed=true;return "OK checkpoint loaded frame="+std::to_string(frame)+" path="+a[2];}return "ERR checkpoint expects save|load";
                    }
                    if(c0=="screenshot"){std::filesystem::path p=a.size()>1?std::filesystem::path(a[1]):options.logs/(std::string("api_screenshot_f")+std::to_string(frame)+"_"+timestamp_name()+".png");video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);if(!video.save_current_screenshot(p))return "ERR screenshot failed";return "OK screenshot "+p.string();}
                    if(c0=="graphics") {
                        if(a.size()<2)return "ERR graphics snapshot PATH"; const auto op=lower(a[1]);
                        if(op=="snapshot") { if(a.size()<3)return "ERR graphics snapshot PATH"; std::filesystem::path p=a[2]; if(!video.save_layered_snapshot(p,machine,*runtime,scene,frame))return "ERR layered snapshot failed"; return "OK graphics_snapshot frame="+std::to_string(frame)+" path="+p.string(); }
                        return "ERR graphics expects snapshot";
                    }
                    if(c0=="research"){if(a.size()<2)return "ERR research on|off";const auto op=lower(a[1]);if(op=="on"){runtime->bus.research_enable(true);return "OK research enabled";}if(op=="off"){runtime->bus.research_enable(false);return "OK research disabled";}return "ERR research on|off";}
                    return "ERR unknown command; use help";
                });
            }
            if(runtime && !live_palette_overrides.empty())
                for(const auto& q:live_palette_overrides) runtime->bus.palette[q.first]=q.second;

            if (runtime) {
                if (ui_actions.workbench_opened) {
                    runtime->bus.research_enable(true);
                    runtime->bus.research_clear_events();
                    bool have_sprite_watch=false;
                    for(const auto& w:runtime->bus.research_watches()) if(w.space==chq::BusSpace::Main && w.start==0xd00000u && w.end==0xd007ffu) have_sprite_watch=true;
                    if(!have_sprite_watch){ chq::ResearchWatch w{}; w.space=chq::BusSpace::Main; w.start=0xd00000u; w.end=0xd007ffu; w.writes=true; runtime->bus.research_add_watch(w); }
                    debug_paused=true; debug_step_remaining=0;
                    std::filesystem::create_directories(workbench_dir); runtime->save_checkpoint(workbench_pause_point,frame); workbench_have_pause_point=true;
                    std::cout<<console_stamp(process_start)<<"[WORKBENCH] opened + paused frame="<<frame<<" baseline="<<workbench_pause_point.string()<<"\n"; changed=true;
                }
                if (ui_actions.workbench_closed) std::cout<<console_stamp(process_start)<<"[WORKBENCH] closed frame="<<frame<<" (research capture remains armed)\n";
                if (ui_actions.toggle_pause) {
                    debug_paused=!debug_paused;
                    if(debug_paused && video.research_workbench_visible()){ std::filesystem::create_directories(workbench_dir); runtime->save_checkpoint(workbench_pause_point,frame); workbench_have_pause_point=true; }
                    std::cout<<console_stamp(process_start)<<"[DEBUG UI] "<<(debug_paused?"paused":"running")<<" frame="<<frame<<"\n";
                }
                if (ui_actions.snapshot_pause_point) { debug_paused=true; debug_step_remaining=0; std::filesystem::create_directories(workbench_dir); runtime->save_checkpoint(workbench_pause_point,frame); workbench_have_pause_point=true; std::cout<<console_stamp(process_start)<<"[WORKBENCH] pause point saved frame="<<frame<<"\n"; changed=true; }
                if (ui_actions.restore_pause_point) { if(workbench_have_pause_point){ runtime->bus.research_clear_patches(); frame=runtime->load_checkpoint(workbench_pause_point); debug_paused=true; debug_step_remaining=0; std::cout<<console_stamp(process_start)<<"[WORKBENCH] restored baseline frame="<<frame<<"\n"; changed=true; } else std::cout<<console_stamp(process_start)<<"[WORKBENCH] no pause baseline yet\n"; }
                if (ui_actions.step_instruction_a || ui_actions.step_instruction_b) { debug_paused=true; debug_step_remaining=0; const auto sp=ui_actions.step_instruction_b?chq::BusSpace::Sub:chq::BusSpace::Main; const int used=runtime->step_instruction(sp); std::cout<<console_stamp(process_start)<<"[WORKBENCH] single instruction CPU "<<(sp==chq::BusSpace::Main?'A':'B')<<" cycles="<<used<<" frame="<<frame<<"\n"; changed=true; }
                if (ui_actions.clear_research_patches) { runtime->bus.research_clear_patches(); std::cout<<console_stamp(process_start)<<"[WORKBENCH] live patches cleared\n"; changed=true; }
                if (ui_actions.clear_research_watches) { runtime->bus.research_clear_watches(); std::cout<<console_stamp(process_start)<<"[WORKBENCH] watches cleared\n"; changed=true; }
                if (ui_actions.clear_research_events) { runtime->bus.research_clear_events(); std::cout<<console_stamp(process_start)<<"[WORKBENCH] event history cleared\n"; changed=true; }
                using RC=chq::DebugUiActions::ResearchCommand;
                if(ui_actions.research_command!=RC::None){
                    debug_paused=true; debug_step_remaining=0;
                    const auto oldv=runtime->bus.peek(ui_actions.research_address,ui_actions.research_size,ui_actions.research_space);
                    if(ui_actions.research_command==RC::WriteOnce) runtime->bus.debug_write(ui_actions.research_address,ui_actions.research_value,ui_actions.research_size,ui_actions.research_space);
                    else if(ui_actions.research_command==RC::Freeze){ runtime->bus.debug_write(ui_actions.research_address,ui_actions.research_value,ui_actions.research_size,ui_actions.research_space); runtime->bus.research_add_patch(ui_actions.research_space,ui_actions.research_address,ui_actions.research_size,ui_actions.research_value,chq::LivePatchMode::Freeze); }
                    else if(ui_actions.research_command==RC::Suppress) runtime->bus.research_add_patch(ui_actions.research_space,ui_actions.research_address,ui_actions.research_size,oldv,chq::LivePatchMode::Suppress);
                    else { chq::ResearchWatch w{}; w.space=ui_actions.research_space; w.start=ui_actions.research_address; w.end=ui_actions.research_address+ui_actions.research_size-1; w.writes=true; w.change_only=ui_actions.research_command==RC::WatchChange; w.break_on_match=true; runtime->bus.research_add_watch(w); }
                    std::cout<<console_stamp(process_start)<<"[WORKBENCH] mode="<<int(ui_actions.research_command)<<" cpu="<<(ui_actions.research_space==chq::BusSpace::Main?'A':'B')<<" addr=0x"<<std::hex<<ui_actions.research_address<<" old=0x"<<oldv<<" value=0x"<<ui_actions.research_value<<std::dec<<"\n"; changed=true;
                }
                if (ui_actions.step_frames) { debug_paused = true; debug_step_remaining += ui_actions.step_frames; }
                if (ui_actions.timer_preset_bcd >= 0) {
                    runtime->bus.write(0x100200, static_cast<std::uint32_t>(ui_actions.timer_preset_bcd), 1, chq::BusSpace::Main);
                    runtime->bus.write(0x100201, 0, 1, chq::BusSpace::Main);
                    if (timer_frozen) { frozen_timer_value = static_cast<std::uint8_t>(ui_actions.timer_preset_bcd); frozen_timer_fraction = 0; }
                    std::cout << console_stamp(process_start) << "[DEBUG UI] race_timer=0x" << std::hex << ui_actions.timer_preset_bcd << std::dec << " frame=" << frame << "\n"; changed=true;
                }
                if (ui_actions.timer_freeze_toggle) {
                    timer_frozen = !timer_frozen;
                    if (timer_frozen) { frozen_timer_value = static_cast<std::uint8_t>(runtime->bus.peek(0x100200,1,chq::BusSpace::Main)); frozen_timer_fraction = static_cast<std::uint8_t>(runtime->bus.peek(0x100201,1,chq::BusSpace::Main)); }
                    std::cout << console_stamp(process_start) << "[DEBUG UI] timer freeze " << (timer_frozen?"ON":"OFF") << " frame=" << frame << "\n"; changed=true;
                }
                if (ui_actions.save_checkpoint) { const auto state=ui_state_path(); runtime->save_checkpoint(state, frame); std::cout << console_stamp(process_start) << "[DEBUG UI] state slot "<<video.checkpoint_slot()<<" saved -> " << state.string() << "\n"; changed=true; }
                if (ui_actions.load_checkpoint) {
                    const auto state=ui_state_path(); std::error_code ec; if (std::filesystem::exists(state,ec)) { frame = runtime->load_checkpoint(state); std::cout << console_stamp(process_start) << "[DEBUG UI] state slot "<<video.checkpoint_slot()<<" loaded frame=" << frame << " <- "<<state.string()<<"\n"; changed=true; }
                    else std::cout << console_stamp(process_start) << "[DEBUG UI] no state in slot "<<video.checkpoint_slot()<<": " << state.string() << "\n";
                }
                if (ui_actions.capture_graphics) { video.add_diagnostics_frame(frame); video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen); std::cout << console_stamp(process_start) << "[DEBUG UI] graphics capture frame=" << frame << "\n"; changed=false; }
                if (ui_actions.screenshot) {
                    const auto shot=options.logs/(std::string("ui_screenshot_f")+std::to_string(frame)+"_"+timestamp_name()+".png");
                    if(video.save_current_screenshot(shot)) std::cout<<console_stamp(process_start)<<"[DEBUG UI] screenshot -> "<<shot.string()<<"\n";
                }
                if (ui_actions.export_bundle) {
                    video.add_diagnostics_frame(frame); video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
                    capture_hardware_state(*runtime, options, frame);
                    const auto stage=options.logs/(std::string("interactive_bundle_f")+std::to_string(frame)+"_"+timestamp_name()); std::filesystem::create_directories(stage);
                    std::ofstream ui(stage/"interactive_state.txt"); ui<<"build="<<chq::kNativeVersion<<"\nframe="<<frame<<"\npaused="<<debug_paused<<"\ntimer_frozen="<<timer_frozen<<"\nselected_sprite="<<video.selected_sprite()<<"\n"; ui.close();
                    if(runtime->bus.research_enabled()) {
                        std::ofstream re(stage/"research_events.csv"); re<<"frame,kind,cpu,pc,address,width,old_value,new_value,changed\n";
                        for(const auto& e:runtime->bus.research_events()){ const char* k=e.kind==chq::ResearchEventKind::Read?"READ":(e.kind==chq::ResearchEventKind::Write?"WRITE":(e.kind==chq::ResearchEventKind::Intervention?"INTERVENTION":"SUPPRESSED_WRITE")); re<<e.frame<<','<<k<<','<<(e.space==chq::BusSpace::Main?'A':'B')<<",0x"<<std::hex<<e.pc<<",0x"<<e.address<<std::dec<<','<<(e.size*8)<<",0x"<<std::hex<<e.old_value<<",0x"<<e.new_value<<std::dec<<','<<(e.changed?1:0)<<'\n'; }
                        std::ofstream rp(stage/"active_patches.csv"); rp<<"id,cpu,address,width,value,mode,interceptions,last_frame,last_pc\n";
                        for(const auto& q:runtime->bus.research_patches()) rp<<q.id<<','<<(q.space==chq::BusSpace::Main?'A':'B')<<",0x"<<std::hex<<q.address<<std::dec<<','<<(q.size*8)<<",0x"<<std::hex<<q.value<<std::dec<<','<<(q.mode==chq::LivePatchMode::Freeze?"FREEZE":q.mode==chq::LivePatchMode::Replace?"REPLACE":"SUPPRESS")<<','<<q.interceptions<<','<<q.last_frame<<",0x"<<std::hex<<q.last_pc<<std::dec<<'\n';
                        std::ofstream sw(stage/"sprite_changes.csv"); sw<<"frame,slot,semantic,old_map,new_map,old_palette,new_palette,old_x,old_y,new_x,new_y,old_visible,new_visible,writer_pc\n";
                        for(const auto& e:video.sprite_change_events()) sw<<e.frame<<','<<e.slot<<','<<(e.semantic?1:0)<<','<<e.old_map<<','<<e.new_map<<','<<unsigned(e.old_palette)<<','<<unsigned(e.new_palette)<<','<<e.old_x<<','<<e.old_y<<','<<e.new_x<<','<<e.new_y<<','<<(e.old_visible?1:0)<<','<<(e.new_visible?1:0)<<",0x"<<std::hex<<e.writer_pc<<std::dec<<'\n';
                    }
                    std::error_code ec; const auto cap=options.logs/(std::string("capture_frame_")+std::to_string(frame)); const auto gfx=options.logs/(std::string("graphics_frame_")+std::to_string(frame));
                    if(std::filesystem::exists(cap)) std::filesystem::copy(cap,stage/"hardware_state",std::filesystem::copy_options::recursive|std::filesystem::copy_options::overwrite_existing,ec); ec.clear();
                    if(std::filesystem::exists(gfx)) std::filesystem::copy(gfx,stage/"graphics",std::filesystem::copy_options::recursive|std::filesystem::copy_options::overwrite_existing,ec);
                    const auto zip=options.logs/(std::string("ChaseHQ_v")+chq::kNativeVersion+"_interactive_f"+std::to_string(frame)+"_"+timestamp_name()+".zip");
                    if(zip_directory_store(stage,zip)) std::cout<<console_stamp(process_start)<<"[DEBUG UI] diagnostic bundle -> "<<zip.string()<<"\n"; else std::cout<<console_stamp(process_start)<<"[DEBUG UI] bundle creation FAILED\n";
                    changed=false;
                }
            }
            if (fast_forwarding && options.fast_forward_status_every && (frame % options.fast_forward_status_every)==0) {
                const double pct = options.fast_forward_to ? (100.0 * double(frame) / double(options.fast_forward_to)) : 100.0;
                const double elapsed_s=std::chrono::duration<double>(std::chrono::steady_clock::now()-process_start).count();
                const double fps=(elapsed_s>0.0)?double(frame)/elapsed_s:0.0;
                const double eta=(fps>0.0)?double(options.fast_forward_to-frame)/fps:0.0;
                std::cout << console_stamp(process_start) << "[FAST-FORWARD] frame=" << frame << "/" << options.fast_forward_to
                          << " (" << std::fixed << std::setprecision(1) << pct << "%) rate="<<fps<<" f/s ETA="<<eta<<"s\n" << std::defaultfloat;
                // v0.58.2 quiet path: do not touch SDL/presentation while uncapped.
                // Console status remains available without forcing a present/update.
            }

            if (!fast_forwarding && bounded_run && options.progress_status_every && frame > run_start_frame &&
                (frame % options.progress_status_every)==0 && frame < effective_frames) {
                const double elapsed_s=std::chrono::duration<double>(std::chrono::steady_clock::now()-progress_start).count();
                const unsigned done=frame-run_start_frame;
                const unsigned total=effective_frames>run_start_frame ? effective_frames-run_start_frame : 0;
                const double fps=(elapsed_s>0.0)?double(done)/elapsed_s:0.0;
                const double pct=total?100.0*double(done)/double(total):100.0;
                const double eta=(fps>0.0)?double(effective_frames-frame)/fps:0.0;
                std::cout<<console_stamp(process_start)<<"[PROGRESS] frame="<<frame<<"/"<<effective_frames
                         <<" ("<<std::fixed<<std::setprecision(1)<<pct<<"%) elapsed="<<elapsed_s<<"s rate="<<fps<<" f/s ETA="<<eta<<"s\n"<<std::defaultfloat;
            }

            if (runtime && running && (!bounded_run || frame < effective_frames) && runtime->fault.empty() && (!debug_paused || debug_step_remaining > 0)) {
                runtime->set_trace_frame(frame);
                runtime->bus.set_ioc_debug_frame(frame);
                active_ioc_xor.fill(0);
                if (options.course_survey) active_ioc_xor[3] ^= 0x20; // hold genuine accelerator input
                if(options.steering_fixed) runtime->bus.set_ioc_steering(*options.steering_fixed);
                for(const auto& se:options.steering_events) if(frame>=se.start_frame && frame<=se.end_frame) runtime->bus.set_ioc_steering(se.value);
                course_follow_target = 0;
                course_follow_profile_feedforward = 0; course_follow_profile_delta = 0;
                course_follow_p_term = course_follow_d_term = course_follow_error_rate = 0;
                course_follow_speed_target = 390;
                course_follow_accel_cmd = true;
                course_follow_brake_cmd = false;
                if (options.course_follow) {
                    const auto cp = runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub);
                    const unsigned bank = runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub) & 0x0f;
                    const unsigned off = (cp & 0x3e000u) >> 10;
                    const auto ca = 0x109000u + bank*0x100u + (off & 0xf8u);
                    course_follow_curve_now = static_cast<std::int8_t>(runtime->bus.peek(ca,1,chq::BusSpace::Sub));
                    course_follow_curve_ahead = course_follow_curve_now;
                    int strongest = course_follow_curve_now;
                    int weighted_sum = course_follow_curve_now * (options.course_follow_lookahead + 1);
                    int weight_sum = options.course_follow_lookahead + 1;
                    int curve_severity = std::abs(course_follow_curve_now);
                    for(int la=1; la<=options.course_follow_lookahead; ++la){
                        const unsigned roff=(off & 0xf8u) + unsigned(la)*8u;
                        if(roff>=0x100u) break;
                        const int c=static_cast<std::int8_t>(runtime->bus.peek(0x109000u+bank*0x100u+roff,1,chq::BusSpace::Sub));
                        if(std::abs(c)>std::abs(strongest)) strongest=c;
                        curve_severity=std::max(curve_severity,std::abs(c));
                        const int w=std::max(1, options.course_follow_lookahead + 1 - la);
                        weighted_sum += c*w; weight_sum += w;
                        if(la==options.course_follow_lookahead) course_follow_curve_ahead=c;
                    }
                    course_follow_curve_predict = weight_sum ? int(std::lround(double(weighted_sum)/double(weight_sum))) : course_follow_curve_now;
                    const auto bounds=runtime->bus.peek(0x10a05c,4,chq::BusSpace::Main);
                    const std::uint16_t b0=static_cast<std::uint16_t>((bounds>>16)&0xffffu);
                    const std::uint16_t b1=static_cast<std::uint16_t>(bounds&0xffffu);
                    road_left_bound=b0; road_right_bound=b1;
                    const auto delta=static_cast<std::int16_t>(static_cast<std::uint16_t>(b1-b0));
                    road_centre=static_cast<std::uint16_t>(b0 + delta/2);
                    road_width=std::abs(int(delta));
                    car_lateral=static_cast<std::uint16_t>(runtime->bus.peek(0x10a044,2,chq::BusSpace::Main));
                    lateral_error=static_cast<std::int16_t>(static_cast<std::uint16_t>(car_lateral-road_centre));
                    lateral_error_norm = road_width>0 ? double(lateral_error)/(double(road_width)*0.5) : 0.0;
                    lateral_side=(lateral_error < -options.course_follow_lateral_deadzone)?"LEFT":((lateral_error > options.course_follow_lateral_deadzone)?"RIGHT":"CENTRE");
                    const auto rf_pre=static_cast<unsigned>(runtime->bus.peek(0x10a048,1,chq::BusSpace::Main));
                    const bool recovery=(rf_pre&0x04)||((rf_pre&0x06)==0x02);
                    if (options.course_follow_controller == "profile") {
                        const auto* pp=nearest_profile_point(course_profile,cp);
                        course_follow_profile_feedforward=pp?pp->steering:0;
                        course_follow_profile_delta=pp?std::int64_t(cp)-std::int64_t(pp->course_position):0;
                        course_follow_feedforward=course_follow_profile_feedforward;
                        course_follow_error_rate=course_follow_have_prev_error ? int(lateral_error)-int(course_follow_prev_error) : 0;
                        course_follow_prev_error=lateral_error; course_follow_have_prev_error=true;
                        const bool hard_recovery=recovery || std::abs(lateral_error_norm)>0.72 || std::abs(int(lateral_error))>1800;
                        course_follow_p_term=0;
                        if(std::abs(int(lateral_error))>options.course_follow_lateral_deadzone)
                            course_follow_p_term=static_cast<int>(std::lround(-double(lateral_error)*options.course_follow_lateral_kp*(hard_recovery?2.20:1.0)));
                        course_follow_d_term=hard_recovery?0:static_cast<int>(std::lround(-double(course_follow_error_rate)*options.course_follow_lateral_kd));
                        course_follow_lateral_correction=std::clamp(course_follow_p_term+course_follow_d_term,hard_recovery?-96:-options.course_follow_lateral_max,hard_recovery?96:options.course_follow_lateral_max);
                        const int desired=std::clamp((hard_recovery?0:course_follow_feedforward)+course_follow_lateral_correction,-96,96);
                        course_follow_output=hard_recovery?desired:std::clamp(desired,course_follow_output-options.course_follow_slew,course_follow_output+options.course_follow_slew);
                        course_follow_target=course_follow_output;
                        course_follow_mode=hard_recovery?"PROFILE_HARD_RECOVERY":"PROFILE_CENTRE_CORRECT";
                        if(pp){ course_follow_accel_cmd=pp->accel; course_follow_brake_cmd=pp->brake; course_follow_speed_target=pp->speed_target; }
                    } else if (options.course_follow_controller == "legacy") {
                        course_follow_feedforward=0;
                        if (strongest > options.course_follow_deadzone) course_follow_feedforward=options.course_follow_steer;
                        else if (strongest < -options.course_follow_deadzone) course_follow_feedforward=-options.course_follow_steer;
                        course_follow_lateral_correction=0;
                        if(std::abs(int(lateral_error))>options.course_follow_lateral_deadzone){
                            course_follow_lateral_correction=static_cast<int>(std::lround(-double(lateral_error)*options.course_follow_lateral_kp));
                            course_follow_lateral_correction=std::clamp(course_follow_lateral_correction,-options.course_follow_lateral_max,options.course_follow_lateral_max);
                        }
                        course_follow_target=std::clamp(course_follow_feedforward+course_follow_lateral_correction,-96,96);
                        course_follow_output=course_follow_target;
                        course_follow_mode=recovery?"LEGACY_RECOVERY":(course_follow_lateral_correction?"LEGACY_CLOSED_LOOP":"LEGACY_FEED_FORWARD");
                    } else {
                        const auto speed_bcd=runtime->bus.peek(0x100400,2,chq::BusSpace::Main);
                        const int speed_kmh=int(((speed_bcd>>12)&15)*1000+((speed_bcd>>8)&15)*100+((speed_bcd>>4)&15)*10+(speed_bcd&15));
                        const double speed_scale=std::clamp(1.15-double(speed_kmh)/1000.0,0.72,1.05);
                        course_follow_error_rate=course_follow_have_prev_error ? int(lateral_error)-int(course_follow_prev_error) : 0;
                        course_follow_prev_error=lateral_error; course_follow_have_prev_error=true;
                        course_follow_feedforward=0;
                        if(std::abs(course_follow_curve_predict)>options.course_follow_deadzone){
                            course_follow_feedforward=static_cast<int>(std::lround(double(course_follow_curve_predict)*double(options.course_follow_steer)/48.0));
                            course_follow_feedforward=std::clamp(course_follow_feedforward,-options.course_follow_steer,options.course_follow_steer);
                        }
                        const bool hybrid = options.course_follow_controller == "hybrid";
                        const bool hard_recovery = hybrid && (recovery || std::abs(lateral_error_norm) > 0.72 || std::abs(int(lateral_error)) > 1800);
                        course_follow_p_term=0;
                        if(std::abs(int(lateral_error))>options.course_follow_lateral_deadzone)
                            course_follow_p_term=static_cast<int>(std::lround(-double(lateral_error)*options.course_follow_lateral_kp*speed_scale*(hard_recovery?2.20:(recovery?1.35:1.0))));
                        course_follow_d_term=hard_recovery ? 0 : static_cast<int>(std::lround(-double(course_follow_error_rate)*options.course_follow_lateral_kd));
                        const int correction_limit = hard_recovery ? 96 : options.course_follow_lateral_max;
                        course_follow_lateral_correction=std::clamp(course_follow_p_term+course_follow_d_term,-correction_limit,correction_limit);
                        const int desired=std::clamp((hard_recovery?0:course_follow_feedforward)+course_follow_lateral_correction,-96,96);
                        if(hard_recovery) course_follow_output=desired;
                        else course_follow_output=std::clamp(desired,course_follow_output-options.course_follow_slew,course_follow_output+options.course_follow_slew);
                        course_follow_target=course_follow_output;
                        course_follow_mode=hard_recovery?"HYBRID_HARD_RECOVERY":(hybrid?(course_follow_d_term?"HYBRID_PD":"HYBRID_FEED_FORWARD"):(recovery?"PREDICTIVE_RECOVERY":(course_follow_d_term?"PREDICTIVE_PD":"PREDICTIVE")));
                        if(options.course_follow_speed_control){
                            if(curve_severity<=8) course_follow_speed_target=390;
                            else if(curve_severity<=16) course_follow_speed_target=360;
                            else if(curve_severity<=28) course_follow_speed_target=325;
                            else if(curve_severity<=40) course_follow_speed_target=285;
                            else course_follow_speed_target=245;
                            course_follow_accel_cmd = speed_kmh <= course_follow_speed_target + 8;
                            course_follow_brake_cmd = speed_kmh > course_follow_speed_target + 35;
                        }
                    }
                    if(course_follow_accel_cmd) active_ioc_xor[3] |= 0x20; else active_ioc_xor[3] &= static_cast<std::uint8_t>(~0x20u);
                    if(course_follow_brake_cmd) active_ioc_xor[2] |= 0x20; else active_ioc_xor[2] &= static_cast<std::uint8_t>(~0x20u);
                    runtime->bus.set_ioc_steering(static_cast<std::uint16_t>(course_follow_target) & 0x0fff);
                }
                if (options.infinite_time) { runtime->bus.write(0x100200,survey_timer_bcd,1,chq::BusSpace::Main); runtime->bus.write(0x100201,0,1,chq::BusSpace::Main); }
                if (options.unlimited_turbo) runtime->bus.write(0x1003a2,3,2,chq::BusSpace::Main);
                if (options.auto_turbo && ((frame-run_start_frame)%220u)<3u) active_ioc_xor[3] ^= 0x01;
                for (std::size_t i = 0; i < options.input_pulses.size(); ++i) {
                    const auto& pulse = options.input_pulses[i];
                    auto& state = pulse_state[i];
                    const auto port = static_cast<std::uint8_t>(pulse.port & 0x0f);
                    if (!state.armed && frame >= pulse.start_frame) {
                        state.armed = true;
                        state.active_from = frame;
                        state.reads_before = runtime->bus.ioc_read_count(port);
                        std::cout << console_stamp(process_start) << "[IOC PULSE ARMED] frame=" << std::dec << frame
                                  << " port=0x" << std::hex << static_cast<unsigned>(port)
                                  << " xor=0x" << static_cast<unsigned>(pulse.mask)
                                  << std::dec << " duration=" << pulse.duration_frames << "\n";
                    }
                    if (state.armed && !state.observed && runtime->bus.ioc_read_count(port) > state.reads_before) {
                        state.observed = true;
                        state.active_from = frame;
                        std::cout << console_stamp(process_start) << "[IOC PULSE HIT] frame=" << frame
                                  << " port=0x" << std::hex << static_cast<unsigned>(port)
                                  << std::dec << "\n";
                    }
                    if (state.armed && (!state.observed ||
                        static_cast<std::uint64_t>(frame) < static_cast<std::uint64_t>(state.active_from) + pulse.duration_frames))
                        active_ioc_xor[port] ^= pulse.mask;
                }
                for (const auto& pulse : options.periodic_input_pulses) {
                    if (frame < pulse.start_frame) continue;
                    const unsigned delta = frame - pulse.start_frame;
                    const unsigned ordinal = delta / pulse.every_frames;
                    if (pulse.count && ordinal >= pulse.count) continue;
                    if ((delta % pulse.every_frames) < pulse.duration_frames)
                        active_ioc_xor[pulse.port & 0x0f] ^= pulse.mask;
                }
                for (unsigned port = 0; port < active_ioc_xor.size(); ++port)
                    runtime->bus.set_ioc_scenario_xor_mask(static_cast<std::uint8_t>(port), active_ioc_xor[port]);

                // Apply debugger/scenario patches immediately before the CPUs execute this frame.
                for (auto& patch : options.memory_patches) {
                    if (patch.applied || frame < patch.frame) continue;
                    const unsigned size = static_cast<unsigned>(patch.width);
                    const auto oldv = runtime->bus.peek(patch.address, size, patch.space);
                    if (patch.conditional && oldv != patch.expected) continue;
                    runtime->bus.write(patch.address, patch.value, size, patch.space);
                    patch.applied = true;
                    if (patch_log) { patch_log << frame << ',' << (patch.space==chq::BusSpace::Main?'A':'B') << ',' << (size*8)
                        << ",0x" << std::hex << patch.address << ",0x" << oldv << ",0x" << patch.value << std::dec
                        << ',' << (patch.conditional?"conditional":"scheduled") << '\n'; patch_log.flush(); }
                    if (timeline_log) { timeline_log << frame << ",PATCH,0x" << std::hex << patch.address << " 0x" << oldv << "->0x" << patch.value << std::dec << "\n"; timeline_log.flush(); }
                    if (!patch_relative_scheduled) {
                        for (int off : options.capture_after_patch_offsets) { long long f=(long long)frame+off; if(f>=0) scheduled_capture_frames.insert((unsigned)f); }
                        patch_relative_scheduled = true;
                        if (scheduled_capture_frames.count(frame)) { capture_hardware_state(*runtime, options, frame); std::cout << console_stamp(process_start) << "[CAPTURE] frame=" << frame << " (patch-relative)\n"; scheduled_capture_frames.erase(frame); }
                    }
                    std::cout << console_stamp(process_start) << "[MEMORY PATCH] frame=" << frame << " cpu=" << (patch.space==chq::BusSpace::Main?'A':'B')
                              << " addr=0x" << std::hex << patch.address << " old=0x" << oldv << " new=0x" << patch.value << std::dec << "\n";
                }

                // v0.31: the graphical frontend is a normal playable machine view,
                // so its per-frame IRQ4 is no longer gated by a debugger flag.
                // ChaseHQRuntime retains --irq4 for explicit diagnostic control.
                if (first_loop_iteration) write_run_phase("first_frame_cpu_begin", frame);
                runtime->irq4();
                runtime->run(200000);
                ++frame;
                if (first_loop_iteration) write_run_phase("first_frame_cpu_complete", frame);
                if (timer_frozen) { runtime->bus.write(0x100200,frozen_timer_value,1,chq::BusSpace::Main); runtime->bus.write(0x100201,frozen_timer_fraction,1,chq::BusSpace::Main); }
                if (options.infinite_time) { runtime->bus.write(0x100200,survey_timer_bcd,1,chq::BusSpace::Main); runtime->bus.write(0x100201,0,1,chq::BusSpace::Main); }
                if (options.unlimited_turbo) runtime->bus.write(0x1003a2,3,2,chq::BusSpace::Main);
                if (debug_step_remaining) --debug_step_remaining;
                if (forensic_timeline_recording) forensic_capture_frame(frame);
                if (runtime->bus.research_break_pending()) {
                    const auto reason=runtime->bus.research_take_break_reason(); debug_paused=true; debug_step_remaining=0;
                    std::filesystem::create_directories(workbench_dir); runtime->save_checkpoint(workbench_pause_point,frame); workbench_have_pause_point=true;
                    std::cout<<console_stamp(process_start)<<"[WORKBENCH BREAK] frame="<<frame<<" "<<reason<<"\n"; changed=true;
                }
                if (input_trace && !fast_forwarding) input_trace << frame << ',' << (unsigned)runtime->bus.ioc_port_value(0x02) << ',' << (unsigned)runtime->bus.ioc_port_value(0x03) << ',' << (unsigned)runtime->bus.ioc_port_value(0x0c) << ',' << (unsigned)runtime->bus.ioc_port_value(0x0d) << ',' << runtime->bus.ioc_read_count(0x02) << ',' << runtime->bus.ioc_read_count(0x03) << ',' << runtime->bus.ioc_read_count(0x0c) << ',' << runtime->bus.ioc_read_count(0x0d) << '\n';
                if(gameplay_state && !fast_forwarding){
                    auto bcd=[](std::uint32_t v){return int(((v>>12)&15)*1000+((v>>8)&15)*100+((v>>4)&15)*10+(v&15));};
                    auto signed12=[](std::uint16_t v){v&=0x0fff; return (v&0x0800)?int(v)-0x1000:int(v);};
                    auto speed=runtime->bus.peek(0x100400,2,chq::BusSpace::Main); auto dist=runtime->bus.peek(0x102fc0,4,chq::BusSpace::Main);
                    const auto injected=runtime->bus.ioc_steering(); const auto game_raw=static_cast<std::uint16_t>(runtime->bus.peek(0x10014c,2,chq::BusSpace::Main)&0x0fff); const auto processed=static_cast<std::uint16_t>(runtime->bus.peek(0x100300,2,chq::BusSpace::Main));
                    const auto p64=runtime->bus.palette[64*16+13], p65=runtime->bus.palette[65*16+13]; const bool lamp=((p64&0x7fff)>0x1000)||((p65&0x7fff)>0x1000);
                    const auto rf=static_cast<unsigned>(runtime->bus.peek(0x10a048,1,chq::BusSpace::Main)); const char* zone=(rf&0x04)?"OFF_ROAD":((rf&0x06)==0x02?"EDGE":"INTERIOR");
                    const auto cp_grade=runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub); const unsigned gb=runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub)&0x0f; const unsigned goff=(cp_grade&0x3e000u)>>10; const auto ga=0x109000u+gb*0x100u+(goff&0xf8u); const int grade_signed=static_cast<std::int8_t>(runtime->bus.peek(ga+1,1,chq::BusSpace::Sub)); const char* grade_state=grade_signed>0?"INCLINE":(grade_signed<0?"DECLINE":"LEVEL");
                    gameplay_state<<frame<<",0x"<<std::hex<<speed<<std::dec<<','<<bcd(speed)<<','<<runtime->bus.peek(0x10041c,2,chq::BusSpace::Main)<<','<<dist<<','<<(double(dist)/256.0)<<",0x"<<std::hex<<runtime->bus.peek(0x100200,1,chq::BusSpace::Main)<<",0x"<<runtime->bus.peek(0x100408,2,chq::BusSpace::Main)<<std::dec<<','<<injected<<','<<signed12(injected)<<','<<game_raw<<','<<signed12(game_raw)<<','<<static_cast<std::int16_t>(processed)<<','<<course_follow_target<<','<<course_follow_curve_now<<','<<course_follow_curve_ahead<<','<<course_follow_curve_predict<<','<<course_follow_feedforward<<','<<course_follow_p_term<<','<<course_follow_d_term<<','<<course_follow_error_rate<<','<<course_follow_lateral_correction<<','<<course_follow_speed_target<<','<<(course_follow_accel_cmd?1:0)<<','<<(course_follow_brake_cmd?1:0)<<','<<options.course_follow_controller<<','<<course_follow_mode<<",0x"<<std::hex<<road_left_bound<<",0x"<<road_centre<<",0x"<<road_right_bound<<std::dec<<','<<road_width<<",0x"<<std::hex<<car_lateral<<std::dec<<','<<lateral_error<<','<<lateral_error_norm<<','<<lateral_side<<','<<((runtime->bus.ioc_port_value(3)&0x20)?0:1)<<','<<((runtime->bus.ioc_port_value(2)&0x20)?0:1)<<','<<(runtime->bus.peek(0x100212,2,chq::BusSpace::Main)?1:0)<<','<<runtime->bus.peek(0x1003a2,2,chq::BusSpace::Main)<<','<<(lamp?1:0)<<",0x"<<std::hex<<p64<<",0x"<<p65<<std::dec<<','<<((rf&0x04)?"OFF_ROAD":"ON_ROAD")<<','<<zone<<",0x"<<std::hex<<rf<<std::dec<<','<<grade_state<<','<<grade_signed<<'\n';
                    if(options.course_survey){
                        const auto& hs=runtime->handling_snapshot();
                        if(handling_state && hs.valid) handling_state<<frame<<','<<hs.turn_state<<','<<hs.table_index<<','<<hs.speed_internal<<','<<hs.forward_coeff<<','<<(double(static_cast<std::int16_t>(hs.forward_coeff))/256.0)<<','<<hs.lateral_coeff<<','<<(double(static_cast<std::int16_t>(hs.lateral_coeff))/256.0)<<','<<hs.applied_forward_coeff<<','<<(double(static_cast<std::int16_t>(hs.applied_forward_coeff))/256.0)<<','<<hs.applied_lateral_coeff<<','<<(double(static_cast<std::int16_t>(hs.applied_lateral_coeff))/256.0)<<','<<hs.forward_component<<','<<hs.lateral_component<<','<<(hs.override_active?1:0)<<','<<hs.samples<<'\n';
                        const auto prof_cp=runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub); const unsigned prof_bank=runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub)&0x0f; const unsigned prof_rec=((((prof_cp&0x3e000u)>>10)&0xffu)/8u);
                        if(driver_profile) driver_profile<<frame<<",0x"<<std::hex<<prof_cp<<std::dec<<','<<prof_bank<<','<<prof_rec<<','<<course_follow_curve_now<<','<<course_follow_curve_predict<<",0x"<<std::hex<<road_left_bound<<",0x"<<road_centre<<",0x"<<road_right_bound<<std::dec<<','<<road_width<<",0x"<<std::hex<<car_lateral<<std::dec<<','<<lateral_error<<','<<lateral_error_norm<<','<<course_follow_target<<','<<(course_follow_accel_cmd?1:0)<<','<<(course_follow_brake_cmd?1:0)<<','<<course_follow_speed_target<<','<<options.course_follow_controller<<','<<course_follow_mode<<','<<course_follow_profile_feedforward<<','<<course_follow_profile_delta<<'\n';
                        const int skmh=bcd(speed); const auto cp=runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub); const unsigned bank=runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub)&0x0f; const unsigned rec=(((cp&0x3e000u)>>10)&0xffu)/8u;
                        if((rf&0x04)!=0){ ++offroad_frames; if(!excursion_active){excursion_active=true;++excursion_id;excursion_start_frame=frame;excursion_start_cp=cp;excursion_start_bank=bank;excursion_start_record=rec;excursion_max_speed=skmh;excursion_start_lateral_error=lateral_error;excursion_max_abs_lateral_error=std::abs(int(lateral_error));} excursion_max_speed=std::max(excursion_max_speed,skmh); excursion_max_abs_lateral_error=std::max(excursion_max_abs_lateral_error,std::abs(int(lateral_error))); }
                        else { if((rf&0x06)==0x02) ++edge_frames; else ++interior_frames; if(excursion_active){ if(excursion_log) excursion_log<<excursion_id<<','<<excursion_start_frame<<','<<frame<<','<<(frame-excursion_start_frame)<<",0x"<<std::hex<<excursion_start_cp<<",0x"<<cp<<std::dec<<','<<excursion_start_bank<<','<<excursion_start_record<<','<<bank<<','<<rec<<','<<excursion_max_speed<<','<<excursion_start_lateral_error<<','<<excursion_max_abs_lateral_error<<'\n'; excursion_active=false; } }
                    }
                }
                if(target_state && !fast_forwarding){
                    const auto player_long=runtime->bus.peek(0x10a040,4,chq::BusSpace::Main);
                    const auto target_long=runtime->bus.peek(0x10a080,4,chq::BusSpace::Main);
                    const auto separation=static_cast<std::int32_t>(target_long-player_long);
                    const auto target_status=static_cast<std::uint8_t>(runtime->bus.peek(0x10a089,1,chq::BusSpace::Main));
                    const bool contact=(target_status&0x20)!=0;
                    const auto target_speed=static_cast<std::uint16_t>(runtime->bus.peek(0x10a092,2,chq::BusSpace::Main));
                    const auto target_motion=static_cast<std::uint16_t>(runtime->bus.peek(0x10a096,2,chq::BusSpace::Main));
                    const auto defeat_flags=static_cast<std::uint8_t>(runtime->bus.peek(0x10018d,1,chq::BusSpace::Main));
                    const bool defeat_mode=(defeat_flags&0x05)==0x05;
                    const auto defeat_timer=static_cast<std::uint16_t>(runtime->bus.peek(0x1002d0,2,chq::BusSpace::Main));
                    const auto interaction_type=static_cast<std::uint16_t>(runtime->bus.peek(0x10042e,2,chq::BusSpace::Main));
                    const auto interaction_timer=static_cast<std::uint16_t>(runtime->bus.peek(0x1002c6,2,chq::BusSpace::Main));
                    const auto interaction_counter=static_cast<std::uint16_t>(runtime->bus.peek(0x1002ae,2,chq::BusSpace::Main));
                    const auto suppressed=runtime->bus.suppressed_collision_shoves();
                    const auto suppressed_lateral=runtime->bus.suppressed_collision_lateral();
                    const auto suppressed_speed=runtime->bus.suppressed_collision_speed();
                    target_state<<frame<<",0x"<<std::hex<<player_long<<",0x"<<target_long<<std::dec<<','<<separation<<",0x"<<std::hex<<unsigned(target_status)<<std::dec<<','<<(contact?1:0)<<",0x"<<std::hex<<target_speed<<",0x"<<target_motion<<",0x"<<unsigned(defeat_flags)<<std::dec<<','<<(defeat_mode?1:0)<<','<<defeat_timer<<','<<interaction_type<<','<<interaction_timer<<','<<interaction_counter<<','<<suppressed<<','<<suppressed_lateral<<','<<suppressed_speed<<",319|320|321|322,152\n";
                    if(!target_contact_known){ last_target_contact=contact; target_contact_known=true; last_collision_timer=interaction_timer; last_suppressed_shoves=suppressed; last_suppressed_lateral=suppressed_lateral; last_suppressed_speed=suppressed_speed; }
                    else {
                        auto emit_collision=[&](const char* ev){ if(collision_events) collision_events<<frame<<','<<ev<<",0x"<<std::hex<<unsigned(target_status)<<std::dec<<','<<interaction_type<<','<<interaction_timer<<','<<interaction_counter<<','<<separation<<','<<suppressed<<','<<suppressed_lateral<<','<<suppressed_speed<<'\n'; };
                        if(contact!=last_target_contact) emit_collision(contact?"CONTACT_START":"CONTACT_END");
                        if(last_collision_timer==0 && interaction_timer!=0) emit_collision("INTERACTION_TIMER_START");
                        if(suppressed_lateral!=last_suppressed_lateral) emit_collision("LATERAL_RESPONSE_SUPPRESSED");
                        if(suppressed_speed!=last_suppressed_speed) emit_collision("SPEED_PENALTY_SUPPRESSED");
                        last_target_contact=contact; last_collision_timer=interaction_timer; last_suppressed_shoves=suppressed; last_suppressed_lateral=suppressed_lateral; last_suppressed_speed=suppressed_speed;
                    }
                }
                if(course_state && !fast_forwarding){
                    const auto cp=runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub); const unsigned bank=runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub)&0x0f; const unsigned off=(cp&0x3e000u)>>10; const unsigned rec=(off&0xffu)/8u; const auto a=0x109000u+bank*0x100u+(off&0xf8u); const auto c0=static_cast<std::uint8_t>(runtime->bus.peek(a,1,chq::BusSpace::Sub)); const auto rf=static_cast<unsigned>(runtime->bus.peek(0x10a048,1,chq::BusSpace::Main)); const char* zone=(rf&0x04)?"OFF_ROAD":((rf&0x06)==0x02?"EDGE":"INTERIOR");
                    course_state<<frame<<",0x"<<std::hex<<cp<<std::dec<<','<<bank<<','<<rec<<",0x"<<std::hex<<a<<std::dec<<','<<int(static_cast<std::int8_t>(c0))<<",0x"<<std::hex<<unsigned(c0)<<std::dec<<','<<int(static_cast<std::int8_t>(runtime->bus.peek(a+1,1,chq::BusSpace::Sub)))<<",0x"<<std::hex<<runtime->bus.peek(a+1,1,chq::BusSpace::Sub)<<",0x"<<runtime->bus.peek(a+3,1,chq::BusSpace::Sub)<<",0x"<<rf<<std::dec<<','<<zone<<'\n';
                }
                // v0.58 lean fast-forward: pursuit probe/palette CRC is observational and
                // expensive; resume it at the target frame. Emulated/game state is untouched.
                if(pursuit_probe && !fast_forwarding){
                    const auto cp=runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub); const unsigned bank=runtime->bus.peek(0x1021bc,1,chq::BusSpace::Sub)&0x0f; const unsigned rec=((((cp&0x3e000u)>>10)&0xffu)/8u); const auto score=runtime->bus.peek(0x100408,2,chq::BusSpace::Main);
                    const auto pal_crc=crc32_bytes(reinterpret_cast<const std::uint8_t*>(runtime->bus.palette.data()),runtime->bus.palette.size()*sizeof(runtime->bus.palette[0]));
                    pursuit_probe<<frame<<",0x"<<std::hex<<cp<<std::dec<<','<<bank<<','<<rec<<",0x"<<std::hex<<score<<",0x"<<pal_crc<<std::dec<<'\n';
                    if(last_pursuit_score==0xffffffffu) last_pursuit_score=score; else if(score!=last_pursuit_score){ if(pursuit_events) pursuit_events<<frame<<",SCORE_CHANGE,0x"<<std::hex<<last_pursuit_score<<",0x"<<score<<",0x"<<cp<<std::dec<<','<<bank<<','<<rec<<",0x"<<std::hex<<pal_crc<<std::dec<<'\n'; last_pursuit_score=score; }
                }
                for(auto& w: options.memory_watches) {
                    const unsigned size=static_cast<unsigned>(w.width); const auto v=runtime->bus.peek(w.address,size,w.space);
                    if(!w.initialised || v!=w.last_value) {
                        if(watch_log) watch_log<<frame<<','<<(w.space==chq::BusSpace::Main?'A':'B')<<','<<(size*8)<<",0x"<<std::hex<<w.address<<std::dec<<','<<w.label<<",0x"<<std::hex<<(w.initialised?w.last_value:v)<<",0x"<<v<<std::dec<<'\n';
                        std::cout<<console_stamp(process_start)<<"[WATCH] frame="<<frame<<" "<<w.label<<" 0x"<<std::hex<<w.address<<" = 0x"<<v<<std::dec<<"\n";
                        w.last_value=v; w.initialised=true;
                    }
                }
                for (auto& trig : options.transition_triggers) {
                    const unsigned size=static_cast<unsigned>(trig.width); const auto v=runtime->bus.peek(trig.address,size,trig.space);
                    if (!trig.initialised) { trig.last_value=v; trig.initialised=true; }
                    bool fire=false; if(!trig.fired){ if(trig.on_change && v!=trig.last_value) fire=true; else if(trig.value && v==*trig.value) fire=true; }
                    if(fire){ trig.fired=true; const std::string label=trig.label.empty()?"transition":trig.label;
                        std::cout<<console_stamp(process_start)<<"[TRANSITION] frame="<<frame<<" "<<label<<" addr=0x"<<std::hex<<trig.address<<" value=0x"<<v<<std::dec<<"\n";
                        if(timeline_log){timeline_log<<frame<<",TRANSITION,"<<label<<" addr=0x"<<std::hex<<trig.address<<" value=0x"<<v<<std::dec<<"\n";timeline_log.flush();}
                        for(int off:options.capture_relative_offsets){long long f=(long long)frame+off;if(f>=0)scheduled_capture_frames.insert((unsigned)f);}
                    }
                    trig.last_value=v;
                }
                if (scheduled_capture_frames.count(frame)) {
                    capture_hardware_state(*runtime, options, frame);
                    if (options.diagnostics_profile == "graphics" || options.diagnostics_profile == "all") {
                        video.add_diagnostics_frame(frame);
                        video.draw_runtime(machine,*runtime,scene,frame,debug_paused,timer_frozen);
                    }
                    if(timeline_log){timeline_log<<frame<<",CAPTURE,capture_frame_"<<frame<<"\n";timeline_log.flush();}
                    std::cout << console_stamp(process_start) << "[CAPTURE] frame=" << frame << " -> " << (options.logs/("capture_frame_"+std::to_string(frame))).string() << "\n";
                    scheduled_capture_frames.erase(frame);
                }
                for(auto& q:options.save_checkpoints){ if(!q.saved && frame>=q.frame){ runtime->save_checkpoint(q.path,frame); q.saved=true;
                    if(timeline_log){timeline_log<<frame<<",CHECKPOINT_SAVE,"<<q.path.string()<<"\n";timeline_log.flush();}
                    std::cout<<console_stamp(process_start)<<"[CHECKPOINT SAVE] frame="<<frame<<" -> "<<q.path.string()<<"\n"; }}

                bool break_rendered = false;
                const bool break_hit = std::find(options.break_frames.begin(), options.break_frames.end(), frame) != options.break_frames.end();
                if (break_hit) {
                    const auto br = options.logs/("break_frame_"+std::to_string(frame)); std::filesystem::create_directories(br);
                    capture_hardware_state(*runtime, options, frame);
                    runtime->write_forensic_history(br/"recent_execution.csv");
                    std::ofstream bsum(br/"break_summary.txt"); bsum<<"Chase H.Q. Native v"<<chq::kNativeVersion<<" forensic frame breakpoint\nframe="<<frame<<"\nhistory_entries="<<options.break_history<<"\ncontinue="<<options.break_frame_continue<<"\nwait="<<options.break_frame_wait<<"\n"; runtime->summary(bsum);
                    video.draw_runtime(machine,*runtime,scene,frame); break_rendered=true;
                    runtime->write_provenance_reports();
                    std::cout<<console_stamp(process_start)<<"[FORENSIC BREAK] frame="<<frame<<" -> "<<br.string()<<"\n";
                    if(options.break_frame_wait){std::cout<<"Press Enter to continue..."<<std::flush;std::string dummy;std::getline(std::cin,dummy);}
                    if(!options.break_frame_continue) running=false;
                }

                if (options.sprite_debug) {
                    bool hit = options.sprite_debug_frames.empty() ? (frame == options.frames) :
                        std::find(options.sprite_debug_frames.begin(), options.sprite_debug_frames.end(), frame) != options.sprite_debug_frames.end();
                    if (hit) { dump_sprite_diagnostics(machine, *runtime, options, frame); if (options.sprite_export) export_sprite_assets(machine, *runtime, options, frame); }
                }

                const bool fast_preview = fast_forwarding && options.fast_forward_render_every && (frame % options.fast_forward_render_every)==0;
                if ((!fast_forwarding || fast_preview) && !break_rendered) {
                    if (first_loop_iteration) write_run_phase("first_frame_render_begin", frame);
                    video.draw_runtime(machine, *runtime, scene, frame, debug_paused, timer_frozen);
                    if (first_loop_iteration) write_run_phase("first_frame_render_complete", frame);
                    const bool sprite_evidence = options.sprite_evidence_every && frame >= options.sprite_evidence_from && (!options.sprite_evidence_to || frame <= options.sprite_evidence_to) && ((frame - options.sprite_evidence_from) % options.sprite_evidence_every == 0);
                    if (sprite_evidence) { const auto dir = options.sprite_evidence_dir.empty() ? (options.logs / "sprite_evidence") : options.sprite_evidence_dir; std::ostringstream n; n << "sprites_frame_" << std::setfill('0') << std::setw(8) << frame << ".csv"; video.write_sprite_evidence_csv(dir / n.str(), frame); if(!options.sprite_quad_candidates.empty()){std::ostringstream q; q << "sprite_objects_frame_" << std::setfill('0') << std::setw(8) << frame << ".csv"; video.write_sprite_quad_evidence_csv(dir / q.str(), frame);} std::vector<std::string> semantic=options.object_evidence; for(const auto& x:options.object_track) if(std::find(semantic.begin(),semantic.end(),x)==semantic.end()) semantic.push_back(x); if(!semantic.empty()) video.write_semantic_object_evidence_csv(dir / "semantic_objects.csv", frame, semantic); }
                    const bool periodic_shot = options.screenshot_every && frame >= options.screenshot_from && (!options.screenshot_to || frame <= options.screenshot_to) && ((frame - options.screenshot_from) % options.screenshot_every == 0);
                    const bool explicit_shot = scheduled_screenshot_frames.erase(frame) != 0;
                    if ((periodic_shot || explicit_shot) && screenshots_written < options.screenshot_limit) {
                        const auto dir = options.screenshot_dir.empty() ? (options.logs / "screenshots") : options.screenshot_dir;
                        std::ostringstream fn; fn << "frame_" << std::setfill('0') << std::setw(8) << frame << ".png";
                        const auto shot = dir / fn.str();
                        if (video.save_current_screenshot(shot)) { captured_screenshots.push_back(shot); ++screenshots_written; std::cout << console_stamp(process_start) << "[SCREENSHOT] frame=" << frame << " -> " << shot.string() << "\n"; }
                    }
                    changed = false;
                }
            } else if (runtime) {
                if ((bounded_run && frame >= effective_frames) || !runtime->fault.empty()) save();
                if (changed || debug_paused) {
                    video.draw_runtime(machine, *runtime, scene, frame, debug_paused, timer_frozen);
                    changed = false;
                }
            } else if (changed) {
                video.draw_scene(machine, scene);
                changed = false;
            }

            if (runtime && !runtime->fault.empty()) {
                std::cerr << console_stamp(process_start) << "[RUN FAULT] frame=" << frame << " " << runtime->fault << "\n";
                running = false;
            }
            if (runtime && bounded_run && frame >= effective_frames) {
                std::cout << console_stamp(process_start) << "[RUN TARGET REACHED] frame=" << frame << " target=" << effective_frames << "\n";
                running = false;
            }
            if (first_loop_iteration) { write_run_phase("first_loop_iteration_complete", frame); first_loop_iteration = false; }
            if (running && !fast_forwarding) std::this_thread::sleep_until(tick + std::chrono::microseconds(16667));
        }

        if(forensic_timeline_recording) forensic_stop();
        save();
        const bool faulted = runtime && !runtime->fault.empty();
        {
            const double total_s=std::chrono::duration<double>(std::chrono::steady_clock::now()-process_start).count();
            std::filesystem::create_directories(options.logs);
            std::ofstream tf(options.logs/"run_timing.txt");
            tf<<"build="<<chq::kNativeVersion<<"\nframes="<<frame<<"\ntotal_seconds="<<std::fixed<<std::setprecision(3)<<total_s
              <<"\naverage_frames_per_second="<<(total_s>0?double(frame)/total_s:0.0)<<"\n";
            std::cout<<console_stamp(process_start)<<"[TIMING] frames="<<frame<<" total="<<std::fixed<<std::setprecision(3)<<total_s
                     <<"s avg="<<(total_s>0?double(frame)/total_s:0.0)<<" f/s\n"<<std::defaultfloat;
        }
        video.shutdown();
        if(input_trace.is_open()){input_trace.flush();input_trace.close();}
        if(gameplay_state.is_open()){gameplay_state.flush();gameplay_state.close();}
        if(target_state.is_open()){target_state.flush();target_state.close();}
        if(collision_events.is_open()){collision_events.flush();collision_events.close();}
        if(course_state.is_open()){course_state.flush();course_state.close();}
        if(pursuit_probe.is_open()){pursuit_probe.flush();pursuit_probe.close();} if(pursuit_events.is_open()){pursuit_events.flush();pursuit_events.close();}
        if(excursion_log.is_open()){ if(excursion_active){ const auto cp=runtime?runtime->bus.peek(0x10080e,4,chq::BusSpace::Sub):0; excursion_log<<excursion_id<<','<<excursion_start_frame<<','<<frame<<','<<(frame-excursion_start_frame)<<",0x"<<std::hex<<excursion_start_cp<<",0x"<<cp<<std::dec<<','<<excursion_start_bank<<','<<excursion_start_record<<",-1,-1,"<<excursion_max_speed<<','<<excursion_start_lateral_error<<','<<excursion_max_abs_lateral_error<<'\n'; } excursion_log.flush(); excursion_log.close(); }
        if(options.course_survey){ std::ofstream ss(options.logs/"course_survey_summary.txt"); const auto total=interior_frames+edge_frames+offroad_frames; ss<<"build="<<chq::kNativeVersion<<"\nframes="<<total<<"\ninterior_frames="<<interior_frames<<"\nedge_frames="<<edge_frames<<"\noffroad_frames="<<offroad_frames<<"\nexcursions="<<excursion_id<<"\nlookahead_records="<<options.course_follow_lookahead<<"\nbase_steer="<<options.course_follow_steer<<"\nlegacy_recovery_steer="<<options.course_follow_recovery_steer<<"\nlateral_kp="<<options.course_follow_lateral_kp<<"\nlateral_kd="<<options.course_follow_lateral_kd<<"\nlateral_max="<<options.course_follow_lateral_max<<"\nlateral_deadzone="<<options.course_follow_lateral_deadzone<<"\ncontroller="<<options.course_follow_controller<<"\nsteering_slew="<<options.course_follow_slew<<"\nspeed_control="<<(options.course_follow_speed_control?1:0)<<"\nsuppressed_collision_responses="<<(runtime?runtime->bus.suppressed_collision_shoves():0)<<"\nsuppressed_collision_lateral="<<(runtime?runtime->bus.suppressed_collision_lateral():0)<<"\nsuppressed_collision_speed="<<(runtime?runtime->bus.suppressed_collision_speed():0)<<"\n"; }
        if(patch_log.is_open()){patch_log.flush();patch_log.close();}
        if(watch_log.is_open()){watch_log.flush();watch_log.close();}
        if(timeline_log.is_open()){timeline_log.flush();timeline_log.close();}
        runtime.reset(); // close all runtime-owned log streams before packaging
        const auto sprite_csv_count = count_matching_files(options.logs / "sprite_evidence", "sprites_frame_", ".csv");
        const bool completed = !faulted && (!bounded_run || frame >= effective_frames);
        write_run_status(options.logs, run_start_frame, frame, effective_frames, bounded_run, completed, faulted ? std::string("runtime fault") : std::string(), sprite_csv_count);
        std::cout << console_stamp(process_start) << "[RUN STATUS] completed=" << (completed?"yes":"no") << " start=" << run_start_frame << " final=" << frame
                  << " sprite_evidence_files=" << sprite_csv_count << "\n";
        if(options.auto_zip_logs) {
            for(const auto& shot : captured_screenshots) {
                const auto target=options.logs/"screenshots"/shot.filename();
                if(std::filesystem::absolute(shot).lexically_normal()!=std::filesystem::absolute(target).lexically_normal()) {
                    std::filesystem::create_directories(target.parent_path());
                    std::filesystem::copy_file(shot,target,std::filesystem::copy_options::overwrite_existing);
                }
            }
            for(auto requested : options.screenshot_at) if(requested>=run_start_frame && requested<=frame) {
                std::ostringstream fn;fn<<"frame_"<<std::setfill('0')<<std::setw(8)<<requested<<".png";
                if(!std::filesystem::exists(options.logs/"screenshots"/fn.str())) throw std::runtime_error("Requested screenshot is missing from evidence run");
            }
            for(auto requested : options.graphics_debug_frames) if(requested>=run_start_frame && requested<=frame)
                if(!std::filesystem::exists(options.logs/("graphics_frame_"+std::to_string(requested)))) throw std::runtime_error("Requested graphics diagnostics are missing from evidence run");
            std::ofstream manifest(options.logs/"evidence-manifest.json");
            manifest<<"{\"schema\":\"chq-cli-evidence-v1\",\"build\":\""<<chq::kNativeVersion<<"\",\"run\":\""<<json_escape(options.logs.filename().string())<<"\",\"name\":\""<<json_escape(options.evidence_name)<<"\",\"created\":\""<<timestamp_name()<<"\",\"startFrame\":"<<run_start_frame<<",\"finalFrame\":"<<frame<<",\"files\":[";
            bool first=true;
            for(const auto& entry : std::filesystem::recursive_directory_iterator(options.logs)) if(entry.is_regular_file() && entry.path().filename()!= "evidence-manifest.json") {
                if(!first) manifest<<',';first=false;
                manifest<<"{\"path\":\""<<json_escape(std::filesystem::relative(entry.path(),options.logs).generic_string())<<"\",\"bytes\":"<<entry.file_size()<<'}';
            }
            manifest<<"]}\n";
        }
        write_artifact_validation(options.logs);
        if (options.auto_zip_logs && std::filesystem::exists(options.logs)) {
            auto zip = options.zip_name.empty() ? default_bundle_name(options) : std::filesystem::path(options.zip_name);
            if (zip.extension() != ".zip") zip += ".zip";
            if (zip_directory_best(options.logs,zip)) {
                std::error_code zec; const auto zip_bytes=std::filesystem::file_size(zip,zec);
                std::cout << console_stamp(process_start) << "[LOG BUNDLE] ready=" << zip.string();
                if(!zec) std::cout << " bytes=" << zip_bytes;
                std::cout << "\n";
                if (options.zip_delete_source) { std::error_code ec; std::filesystem::remove_all(options.logs,ec); }
            } else std::cerr << "[LOG BUNDLE] failed to create bundle\n";
        }
        return faulted ? 2 : 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        if (emergency_bundle && !emergency_logs.empty()) {
            try {
                std::filesystem::create_directories(emergency_logs);
                std::ofstream crash(emergency_logs/"run_failure.txt"); crash << "build=" << chq::kNativeVersion << "\nerror=" << e.what() << "\n"; crash.close();
                write_artifact_validation(emergency_logs);
                if (!emergency_zip.empty() && zip_directory_best(emergency_logs, emergency_zip))
                    std::cerr << "[EMERGENCY BUNDLE] " << emergency_zip.string() << "\n";
            } catch (...) { std::cerr << "[EMERGENCY BUNDLE] failed while handling error\n"; }
        }
        return 1;
    }
}

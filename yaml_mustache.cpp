#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <mustache.hpp>
#include <ranges>
#include <vector>

using mdata = kainjow::mustache::data;
using kainjow::mustache::partial;
using mustache = kainjow::mustache::mustache;
using namespace std;

constexpr string_view KWD_SETTINGS    = "__settings__";
constexpr string_view KWD_MAP_AS_LIST = "map_as_list";
constexpr string_view KWD_KEY         = "key";
constexpr string_view KWD_VALUE       = "value";

class Settings
{
    set<string> _map_as_list;
    auto split_path(string path)
    {
        vector<string> parts;
        for (auto&& sub : path | std::views::split('/'))
        {
            string s(&*sub.begin(), std::ranges::distance(sub));
            if (!s.empty()) parts.push_back(move(s));
        }
        return parts;
    }
    void handleMapAsList(YAML::Node& node, vector<string>& pattern, size_t index=0, string curpath="")
    {
        if ( index == pattern.size() )
        {
            _map_as_list.insert(curpath);
            return;
        }
        if ( not node.IsMap() ) return;
        string child_path = curpath + "/";
        auto& key = pattern[index];
        if ( key == "*" )
            for( auto it : node )
                handleMapAsList(it.second, pattern, index + 1, child_path + it.first.as<string>());
        else
        {
            auto&& child = node[key];
            if (child) handleMapAsList(child,pattern,index+1,child_path+key);
        }
    }
public:
    bool treatAsList(string path) const
    { return _map_as_list.find(path) != _map_as_list.end(); }
    Settings(YAML::Node& topnode)
    {
        if ( topnode[KWD_SETTINGS] && topnode[KWD_SETTINGS][KWD_MAP_AS_LIST] )
            for ( auto&& n : topnode[KWD_SETTINGS][KWD_MAP_AS_LIST] )
            {
                auto&& patternv = split_path(n.as<string>());
                handleMapAsList(topnode,patternv);
            }
    }
};

mdata yaml_to_mustache_data(const YAML::Node& node, const Settings& settings, string path="/")
{
    switch (node.Type()) {

        case YAML::NodeType::Scalar: return mdata(node.Scalar());

        case YAML::NodeType::Sequence: {
            mdata list = mdata::type::list;
            for (auto&& elem : node) list.push_back(yaml_to_mustache_data(elem,settings));
            return list;
        }

        case YAML::NodeType::Map: {
            string child_path = path;
            if (child_path.back() != '/') child_path += '/';
            if ( settings.treatAsList(path) )
            {
                mdata list = mdata::type::list;
                for (auto&& it : node) {
                    auto key = it.first.as<string>();
                    child_path += key;
                    mdata o = mdata::type::object;
                    o.set(string(KWD_KEY),key);
                    o.set(string(KWD_VALUE),yaml_to_mustache_data(it.second,settings,child_path));
                    list.push_back(o);
                }
                return list;
            }
            {
                mdata o = mdata::type::object;
                for (auto&& it : node) {
                    auto key = it.first.as<string>();
                    child_path += key;
                    o.set(key,yaml_to_mustache_data(it.second,settings,child_path));
                }
                return o;
            }
        }

        case YAML::NodeType::Null:
        case YAML::NodeType::Undefined:
        default: return mdata();
    }
}

void check_exists(char *path)
{
    if ( not filesystem::exists(path) )
    {
        cerr << "File not found: " << path << endl;
        exit(1);
    }
}

YAML::Node file2yaml(char* path)
{
    try {
        auto yaml = YAML::LoadFile(path);
        return yaml;
    }
    catch ( exception& e ) {
        cerr << e.what() << endl;
        exit(1);
    }
}

stringstream file2str(char* path)
{
    ifstream file(path);
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer;
}

mustache file2tmpl(char* path)
{
    auto buf = file2str(path);
    string tmpl_text = buf.str();
    mustache tmpl(tmpl_text);
    return tmpl;
}

void set_partial(mdata& data,char *path)
{
    auto partialname = filesystem::path(path).filename().string();
    auto buf = file2str(path).str();
    data.set(partialname, partial{ [buf]{ return buf; }});
}

int main(int argc, char *argv[])
{
    if ( argc < 3 )
    {
        cerr << "Usage: " << argv[0] << " <yamlfile> <main-mustache-template> [sub-template]..." << endl;
        exit(1);
    }
    for(int i=1; i<argc; i++) check_exists(argv[i]);
    auto yaml = file2yaml(argv[1]);
    Settings settings(yaml);
    auto data = yaml_to_mustache_data(yaml,settings);

    auto main_tmpl = file2tmpl(argv[2]);
    for(int i=3; i<argc; i++) set_partial(data,argv[i]);
 
    cout << main_tmpl.render(data);
}

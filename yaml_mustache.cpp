#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <mustache.hpp>

using mdata = kainjow::mustache::data;
using kainjow::mustache::partial;
using mustache = kainjow::mustache::mustache;
using namespace std;

mdata yaml_to_mustache_data(const YAML::Node& node)
{
    switch (node.Type()) {

        case YAML::NodeType::Scalar: return mdata(node.Scalar());

        case YAML::NodeType::Sequence: {
            mdata list = mdata::type::list;
            for (auto&& elem : node) list.push_back(yaml_to_mustache_data(elem));
            return list;
        }

        case YAML::NodeType::Map: {
            mdata o = mdata::type::object;
            for (auto&& it : node) {
                auto key = it.first.as<string>();
                o.set(key,yaml_to_mustache_data(it.second));
            }
            return o;
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
    auto data = yaml_to_mustache_data(yaml);

    auto main_tmpl = file2tmpl(argv[2]);
    for(int i=3; i<argc; i++) set_partial(data,argv[i]);
 
    cout << main_tmpl.render(data);
}

#ifndef XMLCONVERTER_H
#define XMLCONVERTER_H

#include <string>
#include <vector>
#include <ostream>

namespace xmlconverter {

// 各XML要素に対応する構造体の定義
struct Metadata {
    std::string key;
    std::string value;
};

struct MeshStat {
    int edges_fixed;
    int degenerate_facets;
    int facets_removed;
    int facets_reversed;
    int backwards_edges;
};

struct Part {
    int id;
    std::string subtype;
    std::vector<Metadata> metadata;
    MeshStat mesh_stat;
};

struct Object {
    int id;
    std::vector<Metadata> metadata;
    std::vector<Part> parts;
};

struct ModelInstance {
    std::vector<Metadata> metadata;
};

struct Plate {
    std::vector<Metadata> metadata;
    ModelInstance model_instance;
};

struct AssembleItem {
    int object_id;
    int instance_id;
    std::string transform;
    std::string offset;
};

struct Assemble {
    std::vector<AssembleItem> items;
};

struct Config {
    std::vector<Object> objects;
    std::vector<Plate> plates;
    Assemble assemble;
};

// XML出力関数
// 指定した出力ストリームにXML形式で出力します。
void writeConfig(const Config& config, std::ostream& os);

// 指定したファイル名へXMLを書き出します。書き出しに成功すれば true を返します。
bool writeConfigToFile(const Config& config, const std::string& filename);

} // namespace xmlconverter

#endif // XMLCONVERTER_H

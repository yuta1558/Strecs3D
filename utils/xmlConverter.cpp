#include "xmlConverter.h"
#include <fstream>
#include <iostream>

namespace xmlconverter {

// ヘルパー関数：インデント出力
static void indent(std::ostream& os, int level) {
    for (int i = 0; i < level; i++) {
        os << "  ";
    }
}

static void writeMetadata(const Metadata& m, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<metadata key=\"" << m.key << "\" value=\"" << m.value << "\"/>\n";
}

static void writeMeshStat(const MeshStat& ms, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<mesh_stat edges_fixed=\"" << ms.edges_fixed
       << "\" degenerate_facets=\"" << ms.degenerate_facets
       << "\" facets_removed=\"" << ms.facets_removed
       << "\" facets_reversed=\"" << ms.facets_reversed
       << "\" backwards_edges=\"" << ms.backwards_edges
       << "\"/>\n";
}

static void writePart(const Part& part, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<part id=\"" << part.id << "\" subtype=\"" << part.subtype << "\">\n";
    for (const auto& m : part.metadata) {
        writeMetadata(m, os, indentLevel + 1);
    }
    writeMeshStat(part.mesh_stat, os, indentLevel + 1);
    indent(os, indentLevel);
    os << "</part>\n";
}

static void writeObject(const Object& obj, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<object id=\"" << obj.id << "\">\n";
    for (const auto& m : obj.metadata) {
        writeMetadata(m, os, indentLevel + 1);
    }
    for (const auto& p : obj.parts) {
        writePart(p, os, indentLevel + 1);
    }
    indent(os, indentLevel);
    os << "</object>\n";
}

static void writeModelInstance(const ModelInstance& mi, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<model_instance>\n";
    for (const auto& m : mi.metadata) {
        writeMetadata(m, os, indentLevel + 1);
    }
    indent(os, indentLevel);
    os << "</model_instance>\n";
}

static void writePlate(const Plate& plate, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<plate>\n";
    for (const auto& m : plate.metadata) {
        writeMetadata(m, os, indentLevel + 1);
    }
    writeModelInstance(plate.model_instance, os, indentLevel + 1);
    indent(os, indentLevel);
    os << "</plate>\n";
}

static void writeAssembleItem(const AssembleItem& item, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<assemble_item object_id=\"" << item.object_id 
       << "\" instance_id=\"" << item.instance_id 
       << "\" transform=\"" << item.transform 
       << "\" offset=\"" << item.offset << "\" />\n";
}

static void writeAssemble(const Assemble& asmbl, std::ostream& os, int indentLevel) {
    indent(os, indentLevel);
    os << "<assemble>\n";
    for (const auto& item : asmbl.items) {
        writeAssembleItem(item, os, indentLevel + 1);
    }
    indent(os, indentLevel);
    os << "</assemble>\n";
}

void writeConfig(const Config& config, std::ostream& os) {
    os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    os << "<config>\n";
    for (const auto& obj : config.objects) {
        writeObject(obj, os, 1);
    }
    for (const auto& plate : config.plates) {
        writePlate(plate, os, 1);
    }
    writeAssemble(config.assemble, os, 1);
    os << "</config>\n";
}

bool writeConfigToFile(const Config& config, const std::string& filename) {
    std::ofstream ofs(filename);
    if (!ofs) {
        std::cerr << "ファイル " << filename << " を開けませんでした" << std::endl;
        return false;
    }
    writeConfig(config, ofs);
    ofs.close();
    return true;
}

} // namespace xmlconverter

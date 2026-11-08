#include "Yaml.h"
#include <format>
#include <fstream>
#include <iterator>
#include <yaml-cpp/yaml.h>

using namespace ReyEngine;

namespace {
   void collect(const YAML::Node& node, const std::string& path, std::vector<YamlLeaf>& leaves) {
      const auto child = [&](const std::string& key){return path.empty() ? key : path + "." + key;};
      switch (node.Type()) {
         case YAML::NodeType::Map:
            for (const auto& entry : node) collect(entry.second, child(entry.first.Scalar()), leaves);
            break;
         case YAML::NodeType::Sequence:
            for (size_t i = 0; i < node.size(); i++) collect(node[i], child(std::to_string(i)), leaves);
            break;
         case YAML::NodeType::Scalar:
            leaves.push_back({path, node.Scalar(), node.Mark().line + 1});
            break;
         case YAML::NodeType::Null:
         case YAML::NodeType::Undefined:
            break;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::expected<std::vector<YamlLeaf>, std::string> ReyEngine::readYamlLeaves(const std::string& text) {
   try {
      std::vector<YamlLeaf> leaves;
      collect(YAML::Load(text), "", leaves);
      return leaves;
   } catch (const YAML::Exception& e) {
      return std::unexpected(std::format("line {}: {}", e.mark.line + 1, e.msg));
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
std::expected<std::vector<YamlLeaf>, std::string> ReyEngine::loadYamlLeaves(const std::filesystem::path& file) {
   std::ifstream in(file, std::ios::binary);
   if (!in) return std::unexpected("can't read " + file.string());
   auto leaves = readYamlLeaves(std::string(std::istreambuf_iterator<char>(in), {}));
   if (!leaves) return std::unexpected(file.string() + ", " + leaves.error());
   return leaves;
}

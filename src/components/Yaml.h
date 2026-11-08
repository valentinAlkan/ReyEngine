#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <vector>

// A YAML document read as its leaves: every value that isn't a map or a list, with the dotted path to it ("a.b.c", a list's
// items by their place, "a.0") and the line it's on, so a settings file can be bound to fields by name, and any mistake in
// it said with where it is. A key with nothing under it (an empty map, a null) has no leaves. Needs REY_ENABLE_YAML.
namespace ReyEngine {
   struct YamlLeaf {
      std::string path;
      std::string value; //as written, without quotes
      int line = 0;      //from 1
   };
   /// a document's leaves, in the order they're written; why not if it isn't YAML
   [[nodiscard]] std::expected<std::vector<YamlLeaf>, std::string> readYamlLeaves(const std::string& text);
   /// a file's: why not if it can't be read, or isn't YAML (with its name)
   [[nodiscard]] std::expected<std::vector<YamlLeaf>, std::string> loadYamlLeaves(const std::filesystem::path& file);
}

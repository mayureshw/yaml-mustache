# yaml-mustache
yaml-mustache bridge to generate templatized text from yaml

## Dependencies

yaml-cpp and mustache-cpp. This utility is a bridge between them.

## settings

TODO: Examples are to be added

- `map_as_list`: specify paths to keys whose vals are to be treated as a list instead of map. Paths are a '/' delimited string of keys starting with a leading '/'. When a map is converted to a list, it becomes a list of maps with keys 'key' and 'value'.

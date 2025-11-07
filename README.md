# yaml-mustache
yaml-mustache bridge to generate templatized text from yaml

## Dependencies

yaml-cpp and mustache-cpp. This utility is a bridge between them.

## Note

Since this is a generic converter, there is no intermediate data structure conversion involved from an input data structure to the one that feeds to mustache. Here, yaml data feeds as-is to mustache. Hence dynamic key names are difficult to use. If this becomes a frequent requirement we may add support for it, but we may require at least a metadata tag that tells which maps are to be converted to a list of maps with fixed keys.

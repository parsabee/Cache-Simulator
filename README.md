## BUILD
in the root of the project
```
mkdir build
cd build
cmake ../
cmake --build .
```
## Execute
```
usage: cache-sim [-hvd] -i input-file -c config-level -s associativity

options:
  -c, --config             configuration level: 1 | 2 | 3
  -s, --associativity      set associativity
  -i, --input              input trace file
  -d, --debug
  -h, --help
  -v, --version
```
## Example
in the build directory
```
./cache-sim -i ../sample-trace/cc.trace -c 3 -s 16
```

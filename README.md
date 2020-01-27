###BUILD
in the root of the project
```asm
mkdir build
cd build
cmake ../
cmake --build .
```
###Execute
```asm
usage: cache-sim [-hvd] -i input-file -c config-level -s associativity

options:
  --config, c             configuration level: 1 | 2 | 3
  --associativity, s      set associativity
  --input, i              input trace file
  --debug, d
  --help, d
  --version, d
```
###Example
```asm
./cache-sim -i ../sample-trace/cc.trace -c 3 -s 16
```
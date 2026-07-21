```sh
g++ -O3 naive.cpp && ./a.out 
```

https://microsoftedge.github.io/Demos/json-dummy-data/5MB.json

Assembly:
```sh
g++ -O3 -S naive.cpp
```

```sh
g++ -mavx2 simd_minifier.cpp && ./a.out 307B.json
```


Debug:
```sh
g++ -g -mavx2 simd_minifier.cpp
gdb --args a.out pretty.json
#(gdb) break simd_minifier.cpp:137
#(gdb) run
#(gdb) p/u lookup[lo_mask]
#(gdb) p/x lookup[lo_mask]
#(gdb) p /t lookup[lo_mask]



g++ -g -mavx2 simd_minifier.cpp && gdb -x init.gdb --args ./a.out pretty.json
```

TODO:
NDJSON
Huge workloads, GB/s
Escaped chars
Unrolling to hide latency
Optimize loading/writing too

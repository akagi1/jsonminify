```sh
user@arch:~/jsonminify (master!)% g++ -O3 -march=native naive_minifier.cpp && ./a.out 1GB.json 
Time: 2.62477s
Throughput: 0.409662 GB/s
Minified file written to 1GB.min.json

user@arch:~/jsonminify (master!)% g++ -O3 -march=native simd_minifier.cpp && ./a.out 1GB.json 
Time: 0.447589s
Throughput: 2.40235 GB/s
Minified file written to 1GB.min.json
```

https://microsoftedge.github.io/Demos/json-dummy-data/5MB.json

Assembly:
```sh
g++ -O3 -S naive.cpp
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
Avoid left-packing

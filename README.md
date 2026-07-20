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



TODO:
NDJSON
Huge workloads, GB/s
Unicode
Escaped chars
Unrolling to hide latency
Optimize loading/writing too

<h2 align="center">Cache playground</h2>

A project designed to analyze and visualize the performance of various caching algorithms using real-world datasets.

### Datasets
* **Twitter logs (2020):** Sourced from the official [twitter/cache-trace](https://github.com/twitter/cache-trace) repository.
* **Coming soon...**

### Algorithms
* **LRU cache**
* **LFU cache**: with decaying counters (inspired by the [Misra-Gries algorithm](https://en.wikipedia.org/wiki/Misra%E2%80%93Gries_summary)) 
* **LRFU cache**: a hybrid LRFU cache split 50/50 between LRU and LFU segments, where a key moves to the LFU cache upon the second hit
* **ARC cache**:  [adaptive replacement cache](https://en.wikipedia.org/wiki/Adaptive_replacement_cache)
* **LIRS cache**: a cache based on the [Low Inter-reference Recency Set replacement policy](https://ranger.uta.edu/~sjiang/pubs/papers/jiang02_LIRS.pdf)
* **DLIRS cache**: dynamic LIRS cache ([DLIRS: Improving Low Inter-Reference Recency Set
  Cache Replacement Policy with Dynamics](https://www.systor.org/2018/pdf/systor18-4.pdf))
* **OPT cache**: an offline, theoretically optimal cache algorithm used as a baseline

### Efficiency Metrics

Used a **Accuracy** formula that excludes compulsory misses (the first occurrence of a key):

$$Accuracy = \frac{\text{Total Keys} - \text{Unique Keys}}{\text{Cache Misses} - \text{Unique Keys}}$$


---

<h2 align="center">Twitter Dataset</h2>

<table width="100%">
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster001-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster002-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster003-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster004-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster005-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster006-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster007-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster008-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster009-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster010-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster011-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster012-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster013-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster014-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster015-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster016-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster017-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster018-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster019-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/2_twitter/cluster020-plot.png" width="100%"></td>
  </tr>
</table>



### Plotting graphics
Command to get one graphic (run from root of repository)
```bash
python3 ./plot_metrics.py --dataset=./Datasets/2_twitter/cluster003-parsed.txt --plot=./Datasets/2_twitter/cluster003-plot.png --executable=./cmake-build-debug/Cache
```

Command to plot all graphics (run from root of repository)

```bash
./Datasets/2_twitter/run-all.sh
```

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
* **LIRS2 cache**: [an Improved LIRS Replacement Algorithm](https://ranger.uta.edu/~sjiang/pubs/papers/zhong21-LIRS2.pdf)
* **OPT cache**: an offline, theoretically optimal cache algorithm used as a baseline

### Efficiency Metrics

Used a **Accuracy** formula that excludes compulsory misses (the first occurrence of a key):

$$Accuracy = \frac{\text{Total Keys} - \text{Unique Keys}}{\text{Cache Misses} - \text{Unique Keys}}$$


---

<h2 align="center">Twitter Dataset</h2>

<table width="100%">
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster001-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster002-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster003-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster004-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster005-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster006-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster007-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster008-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster009-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster010-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster011-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster012-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster013-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster014-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster015-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster016-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster017-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster018-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster019-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster020-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster022-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster023-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster024-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster025-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster026-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster027-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster028-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster029-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster030-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster031-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster032-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster033-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster034-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster035-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster036-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster037-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster038-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster039-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster040-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster041-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster042-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster043-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster044-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster045-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster046-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster047-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster048-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster049-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster050-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster051-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster052-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster053-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/twitter-cluster054-plot.png" width="100%"></td>
  </tr>
</table>


<h2 align="center">Synthetic Dataset</h2>

<table width="100%">
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/multiple_sequential_scan-plot.png" width="100%"></td>
  </tr>
  <tr>
    <td width="100%"><img src="Datasets/graphics/all/zig_zag_scan-plot.png" width="100%"></td>
  </tr>
</table>



### Plotting graphics
Command to get one graphic (run from root of repository)
```bash
python3 ./plot_metrics.py --dataset=./Datasets/2_twitter/twitter-cluster003-parsed.txt --plot=./Datasets/2_twitter/twitter-cluster003-plot.png --executable=./cmake-build-debug/Cache
```

Command to plot all graphics (run from root of repository)

```bash
./Datasets/2_twitter/run-all.sh
```

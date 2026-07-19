#!/bin/bash

DATA_DIR="./Datasets/2_twitter"
GRAPHICS_DIR="./Datasets/graphics/lirs"
SCRIPT="./plot_metrics.py"
EXECUTABLE="./cmake-build-debug/Cache"

for i in {001..020}; do
    python3 "$SCRIPT" \
      --dataset="$DATA_DIR/cluster$i-parsed.txt" \
      --plot="$GRAPHICS_DIR/twitter-cluster$i-plot.png" \
      --executable="$EXECUTABLE" \
      --cache opt lru lirs dlirs arc lirs2 &
done


# synthetic data
DATA_DIR="./Datasets/1_synthetic"
python3 "$SCRIPT" \
      --dataset="$DATA_DIR/multiple_sequential_scan.txt" \
      --plot="$GRAPHICS_DIR/multiple_sequential_scan-plot.png" \
      --executable="$EXECUTABLE" &

python3 "$SCRIPT" \
      --dataset="$DATA_DIR/zig_zag_scan.txt" \
      --plot="$GRAPHICS_DIR/zig_zag_scan-plot.png" \
      --executable="$EXECUTABLE" &

wait

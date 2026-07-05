#!/bin/bash

DATA_DIR="./Datasets/2_twitter"
SCRIPT="./plot_metrics.py"
EXECUTABLE="./cmake-build-debug/Cache"

for i in {001..020}; do
    python3 "$SCRIPT" --dataset="$DATA_DIR/cluster$i-parsed.txt" --plot="$DATA_DIR/cluster$i-plot.png" --executable="$EXECUTABLE" &
done

wait

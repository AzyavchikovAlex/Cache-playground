import statistics
import matplotlib.patches as patches
import matplotlib.pyplot as plt
import numpy as np
from collections import defaultdict

def read_keys_sequence(path):
    keys = []
    with open(path, 'r', encoding='utf-8') as file:
        type, size = file.readline().strip().split()
        size = int(size)
    
        for i in range(size):
            key_str = file.readline().strip()
            keys.append(int(key_str))
    return keys


def get_keys_frequencies(keys_sequence):
    keys_frequencies = {}
    for key in keys_sequence:
        if key not in keys_frequencies:
            keys_frequencies[key] = 0
        keys_frequencies[key] += 1
    return keys_frequencies


def cacl_herfindahl_hirschman_index(values):
    frequencies = get_keys_frequencies(keys_sequence=values)
    total_elements = len(values)
    
    return sum((freq / total_elements) ** 2 for freq in frequencies.values())


def print_sequence_stat(values):
    max_val = max(values)
    min_val = min(values)
    print(f"Max: {max_val}")
    print(f"Min: {min_val}")

    avg_val = sum(values) / len(values)
    print(f"AVG: {avg_val:.2f}")

    median_val = statistics.median(values)
    print(f"Median: {median_val}")


def print_by_buckets(keys, num_buckets = None, title = None):
    plt.figure(figsize=(14, 6))
    if num_buckets is not None:
        max_value = max(keys);
        min_value = min(keys);
        bucket_step = max((max_value - min_value + 1) // num_buckets, 1)
        buckets = list(range(min_value, max_value + bucket_step, bucket_step))
        counts, bins, patches = plt.hist(keys, bins=buckets, edgecolor='black', color='skyblue', alpha=0.7)
        plt.xticks(buckets)
    else:
        counts, bins, patches = plt.hist(keys, edgecolor='black', color='skyblue', alpha=0.7)

    if title is not None: 
        plt.title(title)
    plt.grid(axis='y', linestyle='--', alpha=0.7)
    
    plt.show()


# returns requests count between each key re request
def get_key_inter_reference_stats(keys):
    keys_last_used_times = {}
    keys_timings = {}
    
    current_time = 0
    for key in keys:
        if key not in keys_last_used_times:
            keys_last_used_times[key] = 0
        else:
            delta = current_time - keys_last_used_times[key]
            if key not in keys_timings:
                keys_timings[key] = []
            keys_timings[key].append(delta)
    
        keys_last_used_times[key] = current_time;
        current_time += 1
    return keys_timings



# returns unques keys count between each key re request
def get_key_inter_reference_stats2(keys):
    keys_timings = defaultdict(list)
    last_seen = {}
    
    n = len(keys)
    bit = [0] * (n + 1)

    for i, key in enumerate(keys):
        if key in last_seen:
            prev_idx = last_seen[key]
            
            s1 = 0
            idx = i
            while idx > 0:
                s1 += bit[idx]
                idx -= idx & (-idx)
                
            s2 = 0
            idx = prev_idx + 1
            while idx > 0:
                s2 += bit[idx]
                idx -= idx & (-idx)
                
            keys_timings[key].append(s1 - s2)
            
            idx = prev_idx + 1
            while idx <= n:
                bit[idx] -= 1
                idx += idx & (-idx)
                
        idx = i + 1
        while idx <= n:
            bit[idx] += 1
            idx += idx & (-idx)
            
        last_seen[key] = i
        
    return dict(keys_timings)
        

import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def plot_reuse_distance_stability(data: dict, use_median: bool = False):
    """
    Строит 2D scatter plot: Среднее (или медиана) vs Стандартное отклонение.
    Помогает оценить общую предсказуемость и частоту обращений к ключам.
    """
    centers = []
    stds = []
    
    for key, distances in data.items():
        if len(distances) > 1:
            # Выбираем метрику центральной тенденции
            center_val = np.median(distances) if use_median else np.mean(distances)
            centers.append(center_val)
            stds.append(np.std(distances))
            
    if not centers:
        print("Недостаточно данных: ключи должны иметь более 1 интервала.")
        return

    plt.figure(figsize=(10, 6))
    plt.scatter(centers, stds, alpha=0.6, edgecolors='none', c='royalblue')
    
    x_label = 'Медиана' if use_median else 'Среднее значение'
    plt.xlabel(f'{x_label} (Reuse Distance)')
    plt.ylabel('Стандартное отклонение (Изменчивость)')
    plt.title('Анализ стабильности повторных обращений к ключам')
    plt.grid(True, linestyle='--', alpha=0.5)
    plt.show()


def plot_reuse_distance_heatmap_with_rect(data: dict, keys_sequence: list, bins=50, rect_x=None, rect_y=None):
    """
    Строит тепловую карту с возможностью выделения зоны (прямоугольника) 
    и подсчета процента обращений в этой зоне.
    
    rect_x: tuple (x_min, x_max) - границы по оси X (ранг)
    rect_y: tuple (y_min, y_max) - границы по оси Y (reuse distance)
    """
    freqs = get_keys_frequencies(keys_sequence)
    sorted_keys_by_freq = sorted(freqs.keys(), key=lambda k: freqs[k], reverse=True)
    key_to_rank = {key: rank for rank, key in enumerate(sorted_keys_by_freq)}
    
    x_ranks = []
    y_distances = []
    
    for key, distances in data.items():
        rank = key_to_rank[key]
        for dist in distances:
            x_ranks.append(rank)
            y_distances.append(dist)
            
    if not x_ranks:
        return

    fig, ax = plt.subplots(figsize=(12, 8))
    h = ax.hist2d(x_ranks, y_distances, bins=bins, cmap='magma_r', cmin=1)
    fig.colorbar(h[3], ax=ax, label='Количество обращений (плотность)')
    
    if rect_x is not None and rect_y is not None:
        xmin, xmax = rect_x
        ymin, ymax = rect_y
        
        inside_count = sum(1 for x, y in zip(x_ranks, y_distances) 
                           if xmin <= x <= xmax and ymin <= y <= ymax)
        total_count = len(x_ranks)
        percentage = (inside_count / total_count) * 100
        
        print(f"Всего повторных обращений: {total_count}")
        print(f"Обращений в выделенной зоне: {inside_count}")
        print(f"Процент 'теплых' обращений в зоне: {percentage:.2f}%")
        

        rect = patches.Rectangle((xmin, ymin), xmax - xmin, ymax - ymin,
                                 linewidth=2, edgecolor='gray', facecolor='gray', alpha=0.4)
        ax.add_patch(rect)
        
        ax.text(xmax, ymax, f' {percentage:.1f}%', color='black', fontsize=12, 
                verticalalignment='bottom',
                bbox=dict(facecolor='white', alpha=0.7, edgecolor='gray', boxstyle='round,pad=0.3'))

    ax.set_xlabel('Ранг ключа (0 = самый популярный)')
    ax.set_ylabel('Reuse Distance')
    ax.set_title('Тепловая карта с анализом "теплых" зон')
    ax.grid(alpha=0.2)
    plt.show()


def plot_continuous_working_set_size(keys_sequence: list, window_size: int):
    """
    Строит непрерывный график количества уникальных ключей в скользящем окне.
    Окно смещается ровно на 1 элемент. Работает за O(N).
    """
    n = len(keys_sequence)
    if n < window_size:
        print("Размер окна больше длины последовательности.")
        return

    unique_counts = []
    window_freqs = {}
    current_unique = 0

    for i in range(window_size):
        key = keys_sequence[i]
        if window_freqs.get(key, 0) == 0:
            current_unique += 1
        window_freqs[key] = window_freqs.get(key, 0) + 1
        
    unique_counts.append(current_unique)

    for i in range(window_size, n):
        left_key = keys_sequence[i - window_size]
        window_freqs[left_key] -= 1
        if window_freqs[left_key] == 0:
            current_unique -= 1
            del window_freqs[left_key]

        right_key = keys_sequence[i]
        if window_freqs.get(right_key, 0) == 0:
            current_unique += 1
        window_freqs[right_key] = window_freqs.get(right_key, 0) + 1

        unique_counts.append(current_unique)

    plt.figure(figsize=(14, 6))
    
    x_axis = range(window_size, n + 1)
    
    plt.plot(x_axis, unique_counts, color='coral', linewidth=1.5)
    
    plt.xlabel('Номер запроса (конец окна)')
    plt.ylabel(f'Уникальных ключей в окне ({window_size})')
    plt.title('Непрерывная динамика рабочего набора (Sliding Window)')
    plt.grid(True, linestyle='--', alpha=0.7)
    
    min_u = min(unique_counts)
    max_u = max(unique_counts)
    avg_u = sum(unique_counts) / len(unique_counts)
    plt.axhline(avg_u, color='blue', linestyle=':', label=f'Среднее: {avg_u:.0f}')
    plt.legend()
    
    plt.show()

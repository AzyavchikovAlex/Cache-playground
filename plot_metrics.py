import argparse
import os
import subprocess
import sys
import math
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

SPLIT_FRACTION = 0.15

CACHES_TO_TEST = ["lru", "lfu", "lrfu", "arc", "lirs", "dlirs"]

CACHE_COLORS = {
    "lru": "#1f77b4",    # Синий
    "lfu": "#2ca02c",    # Зеленый
    "lrfu": "#d62728",   # Красный
    "arc": "#9467bd",    # Фиолетовый
    "lirs": "#ff7f0e",   # Оранжевый
    "dlirs": "#e377c2",  # Розовый
}
DEFAULT_COLOR = "#7f7f7f"

LINE_WIDTH = 0.8
MARKER_SIZE = 2.5


def parse_arguments():
    parser = argparse.ArgumentParser(description="Бенчмарк кешей: детальный и общий виды.")
    parser.add_argument(
        "-e", "--executable", type=str,
        default="./cmake-build-debug/Cache",
        help="Путь к исполняемому файлу C++",
    )
    parser.add_argument(
        "-d", "--dataset", type=str,
        default="Datasets/2_twitter/cluster001-parsed.txt",
        help="Путь к файлу датасета",
    )
    parser.add_argument(
        "-p", "--plot", type=str,
        default="cache_full_analysis_plot.png",
        help="Путь для сохранения графика",
    )
    return parser.parse_args()


def run_cpp_benchmark(executable_path, cache_name, dataset_path):
    print(f"Запускаем {executable_path} для кеша {cache_name.upper()}...")
    try:
        result = subprocess.run(
            [executable_path, cache_name, dataset_path],
            capture_output=True, text=True, check=True,
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Ошибка при выполнении C++ кода для {cache_name}!", file=sys.stderr)
        print(e.stderr, file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Файл {executable_path} не найден. Проверьте путь.", file=sys.stderr)
        sys.exit(1)


def parse_output(output):
    sizes, fractions, accuracies = [], [], []
    for line in output.strip().split("\n"):
        parts = line.split("\t")
        if len(parts) == 3:
            try:
                sizes.append(int(parts[0]))
                fractions.append(float(parts[1]))
                accuracies.append(float(parts[2]))
            except ValueError:
                continue
    return sizes, fractions, accuracies


def set_evenly_spaced_log_ticks(ax, ylim, num_ticks=10):
    """Создает визуально равномерные отметки для логарифмической шкалы"""
    ymin, ymax = ylim
    log_min = math.log10(ymin)
    log_max = math.log10(ymax)

    step = (log_max - log_min) / (num_ticks - 1)
    ticks = [10**(log_min + i * step) for i in range(num_ticks)]

    ax.set_yticks(ticks)
    ax.set_yticks([], minor=True)

    def format_tick(x, pos):
        s = f"{x:.3g}"
        if "e" not in s.lower():
            s = s.rstrip('.')
        return s

    ax.yaxis.set_major_formatter(ticker.FuncFormatter(format_tick))


def main():
    args = parse_arguments()
    executable_path = args.executable
    test_file = args.dataset

    all_results = {}
    longest_fractions = []

    for cache_name in CACHES_TO_TEST:
        output = run_cpp_benchmark(executable_path, cache_name, test_file)
        sizes, fractions, accuracies = parse_output(output)

        if fractions:
            all_results[cache_name] = {
                "sizes": sizes,
                "fractions": fractions,
                "accuracies": accuracies,
            }
            if len(fractions) > len(longest_fractions):
                longest_fractions = fractions

    if not all_results:
        print("Нет данных для построения графика.")
        return

    first_cache = list(all_results.values())[0]
    total_keys = (
        int(first_cache["sizes"][-1] / first_cache["fractions"][-1])
        if first_cache["fractions"][-1] > 0 else 1
    )

    max_x_value = max([max(res["fractions"]) for res in all_results.values() if res["fractions"]])

    target_split_point = max_x_value * SPLIT_FRACTION
    closest_split_x = min(longest_fractions, key=lambda x: abs(x - target_split_point))

    split_size = int(closest_split_x * total_keys)
    max_size = int(max_x_value * total_keys)

    fig, (ax1, ax2, ax3) = plt.subplots(
        1, 3,
        figsize=(18, 6),
        gridspec_kw={'width_ratios': [1, 1, 0.8], 'wspace': 0.08}
    )

    plot_data = {}
    left_y_min, left_y_max = float('inf'), float('-inf')
    right_y_min_pos, right_y_max = float('inf'), float('-inf')
    all_y_min_pos, all_y_max = float('inf'), float('-inf')

    for cache_name in CACHES_TO_TEST:
        if cache_name not in all_results:
            continue

        res = all_results[cache_name]
        fracs = res["fractions"]
        accs = res["accuracies"]

        if len(fracs) < len(longest_fractions):
            start_idx = len(fracs)
            fracs.extend(longest_fractions[start_idx:])
            last_acc = accs[-1] if accs else 1.0
            accs.extend([last_acc] * (len(longest_fractions) - start_idx))

        miss_rates = [1.0 - a for a in accs]
        line_color = CACHE_COLORS.get(cache_name.lower(), DEFAULT_COLOR)

        fracs_left, miss_left = [], []
        fracs_right, miss_right = [], []

        for f, m in zip(fracs, miss_rates):
            if f <= closest_split_x:
                fracs_left.append(f)
                miss_left.append(m)
            if f >= closest_split_x:
                fracs_right.append(f)
                miss_right.append(m)

        plot_data[cache_name] = {
            "fracs_left": fracs_left, "miss_left": miss_left,
            "fracs_right": fracs_right, "miss_right": miss_right,
            "fracs_all": fracs, "miss_all": miss_rates,
            "color": line_color
        }

        if miss_left:
            left_y_min = min(left_y_min, min(miss_left))
            left_y_max = max(left_y_max, max(miss_left))

        if miss_right:
            right_y_max = max(right_y_max, max(miss_right))
            pos_right = [m for m in miss_right if m > 0]
            if pos_right:
                right_y_min_pos = min(right_y_min_pos, min(pos_right))

        if miss_rates:
            all_y_max = max(all_y_max, max(miss_rates))
            pos_all = [m for m in miss_rates if m > 0]
            if pos_all:
                all_y_min_pos = min(all_y_min_pos, min(pos_all))

    def get_limits_with_margin(vmin, vmax, margin=0.05):
        delta = vmax - vmin
        if delta == 0: delta = 0.1
        return vmin - delta * margin, vmax + delta * margin

    def get_log_limits(ymin_pos, ymax, margin=0.1):
        if ymin_pos == float('inf') or ymax == float('-inf'):
            return 1e-3, 1.0
        if ymin_pos >= ymax:
            return ymin_pos * 0.5, ymax * 2.0
        factor = (ymax / ymin_pos) ** margin
        return ymin_pos / factor, ymax * factor

    ax1_ylim = get_limits_with_margin(left_y_min, left_y_max)
    ax2_ylim = get_log_limits(right_y_min_pos, right_y_max)
    ax3_ylim = get_log_limits(all_y_min_pos, all_y_max)

    for cache_name, data in plot_data.items():
        color = data["color"]

        # График 1 (Линейный)
        ax1.plot(
            data["fracs_left"], data["miss_left"],
            marker="o", markersize=MARKER_SIZE, linewidth=LINE_WIDTH,
            linestyle="-", color=color, label=f"{cache_name.upper()} Cache"
        )

        # График 2 (Логарифмический). Заменяем нули на нижнюю границу графика, чтобы линия уходила вниз
        miss_right_log = [m if m > 0 else ax2_ylim[0] for m in data["miss_right"]]
        ax2.plot(
            data["fracs_right"], miss_right_log,
            marker="o", markersize=MARKER_SIZE, linewidth=LINE_WIDTH,
            linestyle="-", color=color
        )

        # График 3 (Логарифмический). Заменяем нули на нижнюю границу графика
        miss_all_log = [m if m > 0 else ax3_ylim[0] for m in data["miss_all"]]
        ax3.plot(
            data["fracs_all"], miss_all_log,
            marker="o", markersize=MARKER_SIZE, linewidth=LINE_WIDTH,
            linestyle="-", color=color
        )

    ax1.set_ylim(*ax1_ylim)

    ax2.set_yscale('log')
    ax2.set_ylim(*ax2_ylim)

    ax3.set_yscale('log')
    ax3.set_ylim(*ax3_ylim)

    set_evenly_spaced_log_ticks(ax2, ax2_ylim, num_ticks=10)
    set_evenly_spaced_log_ticks(ax3, ax3_ylim, num_ticks=10)
    for ax in (ax2, ax3):
        ax.yaxis.set_minor_locator(ticker.NullLocator())
        ax.yaxis.set_minor_formatter(ticker.NullFormatter())
        ax.yaxis.set_major_formatter(ticker.ScalarFormatter())

        # Настраиваем оси X с небольшими отступами
    ax1_x_margin = closest_split_x * 0.05
    ax1.set_xlim(-ax1_x_margin, closest_split_x + ax1_x_margin)

    ax2_x_margin = (max_x_value - closest_split_x) * 0.03
    ax2.set_xlim(closest_split_x - ax2_x_margin, max_x_value + ax2_x_margin)

    ax3.set_xlim(-max_x_value * 0.02, max_x_value * 1.02)

    num_ticks_ax1 = 5
    step_ax1 = closest_split_x / (num_ticks_ax1 - 1)
    ax1_ticks = [i * step_ax1 for i in range(num_ticks_ax1)]
    ax1.set_xticks(ax1_ticks)

    num_ticks_ax2 = 5
    step_ax2 = (max_x_value - closest_split_x) / (num_ticks_ax2 - 1)
    ax2_ticks = [closest_split_x + i * step_ax2 for i in range(num_ticks_ax2)]
    ax2.set_xticks(ax2_ticks)

    dataset_basename = os.path.basename(test_file)
    dataset_title, _ = os.path.splitext(dataset_basename)

    fig.suptitle(dataset_title, fontsize=18, fontweight="bold", y=0.98)

    ax1.set_title(f"Sizes 0 - {split_size}", fontsize=13)
    ax2.set_title(f"Sizes {split_size} - {max_size}", fontsize=13)
    ax3.set_title(f"Overall (Sizes 0 - {max_size})", fontsize=13)

    ax1.set_ylabel("Miss Rate (1 - Accuracy)", fontsize=12, fontweight="bold")

    fig.supxlabel("Cache fraction (size)", fontsize=14, fontweight="bold", y=0.02)

    def custom_x_formatter(x, pos):
        if x < 0: return ""
        percent = x * 100
        actual_size = int(x * total_keys)
        if percent == 0:
            return "0%\n(0)"
        percent_str = f"{percent:.4f}".rstrip('0').rstrip('.')
        return f"{percent_str}%\n({actual_size})"

    def minor_y_formatter(x, pos):
        s = f"{x:.3f}".rstrip('0').rstrip('.')
        return s if s else "0"

    for ax in (ax1, ax2, ax3):
        ax.xaxis.set_major_formatter(ticker.FuncFormatter(custom_x_formatter))
        ax.minorticks_on()
        ax.grid(True, which='major', linestyle='--', linewidth=0.8, alpha=0.6)
        ax.grid(True, which='minor', linestyle=':', linewidth=0.5, alpha=0.4)

    ax1.yaxis.set_minor_formatter(ticker.FuncFormatter(minor_y_formatter))
    ax1.tick_params(axis='y', which='minor', labelsize=8, labelcolor='#666666')

    ax1.legend(loc="upper right", fontsize=10)

    plt.subplots_adjust(bottom=0.18)
    plt.savefig(args.plot, dpi=300, bbox_inches="tight", pad_inches=0.02)
    print(f"График успешно сохранен в файл '{args.plot}'")
    plt.show()

if __name__ == "__main__":
    main()
import argparse
import os
import subprocess
import sys
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

CACHES_TO_TEST = ["opt", "lru", "lfu", "lrfu", "arc", "lirs", "dlirs"]

CACHE_COLORS = {
    "opt": "#404040",    # Темно-серый
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
    parser = argparse.ArgumentParser(description="Бенчмарк кешей: Accuracy и сравнение с OPT.")
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
        default="cache_accuracy_analysis.png",
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


def main():
    args = parse_arguments()
    executable_path = args.executable
    test_file = args.dataset

    all_results = {}

    for cache_name in CACHES_TO_TEST:
        output = run_cpp_benchmark(executable_path, cache_name, test_file)
        sizes, fractions, accuracies = parse_output(output)

        if fractions:
            all_results[cache_name] = {
                "sizes": sizes,
                "fractions": fractions,
                "accuracies": accuracies,
            }

    if not all_results:
        print("Нет данных для построения графика.")
        return

    if "opt" not in all_results:
        print("Внимание: Данные для OPT кеша отсутствуют. Сравнение невозможно.")
        return

    opt_data = all_results["opt"]
    opt_accuracy_map = {size: acc for size, acc in zip(opt_data["sizes"], opt_data["accuracies"])}

    first_cache = list(all_results.values())[0]
    total_keys = (
        int(first_cache["sizes"][-1] / first_cache["fractions"][-1])
        if first_cache["fractions"][-1] > 0 else 1
    )

    max_x_value = max([max(res["fractions"]) for res in all_results.values() if res["fractions"]])

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6), gridspec_kw={'wspace': 0.15})

    rel_y_min, rel_y_max = float('inf'), float('-inf')

    for cache_name in CACHES_TO_TEST:
        if cache_name not in all_results:
            continue

        res = all_results[cache_name]
        sizes = res["sizes"]
        fracs = res["fractions"]
        accs = res["accuracies"]

        color = CACHE_COLORS.get(cache_name.lower(), DEFAULT_COLOR)
        is_opt = (cache_name.lower() == "opt")

        # --- График 1: Абсолютная Accuracy ---
        ax1.plot(
            fracs, accs,
            marker="o", markersize=MARKER_SIZE, linewidth=LINE_WIDTH * (1.5 if is_opt else 1.0),
            linestyle="--" if is_opt else "-", color=color, label=f"{cache_name.upper()}"
        )

        # --- График 2: Относительная Accuracy (в % от OPT) ---
        rel_accs = []
        valid_fracs = []

        for size, frac, acc in zip(sizes, fracs, accs):
            if size in opt_accuracy_map:
                opt_acc = opt_accuracy_map[size]
                if opt_acc > 0:
                    rel_acc = (acc / opt_acc) * 100.0
                elif acc == 0 and opt_acc == 0:
                    rel_acc = 100.0
                else:
                    continue

                rel_accs.append(rel_acc)
                valid_fracs.append(frac)

        if rel_accs:
            ax2.plot(
                valid_fracs, rel_accs,
                marker="o", markersize=MARKER_SIZE, linewidth=LINE_WIDTH * (2.0 if is_opt else 1.0),
                linestyle="--" if is_opt else "-", color=color, label=f"{cache_name.upper()}"
            )

            # Ищем минимум и максимум для масштабирования (включая OPT, чтобы 100% всегда было в кадре)
            rel_y_min = min(rel_y_min, min(rel_accs))
            rel_y_max = max(rel_y_max, max(rel_accs))


    dataset_basename = os.path.basename(test_file)
    dataset_title, _ = os.path.splitext(dataset_basename)
    fig.suptitle(f"{dataset_title}", fontsize=16, fontweight="bold", y=0.98)

    ax1.set_title("Overall Accuracy", fontsize=13)
    ax1.set_ylabel("Accuracy", fontsize=12, fontweight="bold")

    ax2.set_title("Accuracy Relative to OPT Cache", fontsize=13)
    ax2.set_ylabel("Compliance with OPT (%)", fontsize=12, fontweight="bold")

    # Масштабирование правого графика (под данные, не с нуля)
    if rel_y_min != float('inf') and rel_y_max != float('-inf'):
        y_range = rel_y_max - rel_y_min
        margin = y_range * 0.05 if y_range > 0 else 1.0
        ax2.set_ylim(rel_y_min - margin, rel_y_max + margin)

    fig.supxlabel("Cache fraction (size)", fontsize=14, fontweight="bold", y=0.02)

    def custom_x_formatter(x, pos):
        if x < 0: return ""
        percent = x * 100
        actual_size = int(x * total_keys)
        if percent == 0:
            return "0%\n(0)"
        percent_str = f"{percent:.4f}".rstrip('0').rstrip('.')
        return f"{percent_str}%\n({actual_size})"

    for ax in (ax1, ax2):
        ax.set_xlim(-max_x_value * 0.02, max_x_value * 1.02)
        ax.xaxis.set_major_formatter(ticker.FuncFormatter(custom_x_formatter))

        ax.minorticks_on()
        ax.grid(True, which='major', linestyle='-', linewidth=0.8, alpha=0.7)
        ax.grid(True, which='minor', linestyle=':', linewidth=0.5, alpha=0.5)

        ax.legend(loc="lower right" if ax == ax1 else "best", fontsize=10)

    plt.subplots_adjust(bottom=0.15)
    plt.savefig(args.plot, dpi=300, bbox_inches="tight", pad_inches=0.05)
    print(f"График успешно сохранен в файл '{args.plot}'")
    plt.show()

if __name__ == "__main__":
    main()
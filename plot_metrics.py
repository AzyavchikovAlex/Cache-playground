import argparse
import subprocess
import sys
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# ГЛОБАЛЬНЫЙ ПАРАМЕТР: список кешей для тестирования
CACHES_TO_TEST = ["lru", "lfu", "lrfu", "arc"]


def parse_arguments():
    """Парсер аргументов командной строки."""
    parser = argparse.ArgumentParser(
        description="Бенчмарк кешей с построением графика точности."
    )
    parser.add_argument(
        "-d",
        "--dataset",
        type=str,
        default="Datasets/2_twitter/cluster001-parsed.txt",
        help="Путь к файлу датасета для тестирования",
    )
    parser.add_argument(
        "-p",
        "--plot",
        type=str,
        default="cache_accuracy_plot.png",
        help="Путь для сохранения итогового графика (картинки)",
    )
    return parser.parse_args()


def run_cpp_benchmark(executable_path, cache_name, dataset_path):
    print(f"Запускаем {executable_path} для кеша {cache_name.upper()}...")
    try:
        result = subprocess.run(
            [executable_path, cache_name, dataset_path],
            capture_output=True,
            text=True,
            check=True,
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(
            f"Ошибка при выполнении C++ кода для {cache_name}!",
            file=sys.stderr,
        )
        print(e.stderr, file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Файл {executable_path} не найден.", file=sys.stderr)
        sys.exit(1)


def parse_output(output):
    sizes = []
    fractions = []
    accuracies = []
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
    # Читаем аргументы командной строки
    args = parse_arguments()

    executable_path = "./cmake-build-debug/Cache"
    test_file = args.dataset  # Используем аргумент --dataset

    all_results = {}
    longest_fractions = []  # Эталонный массив X для дополнения коротких графиков

    # 1. Собираем данные для всех кешей
    for cache_name in CACHES_TO_TEST:
        output = run_cpp_benchmark(executable_path, cache_name, test_file)
        sizes, fractions, accuracies = parse_output(output)

        if fractions:
            all_results[cache_name] = {
                "sizes": sizes,
                "fractions": fractions,
                "accuracies": accuracies,
            }
            # Ищем самый длинный массив fractions
            if len(fractions) > len(longest_fractions):
                longest_fractions = fractions
        else:
            print(f"Предупреждение: Нет данных для кеша {cache_name}")

    if not all_results:
        print("Нет данных для построения графика.")
        return

    # Вычисляем общее количество ключей в датасете
    first_cache = list(all_results.values())[0]
    total_keys = (
        int(first_cache["sizes"][-1] / first_cache["fractions"][-1])
        if first_cache["fractions"][-1] > 0
        else 1
    )

    # 2. Строим график
    fig, ax = plt.subplots(figsize=(10, 6))
    colors = ["b", "g", "r", "c", "m", "y", "k"]

    for i, cache_name in enumerate(CACHES_TO_TEST):
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

        ax.plot(
            fracs,
            accs,
            marker="o",
            markersize=4,
            linestyle="-",
            color=colors[i % len(colors)],
            label=f"{cache_name.upper()} Cache",
        )

    # 3. Настраиваем внешний вид
    ax.set_title("Зависимость точности кешей от их размера", fontsize=14)
    ax.set_ylabel("Точность (Accuracy)", fontsize=12)
    ax.set_xlabel(
        "Размер кеша: % от датасета\n(фактическое количество элементов)",
        fontsize=12,
    )
    ax.grid(True, linestyle="--", alpha=0.7)
    ax.set_ylim(0, 1.05)
    ax.legend()

    # 4. МАГИЯ ДЛЯ ОСИ X
    def custom_x_formatter(x, pos):
        percent = x * 100
        actual_size = int(x * total_keys)
        return f"{percent:.1f}%\n({actual_size})"

    ax.xaxis.set_major_formatter(ticker.FuncFormatter(custom_x_formatter))
    plt.subplots_adjust(bottom=0.15)

    # 5. Сохраняем (используем аргумент --plot) и показываем
    plt.savefig(args.plot, dpi=300, bbox_inches="tight")
    print(f"График успешно сохранен в файл '{args.plot}'")
    plt.show()


if __name__ == "__main__":
    main()

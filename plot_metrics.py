import argparse
import os
import subprocess
import sys
import matplotlib.pyplot as plt
import matplotlib.ticker as ticker

# список кешей для тестирования
CACHES_TO_TEST = ["lru", "lfu", "lrfu", "arc", "lirs"]

CACHE_COLORS = {
    "lru": "#1f77b4",   # Синий
    "lfu": "#2ca02c",   # Зеленый
    "lrfu": "#d62728",  # Красный
    "arc": "#9467bd",    # Фиолетовый
    "lirs": "#ff7f0e"   # Оранжевый
}
DEFAULT_COLOR = "#7f7f7f"

# ГЛОБАЛЬНЫЕ ПАРАМЕТРЫ СТИЛЯ ГРАФИКА
LINE_WIDTH = 0.7  # Толщина линий
MARKER_SIZE = 2.8  # Размер точек
ZOOMED_PART = 1 / 10


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
    test_file = args.dataset

    all_results = {}
    longest_fractions = []

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

    # 2. Строим основной график
    fig, ax = plt.subplots(figsize=(10, 6))

    # Вычисляем максимальное значение по оси X среди всех доступных данных
    max_x_value = max([max(res["fractions"]) for res in all_results.values() if res["fractions"]])
    X_MAX_ZOOM = max_x_value * ZOOMED_PART

    # Вычисляем максимальное значение Accuracy для правильного масштабирования осей
    max_accuracy = max([max(res["accuracies"]) for res in all_results.values() if res["accuracies"]])
    # Добавим небольшой отступ сверху (например, 5% от максимума), чтобы линии не упирались в край
    Y_MAX_LIMIT = max_accuracy * 1.05 if max_accuracy > 0 else 1.05

    # Создаем встроенный график (inset) в правом нижнем углу основного графика
    # Координаты: [отступ_слева, отступ_снизу, ширина, высота] от 0 до 1
    ax_inset = ax.inset_axes([0.58, 0.08, 0.4, 0.4])

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

        line_color = CACHE_COLORS.get(cache_name.lower(), DEFAULT_COLOR)

        # Рисуем на основном графике
        ax.plot(
            fracs,
            accs,
            marker="o",
            markersize=MARKER_SIZE,
            linewidth=LINE_WIDTH,
            linestyle="-",
            color=line_color,
            label=f"{cache_name.upper()} Cache",
        )

        # Дублируем на встроенном графике
        ax_inset.plot(
            fracs,
            accs,
            marker="o",
            markersize=MARKER_SIZE,
            linewidth=LINE_WIDTH,
            linestyle="-",
            color=line_color,
        )

    # Получаем чистое имя файла без расширения
    dataset_basename = os.path.basename(test_file)
    dataset_title, _ = os.path.splitext(dataset_basename)

    # 3. Настраиваем внешний вид основного графика
    ax.set_title(dataset_title, fontsize=14, fontweight="bold")
    ax.set_ylabel("Accuracy", fontsize=12, fontweight="bold")
    ax.set_xlabel("Cache fraction (size)", fontsize=12, fontweight="bold")
    ax.grid(True, linestyle="--", alpha=0.7)

    # Динамический максимум по Y для основного графика
    ax.set_ylim(0, Y_MAX_LIMIT)
    ax.legend()

    # НАСТРОЙКА МИНИ-ГРАФИКА (ВРЕЗКИ)
    # Автоматические границы: строго первая 1/8 часть графика по X
    ax_inset.set_xlim(0, X_MAX_ZOOM)

    # Для Y на врезке найдем максимум только среди тех точек, которые попали в первые 1/8 графика
    zoom_accs = []
    for res in all_results.values():
        for f, a in zip(res["fractions"], res["accuracies"]):
            if f <= X_MAX_ZOOM:
                zoom_accs.append(a)
    max_zoom_accuracy = max(zoom_accs) if zoom_accs else max_accuracy

    ax_inset.set_ylim(0, max_zoom_accuracy * 1.05)
    ax_inset.grid(True, linestyle=":", alpha=0.6)
    # ax_inset.set_title("Zoom (First 1/8)", fontsize=10, fontweight="bold")

    # 4. МАГИЯ ДЛЯ ОСИ X
    def custom_x_formatter(x, pos):
        percent = x * 100
        actual_size = int(x * total_keys)
        return f"{percent:.1f}%\n({actual_size})"

    def inset_x_formatter(x, pos):
        percent = x * 100
        actual_size = int(x * total_keys)
        # Используем 3 знака после запятой, так как масштаб стал крупнее
        return f"{percent:.3f}%\n({actual_size})"

    ax.xaxis.set_major_formatter(ticker.FuncFormatter(custom_x_formatter))
    ax_inset.xaxis.set_major_formatter(ticker.FuncFormatter(inset_x_formatter))
    ax_inset.tick_params(axis='both', which='major', labelsize=8)

    # Визуальный контур увеличения (соединяет область зума на главном графике с мини-графиком)
    ax.indicate_inset_zoom(ax_inset, edgecolor="gray")

    # 5. Оптимизация полезного пространства
    plt.subplots_adjust(bottom=0.12)
    plt.tight_layout()

    # 6. Сохраняем и показываем
    plt.savefig(args.plot, dpi=300, bbox_inches="tight")
    print(f"График успешно сохранен в файл '{args.plot}'")
    plt.show()


if __name__ == "__main__":
    main()

import subprocess
import matplotlib.pyplot as plt
import sys

# ГЛОБАЛЬНЫЙ ПАРАМЕТР: список кешей для тестирования
CACHES_TO_TEST = ["lru", "lfu"]

def run_cpp_benchmark(executable_path, cache_name, dataset_path):
    print(f"Запускаем {executable_path} для кеша {cache_name.upper()}...")
    try:
        # Передаем аргументы в C++ программу
        result = subprocess.run(
            [executable_path, cache_name, dataset_path],
            capture_output=True, text=True, check=True
        )
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Ошибка при выполнении C++ кода для {cache_name}!", file=sys.stderr)
        print(e.stderr, file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Файл {executable_path} не найден.", file=sys.stderr)
        sys.exit(1)

def parse_output(output):
    sizes = []
    accuracies = []
    for line in output.strip().split('\n'):
        parts = line.split('\t')
        if len(parts) == 2:
            try:
                sizes.append(int(parts[0]))
                accuracies.append(float(parts[1]))
            except ValueError:
                continue
    return sizes, accuracies

def main():
    executable_path = "./cmake-build-debug/Cache"
    # Убедитесь, что путь к датасету правильный
    test_file = "Datasets/1_synthetic/sample1.txt"

    all_results = {}
    max_size_overall = 0

    # 1. Собираем данные для всех кешей
    for cache_name in CACHES_TO_TEST:
        output = run_cpp_benchmark(executable_path, cache_name, test_file)
        sizes, accuracies = parse_output(output)

        if sizes:
            all_results[cache_name] = (sizes, accuracies)
            # Запоминаем максимальный размер по оси X среди всех кешей
            max_size_overall = max(max_size_overall, sizes[-1])
        else:
            print(f"Предупреждение: Нет данных для кеша {cache_name}")

    if not all_results:
        print("Нет данных для построения графика.")
        return

    # 2. Строим график
    plt.figure(figsize=(10, 6))

    # Набор цветов для разных линий
    colors = ['b', 'g', 'r', 'c', 'm', 'y', 'k']

    for i, cache_name in enumerate(CACHES_TO_TEST):
        if cache_name not in all_results:
            continue

        sizes, accuracies = all_results[cache_name]

        # ЛОГИКА НАСЫЩЕНИЯ:
        # Если этот кеш достиг 100% раньше других, добиваем его массивы
        # последним значением (1.0) вплоть до max_size_overall
        last_size = sizes[-1]
        last_accuracy = accuracies[-1]

        if last_size < max_size_overall:
            for s in range(last_size + 1, max_size_overall + 1):
                sizes.append(s)
                accuracies.append(last_accuracy)

        # Рисуем линию
        plt.plot(
            sizes, accuracies,
            marker='o', markersize=4, linestyle='-',
            color=colors[i % len(colors)],
            label=f'{cache_name.upper()} Cache'
        )

    # 3. Настраиваем внешний вид
    plt.title('Зависимость точности кешей от их размера', fontsize=14)
    plt.xlabel('Размер кеша (Cache Size)', fontsize=12)
    plt.ylabel('Точность (Accuracy)', fontsize=12)
    plt.grid(True, linestyle='--', alpha=0.7)
    plt.ylim(0, 1.05)
    plt.legend()

    # 4. Сохраняем и показываем
    plt.savefig('cache_accuracy_plot.png', dpi=300, bbox_inches='tight')
    print("График успешно сохранен в файл 'cache_accuracy_plot.png'")
    plt.show()

if __name__ == "__main__":
    main()

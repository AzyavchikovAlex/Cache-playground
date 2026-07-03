import subprocess
import matplotlib.pyplot as plt
import sys

def run_cpp_benchmark(executable_path):
    print(f"Запускаем {executable_path}...")
    try:
        # Запускаем C++ программу.
        # capture_output=True перехватывает std::cout
        # text=True возвращает вывод в виде строки, а не байтов
        result = subprocess.run([executable_path], capture_output=True, text=True, check=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print("Ошибка при выполнении C++ кода!", file=sys.stderr)
        print(e.stderr, file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"Файл {executable_path} не найден.", file=sys.stderr)
        print("Скомпилируйте проект в CLion перед запуском скрипта.", file=sys.stderr)
        sys.exit(1)

def parse_output(output):
    sizes = []
    accuracies = []

    # Разбиваем вывод на строки и парсим
    for line in output.strip().split('\n'):
        parts = line.split('\t')
        if len(parts) == 2:
            try:
                size = int(parts[0])
                accuracy = float(parts[1])
                sizes.append(size)
                accuracies.append(accuracy)
            except ValueError:
                # Игнорируем строки, которые не являются числами (например, логи)
                continue

    return sizes, accuracies

def main():
    # Путь к исполняемому файлу.
    # В CLion по умолчанию он лежит в cmake-build-debug
    executable_path = "./cmake-build-debug/Cache"

    # 1. Получаем данные от C++ программы
    output = run_cpp_benchmark(executable_path)

    # 2. Парсим данные
    sizes, accuracies = parse_output(output)

    if not sizes:
        print("Нет данных для построения графика. Проверьте вывод C++ программы.")
        return

    # 3. Строим график
    plt.figure(figsize=(10, 6)) # Размер окна

    # Рисуем линию с точками
    plt.plot(sizes, accuracies, marker='o', markersize=4, linestyle='-', color='b', label='LRU Cache')

    # Настраиваем внешний вид
    plt.title('Зависимость точности кеша от его размера', fontsize=14)
    plt.xlabel('Размер кеша (Cache Size)', fontsize=12)
    plt.ylabel('Точность (Accuracy)', fontsize=12)

    # Добавляем сетку для удобства чтения
    plt.grid(True, linestyle='--', alpha=0.7)

    # Ограничиваем ось Y от 0 до 1.05 (чтобы 1.0 не прилипало к самому верху)
    plt.ylim(0, 1.05)

    # Показываем легенду
    plt.legend()

    # 4. Сохраняем график в файл и показываем на экране
    plt.savefig('cache_accuracy_plot.png', dpi=300, bbox_inches='tight')
    print("График успешно сохранен в файл 'cache_accuracy_plot.png'")

    plt.show() # Откроет интерактивное окно с графиком

if __name__ == "__main__":
    main()
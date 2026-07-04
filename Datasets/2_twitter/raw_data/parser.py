import argparse
import csv
import sys
import os

def convert_twitter_trace(input_csv, output_txt, max_requests=None):
    """
    Конвертирует CSV трейс от Twitter в формат для C++ бенчмарка.
    Формат Twitter: timestamp, key, key_size, val_size, client_id, operation, TTL
    """
    keys = []

    print(f"Чтение данных из {input_csv}...")
    seen_keys = {}
    new_key_index = 0;
    with open(input_csv, 'r') as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 6:
                continue

            key = row[1]
            # convert large string to integer
            if key not in seen_keys:
                seen_keys[key] = new_key_index
                key = new_key_index
                new_key_index += 1
            else:
                key = seen_keys[key]
            operation = row[5]

            # Для тестирования кеша нас обычно интересуют запросы на чтение
            if operation in ('get', 'gets'):
                keys.append(key)

            # Ограничиваем размер датасета, если нужно
            if max_requests and len(keys) >= max_requests:
                break

    print(f"Собрано {len(keys)} ключей. Запись в {output_txt}...")

    with open(output_txt, 'w') as f:
        # Записываем заголовок, который ожидает ваш C++ код (ParseDataset)
        f.write(f"int {len(keys)}\n")

        # Записываем сами ключи
        for key in keys:
            f.write(f"{key}\n")

    print("Готово!")

if __name__ == "__main__":
    # 1. Скачайте любой sample из репозитория Twitter:
    # https://github.com/twitter/cache-trace/tree/master/samples/2020Mar
    # и положите рядом со скриптом под именем twitter_sample.csv

    parser = argparse.ArgumentParser(
        description="Конвертация трейсов кэша Twitter в текстовый датасет."
    )

    parser.add_argument(
        "--input",
        type=str,
        default="twitter_sample.csv",
        help="Путь к исходному CSV-файлу трейса (по умолчанию: twitter_sample.csv)"
    )

    parser.add_argument(
        "--output",
        type=str,
        default="twitter_sample_parsed.txt",
        help="Путь к итоговому TXT-файлу трейса (по умолчанию: twitter_sample_parsed.txt)"
    )

    args = parser.parse_args()

    input_file = args.input
    output_file = args.output

    if not os.path.exists(input_file):
        print(f"Файл {input_file} не найден.")
        print("Скачайте пример трейса из репозитория twitter/cache-trace")
    else:
        # Вытаскиваем первые 500,000 запросов для теста
        # convert_twitter_trace(input_file, output_file, max_requests=500000)
        convert_twitter_trace(input_file, output_file, max_requests=None)
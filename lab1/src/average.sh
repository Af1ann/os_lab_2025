#!/bin/bash

# Если передан один аргумент и это файл, считываем числа из файла
if [ $# -eq 1 ] && [ -f "$1" ]; then
  args=$(cat "$1")
else
  args="$@"
fi

# Проверяем, есть ли аргументы для вычисления
if [ -z "$args" ]; then
  echo "Аргументы не переданы"
  exit 1
fi

count=0
sum=0

# Считаем сумму и количество аргументов
for arg in $args
do
  sum=$((sum + arg))
  count=$((count + 1))
done

# Вычисляем среднее с помощью bc для точности
average=$(echo "scale=2; $sum / $count" | bc)

echo "Количество аргументов: $count"
echo "Среднее арифметическое: $average"

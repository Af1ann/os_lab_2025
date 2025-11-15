#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdbool.h>

// Функция умножения по модулю (для предотвращения переполнения)
uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod);

// Функция конвертации строки в uint64_t
bool ConvertStringToUI64(const char *str, uint64_t *val);

#endif // UTILS_H

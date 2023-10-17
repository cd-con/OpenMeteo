#pragma once
#include <Arduino.h>
#include <Vector.h>

struct FixedPoint{
    short value; // Целое число для реализации числа с фиксированной точкой
    short scale; // Шкала числа с фиксированной точкой

    short getIntegral(FixedPoint num) const {
        return value / (10 ^ scale); // Получение целой части числа с фиксированной точкой
    }

    short getFractional() const {
        return value % (10 ^ scale); // Получение дробной части числа с фиксированной точкой
    }

    short getFloatValue() const {
        return static_cast<double>(value) / (10 ^ scale); // Получение значения числа с плавающей точкой
    }
};


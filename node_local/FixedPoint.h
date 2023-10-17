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

    double getValue() const {
        return (double)value / pow(10.0, scale); // Получение значения числа с плавающей точкой
    }
};


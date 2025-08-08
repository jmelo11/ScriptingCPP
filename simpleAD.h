#pragma once

#include <cmath>

struct Tape {};

struct Number {
    double value;
    static Tape* tape;

    Number(double v = 0.0) : value(v) {}
    Number& operator=(double v) { value = v; return *this; }

    Number& operator+=(const Number& o) { value += o.value; return *this; }
    Number& operator-=(const Number& o) { value -= o.value; return *this; }
    Number& operator*=(const Number& o) { value *= o.value; return *this; }
    Number& operator/=(const Number& o) { value /= o.value; return *this; }

    friend Number operator+(Number lhs, const Number& rhs) { lhs += rhs; return lhs; }
    friend Number operator-(Number lhs, const Number& rhs) { lhs -= rhs; return lhs; }
    friend Number operator*(Number lhs, const Number& rhs) { lhs *= rhs; return lhs; }
    friend Number operator/(Number lhs, const Number& rhs) { lhs /= rhs; return lhs; }

    friend Number operator+(Number lhs, double rhs) { lhs.value += rhs; return lhs; }
    friend Number operator+(double lhs, Number rhs) { rhs.value += lhs; return rhs; }
    friend Number operator-(Number lhs, double rhs) { lhs.value -= rhs; return lhs; }
    friend Number operator-(double lhs, const Number& rhs) { return Number(lhs - rhs.value); }
    friend Number operator*(Number lhs, double rhs) { lhs.value *= rhs; return lhs; }
    friend Number operator*(double lhs, Number rhs) { rhs.value *= lhs; return rhs; }
    friend Number operator/(Number lhs, double rhs) { lhs.value /= rhs; return lhs; }
    friend Number operator/(double lhs, const Number& rhs) { return Number(lhs / rhs.value); }

    friend bool operator>(const Number& a, const Number& b) { return a.value > b.value; }
    friend bool operator<(const Number& a, const Number& b) { return a.value < b.value; }
    friend bool operator>=(const Number& a, const Number& b) { return a.value >= b.value; }
    friend bool operator<=(const Number& a, const Number& b) { return a.value <= b.value; }
    friend bool operator==(const Number& a, const Number& b) { return a.value == b.value; }
    friend bool operator!=(const Number& a, const Number& b) { return a.value != b.value; }

    friend bool operator>(const Number& a, double b) { return a.value > b; }
    friend bool operator>(double a, const Number& b) { return a > b.value; }
    friend bool operator<(const Number& a, double b) { return a.value < b; }
    friend bool operator<(double a, const Number& b) { return a < b.value; }

    void propagateToStart() {}
    double adjoint() const { return 0.0; }

    operator double() const { return value; }
};

inline Tape* Number::tape = nullptr;


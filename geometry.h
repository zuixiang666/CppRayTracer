#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <iostream>
#include <cassert>

template <class t> struct Vec3 {
    t x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(t xx, t yy, t zz) : x(xx), y(yy), z(zz) {}
    
    // ⚠️ 修复 1：增加一个只传两个参数的构造函数 (为了 Vec2f)
    Vec3(t xx, t yy) : x(xx), y(yy), z(0) {}
    
    // 向量加法
    Vec3<t> operator+(const Vec3<t> &v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
    
    // 向量减法
    Vec3<t> operator-(const Vec3<t> &v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
    
    // ⚠️ 修复 2：增加一元负号运算符 (为了 -light_dir 这种写法)
    Vec3<t> operator-() const { return Vec3<t>(-x, -y, -z); }
    
    // 向量点乘
    t operator*(const Vec3<t> &v) const { return x * v.x + y * v.y + z * v.z; }
    // 向量乘标量
    Vec3<t> operator*(const t &scalar) const { return Vec3<t>(x * scalar, y * scalar, z * scalar); }
    
    // [] 操作符
    t& operator[](size_t i) { if (i == 0) return x; if (i == 1) return y; return z; }
    const t& operator[](size_t i) const { if (i == 0) return x; if (i == 1) return y; return z; }
    
    // 向量长度平方
    t norm() const { return x * x + y * y + z * z; }
    
    // 归一化
    Vec3<t> normalize() const {
        t len = std::sqrt(norm());
        return Vec3<t>(x / len, y / len, z / len);
    }
};

typedef Vec3<float> Vec3f;
typedef Vec3<float> Vec2f;

#endif // GEOMETRY_H
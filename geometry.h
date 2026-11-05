#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cmath>
#include <iostream>

template <class t> struct Vec3 {
    t x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(t xx, t yy, t zz) : x(xx), y(yy), z(zz) {}
    
    // 向量加法
    Vec3<t> operator+(const Vec3<t> &v) const { return Vec3<t>(x + v.x, y + v.y, z + v.z); }
    // 向量减法 (求两点之间的方向)
    Vec3<t> operator-(const Vec3<t> &v) const { return Vec3<t>(x - v.x, y - v.y, z - v.z); }
    // 向量点乘 (用来算夹角和投影)
    t operator*(const Vec3<t> &v) const { return x * v.x + y * v.y + z * v.z; }
    // 求向量长度的平方
   
    t norm() const { return x * x + y * y + z * z; }
    
    // 新增：向量归一化 (把长度变成1)
    Vec3<t> normalize() const {
        t len = std::sqrt(norm());
        return Vec3<t>(x / len, y / len, z / len);
    }
};

typedef Vec3<float> Vec3f;

#endif // GEOMETRY_H
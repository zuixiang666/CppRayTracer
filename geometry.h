#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <cassert>
#include <cmath>
#include <iostream>

// 定义一个三维向量类，用来表示颜色 (r,g,b) 或者坐标 (x,y,z)
template <class t> struct Vec3 {
    t x, y, z;
    
    // 默认构造函数
    Vec3() : x(0), y(0), z(0) {}
    
    // 带参数的构造函数
    Vec3(t xx, t yy, t zz) : x(xx), y(yy), z(zz) {}
};

// 给 float 类型的 Vec3 起个别名，叫 Vec3f，这样打字短一点
typedef Vec3<float> Vec3f;

#endif // GEOMETRY_H
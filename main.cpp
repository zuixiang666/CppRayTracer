#include <iostream>
#include <fstream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include "geometry.h"

// 定义球体结构
struct Sphere {
    Vec3f center;
    float radius;

    Sphere(const Vec3f &c, const float &r) : center(c), radius(r) {}

    // 核心：检测光线是否与球相交 (这里就是截图里那段数学公式)
    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        Vec3f L = center - orig;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if (d2 > radius * radius) return false;
        float thc = sqrtf(radius * radius - d2);
        t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

void render() {
    const int width  = 1024;
    const int height = 768;
    
    std::vector<Vec3f> framebuffer(width * height);

    // 1. 定义一个球：球心在 (0, 0, -3)，半径为 1
    Sphere sphere(Vec3f(0, 0, -3), 1.0);

    // 2. 遍历每个像素
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            // 这里的计算是为了把屏幕上的像素坐标，转换成 3D 空间里光线的方向
            // 假设相机在 (0,0,0)，屏幕在 z = -1 的位置
            // 1. 先算出屏幕的宽高比
float aspect_ratio = (float)width / (float)height;

// 2. 在 x 的计算中乘以宽高比，修正横向的拉伸
float x = (2 * (i + 0.5) / (float)width - 1) * aspect_ratio * tan(M_PI / 6.0);
float y = -(2 * (j + 0.5) / (float)height - 1) * tan(M_PI / 6.0);
            
            Vec3f dir = Vec3f(x, y, -1).normalize(); 

            float t0; // 记录交点距离，暂时用不到
            
            // 3. 检测光线是否打中球
            if (sphere.ray_intersect(Vec3f(0, 0, 0), dir, t0)) {
                // 如果打中了，涂红色
                framebuffer[i + j * width] = Vec3f(1.0, 0.0, 0.0);
            } else {
                // 如果没打中，涂背景色（深灰色，方便看清球）
                framebuffer[i + j * width] = Vec3f(0.2, 0.2, 0.2); 
            }
        }
    }

    // 4. 保存为 PPM 图片 (千万别忘了 binary 模式！)
    std::ofstream ofs;
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].x)));
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].y)));
        ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i].z)));
    }
    ofs.close();
}

int main() {
    render();
    return 0;
}
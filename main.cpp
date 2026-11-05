#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits> // 需要用到 limits 来找最大值
#include "geometry.h"

// 定义球体结构
struct Sphere {
    Vec3f center;
    float radius;
    Vec3f color; // 新增：给每个球加上自己的颜色

    Sphere(const Vec3f &c, const float &r, const Vec3f &col) : center(c), radius(r), color(col) {}

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
    const float aspect_ratio = (float)width / (float)height;
    const float PI = 3.14159265358979323846f;
    
    std::vector<Vec3f> framebuffer(width * height);

    // 1. 创建一个包含多个球的列表（你可以随意修改它们的位置、半径和颜色）
    std::vector<Sphere> spheres;
    // 中间的蓝色球
    spheres.push_back(Sphere(Vec3f(0, 0, -4), 1.0, Vec3f(0.0, 0.5, 1.0)));
    // 左边的红色球
    spheres.push_back(Sphere(Vec3f(-1.5, 0, -5), 0.8, Vec3f(1.0, 0.2, 0.2)));
    // 右边的绿色球
    spheres.push_back(Sphere(Vec3f(1.8, -0.5, -6), 1.5, Vec3f(0.2, 0.8, 0.2)));

    // 2. 遍历每个像素
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * aspect_ratio * tan(PI / 6.0);
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(PI / 6.0);
            Vec3f dir = Vec3f(x, y, -1).normalize(); 

            float closest_t = std::numeric_limits<float>::max(); // 初始化最近距离为无穷大
            int hit_index = -1; // 记录打中了第几个球，-1 表示没打中

            // 3. 检查光线和每一个球的相交情况
            for (size_t k = 0; k < spheres.size(); k++) {
                float t0;
                if (spheres[k].ray_intersect(Vec3f(0, 0, 0), dir, t0)) {
                    // 如果打中了，且距离比之前找到的更近，就更新记录
                    if (t0 < closest_t) {
                        closest_t = t0;
                        hit_index = k;
                    }
                }
            }

            // 4. 根据结果上色
            if (hit_index != -1) {
                // 如果打中了球，涂上对应球的颜色
                framebuffer[i + j * width] = spheres[hit_index].color;
            } else {
                // 如果啥都没打中，涂深灰色背景
                framebuffer[i + j * width] = Vec3f(0.2, 0.2, 0.2); 
            }
        }
    }

    // 保存文件
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
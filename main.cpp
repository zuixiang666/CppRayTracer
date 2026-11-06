#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include "geometry.h"

// 1. 升级材质：增加了高光指数和反射率
struct Material {
    Material(const Vec2f &a, const Vec3f &color, const float &spec) : albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : albedo(1, 0), diffuse_color(), specular_exponent() {}
    Vec2f albedo;             // 反射率 (x: 漫反射比例, y: 镜面高光比例)
    Vec3f diffuse_color;      // 漫反射颜色
    float specular_exponent;  // 高光指数 (越大高光越集中)
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f &c, const float &r, const Material &m) : center(c), radius(r), material(m) {}

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

struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

// 2. 新增：计算光线反射方向的函数
Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N * 2.0f * (I * N);
}

// 3. 升级渲染函数：加入高光计算
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    float closest_t = std::numeric_limits<float>::max();
    int hit_index = -1;

    for (size_t k = 0; k < spheres.size(); k++) {
        float t0;
        if (spheres[k].ray_intersect(orig, dir, t0)) {
            if (t0 < closest_t) {
                closest_t = t0;
                hit_index = k;
            }
        }
    }

    if (hit_index == -1) {
        return Vec3f(0.2, 0.7, 0.8); // 背景色改成了好看的天蓝色
    }

    Vec3f point = orig + dir * closest_t;
    Vec3f N = (point - spheres[hit_index].center).normalize();
    Material material = spheres[hit_index].material;

    float diffuse_light_intensity = 0;
    float specular_light_intensity = 0; // 新增：高光强度

    for (size_t i = 0; i < lights.size(); i++) {
        Vec3f light_dir = (lights[i].position - point).normalize();

        // 漫反射
        diffuse_light_intensity += lights[i].intensity * std::max(0.0f, light_dir * N);
        
        // 镜面高光：计算反射光与视线方向（-dir）的夹角
        specular_light_intensity += powf(std::max(0.0f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity;
    }

    // 最终颜色 = 漫反射颜色 * 漫反射强度 * 比例 + 高光白色 * 高光强度 * 比例
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + 
           Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width  = 1024;
    const int height = 768;
    const float aspect_ratio = (float)width / (float)height;
    const float PI = 3.14159265358979323846f;
    
    std::vector<Vec3f> framebuffer(width * height);

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * aspect_ratio * tan(PI / 6.0);
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(PI / 6.0);
            Vec3f dir = Vec3f(x, y, -1).normalize(); 
            framebuffer[i + j * width] = cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
        Vec3f &c = framebuffer[i];
        
        // 4. 新增：色调映射 (防止过曝变纯白)
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1) c = c * (1. / max);
        
        for (size_t j = 0; j < 3; ++j) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
    // 创建材质：albedo(漫反射比例, 高光比例), 颜色, 高光指数
    Material ivory(Vec2f(0.6, 0.3), Vec3f(0.4, 0.4, 0.3), 50.);
    Material red_rubber(Vec2f(0.9, 0.1), Vec3f(0.3, 0.1, 0.1), 10.);
    Material blue(Vec2f(0.6, 0.3), Vec3f(0.1, 0.2, 0.8), 50.);

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 3, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
    spheres.push_back(Sphere(Vec3f( -6.0, -0.5, -18), 3, blue)); 

    // 添加三个光源
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

    render(spheres, lights);
    return 0;
}
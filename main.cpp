#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include<cstdlib>
#include "geometry.h"

struct Material {
    Material(const float &r, const Vec4f &a, const Vec3f &color, const float &spec) : refractive_index(r), albedo(a), diffuse_color(color), specular_exponent(spec) {}
    Material() : refractive_index(1), albedo(1,0,0,0), diffuse_color(), specular_exponent() {}
    float refractive_index;
    Vec4f albedo;
    Vec3f diffuse_color;
    float specular_exponent;
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
struct Box {
    Vec3f min, max;
    Material material;

    Box(const Vec3f &m1, const Vec3f &m2, const Material &m) : min(m1), max(m2), material(m) {}

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        float tx1 = (min.x - orig.x) / (dir.x == 0 ? 1e-6 : dir.x);
        float tx2 = (max.x - orig.x) / (dir.x == 0 ? 1e-6 : dir.x);
        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);

        float ty1 = (min.y - orig.y) / (dir.y == 0 ? 1e-6 : dir.y);
        float ty2 = (max.y - orig.y) / (dir.y == 0 ? 1e-6 : dir.y);
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float tz1 = (min.z - orig.z) / (dir.z == 0 ? 1e-6 : dir.z);
        float tz2 = (max.z - orig.z) / (dir.z == 0 ? 1e-6 : dir.z);
        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        if (tmax < 0 || tmin > tmax) return false;
        t0 = tmin < 0 ? tmax : tmin;
        return true;
    }
};
struct Cylinder {
    Vec3f center;     // 圆柱体的中心点 (Y轴的中心)
    float radius;     // 半径
    float half_height;// 高度的一半 (从中心到盖子的距离)
    Material material;

    Cylinder(const Vec3f &c, float r, float h, const Material &m) 
        : center(c), radius(r), half_height(h), material(m) {}

    // 专门用于求光线与圆盘（盖子）的交点
    bool intersect_disk(const Vec3f &orig, const Vec3f &dir, float disk_y, float &t) const {
        if (fabs(dir.y) < 1e-6) return false; // 光线平行于盖子
        t = (disk_y - orig.y) / dir.y;
        if (t < 0) return false; // 交点在背后
        
        // 检查交点是否在圆盘半径内
        Vec3f p = orig + dir * t;
        return (p.x - center.x)*(p.x - center.x) + (p.z - center.z)*(p.z - center.z) <= radius * radius;
    }

    bool ray_intersect(const Vec3f &orig, const Vec3f &dir, float &t0) const {
        float min_y = center.y - half_height;
        float max_y = center.y + half_height;
        float closest_t = std::numeric_limits<float>::max();
        bool hit = false;

        // 1. 检测侧面（无限长圆柱逻辑）
        Vec3f oc = orig - center;
        float a = dir.x * dir.x + dir.z * dir.z;
        float b = 2.0f * (oc.x * dir.x + oc.z * dir.z);
        float c = oc.x * oc.x + oc.z * oc.z - radius * radius;
        float delta = b * b - 4 * a * c;

        if (delta > 0) {
            float t1 = (-b - sqrtf(delta)) / (2.0f * a);
            float t2 = (-b + sqrtf(delta)) / (2.0f * a);
            // 检查侧面的交点 Y 坐标是否在高度范围内
            if (t1 > 0 && (orig.y + dir.y * t1) >= min_y && (orig.y + dir.y * t1) <= max_y) {
                closest_t = t1; hit = true;
            }
            if (t2 > 0 && (orig.y + dir.y * t2) >= min_y && (orig.y + dir.y * t2) <= max_y) {
                if (t2 < closest_t) { closest_t = t2; hit = true; }
            }
        }

        // 2. 检测下底面
        float t_bottom;
        if (intersect_disk(orig, dir, min_y, t_bottom) && t_bottom < closest_t) {
            closest_t = t_bottom; hit = true;
        }

        // 3. 检测上顶面
        float t_top;
        if (intersect_disk(orig, dir, max_y, t_top) && t_top < closest_t) {
            closest_t = t_top; hit = true;
        }

        if (hit) t0 = closest_t;
        return hit;
    }
};

// 全局变量
std::vector<Cylinder> cylinders;
Material cyl_mat(1.0, Vec4f(0.9, 0.1, 0.0, 0.0), Vec3f(0.7, 0.5, 0.5), 10.);
// 漫反射0.7，高光0.3，反射率0.1（有一点点反光，但绝不透明）
Material my_box_mat(1.0, Vec4f(0.7, 0.3, 0.1, 0.0), Vec3f(0.9, 0.7, 0.4), 150.);
std::vector<Box> boxes; 
struct Light {
    Light(const Vec3f &p, const float &i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

Vec3f reflect(const Vec3f &I, const Vec3f &N) {
    return I - N * 2.0f * (I * N);
}
Vec3f refract(const Vec3f &I, const Vec3f &N, const float &refractive_index) { // Snell's law
    float cosi = - std::max(-1.f, std::min(1.f, I*N));
    float etai = 1, etat = refractive_index;
    Vec3f n = N;
    if (cosi < 0) { // if the ray is inside the object, swap the indices and invert the normal to get the correct result
        cosi = -cosi;
        std::swap(etai, etat); n = -N;
    }
    float eta = etai / etat;
    float k = 1 - eta*eta*(1 - cosi*cosi);
    return k < 0 ? Vec3f(0,0,0) : I*eta + n*(eta * cosi - sqrtf(k));
}
bool scene_intersect(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, Vec3f &hit, Vec3f &N, Material &material) {
    float spheres_dist = std::numeric_limits<float>::max();
    for (size_t i = 0; i < spheres.size(); i++) {
        float dist_i;
        if (spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            hit = orig + dir * dist_i;
            N = (hit - spheres[i].center).normalize();
            material = spheres[i].material;
        }
    }
        for (size_t i = 0; i < boxes.size(); i++) {
        float box_dist = std::numeric_limits<float>::max();
        if (boxes[i].ray_intersect(orig, dir, box_dist) && box_dist < spheres_dist) {
            spheres_dist = box_dist;
            hit = orig + dir * box_dist;
            
            Vec3f center = (boxes[i].min + boxes[i].max) * 0.5;
            Vec3f d = hit - center;
            if (fabs(d.x) > fabs(d.y) && fabs(d.x) > fabs(d.z)) N = Vec3f(d.x > 0 ? 1 : -1, 0, 0);
            else if (fabs(d.y) > fabs(d.z)) N = Vec3f(0, d.y > 0 ? 1 : -1, 0);
            else N = Vec3f(0, 0, d.z > 0 ? 1 : -1);

            material = boxes[i].material;
        }
    }
        for (size_t i = 0; i < cylinders.size(); i++) {
        float cyl_dist = std::numeric_limits<float>::max();
        if (cylinders[i].ray_intersect(orig, dir, cyl_dist) && cyl_dist < spheres_dist) {
            spheres_dist = cyl_dist;
            hit = orig + dir * cyl_dist;
            
            // ⚠️ 新的法线计算逻辑
            float min_y = cylinders[i].center.y - cylinders[i].half_height;
            float max_y = cylinders[i].center.y + cylinders[i].half_height;
            
            // 判断打中的是盖子还是侧面（用极小的误差值判断）
            if (fabs(hit.y - min_y) < 1e-4) {
                N = Vec3f(0, -1, 0); // 打中下底面，法线朝下
            } else if (fabs(hit.y - max_y) < 1e-4) {
                N = Vec3f(0, 1, 0);  // 打中上顶面，法线朝上
            } else {
                // 打中侧面
                Vec3f local_hit = hit - cylinders[i].center;
                N = Vec3f(local_hit.x, 0, local_hit.z).normalize();
            }
            
            material = cylinders[i].material;
        }
    }
    float checkerboard_dist = std::numeric_limits<float>::max();
    if (fabs(dir.y)>1e-3)  {
        float d = -(orig.y+4)/dir.y; // the checkerboard plane has equation y = -4
        Vec3f pt = orig + dir*d;
        if (d>0 && fabs(pt.x)<10 && pt.z<-10 && pt.z>-30 && d<spheres_dist) {
            checkerboard_dist = d;
            hit = pt;
            N = Vec3f(0,1,0);
            material.diffuse_color = (int(.5*hit.x+1000) + int(.5*hit.z)) & 1 ? Vec3f(1,1,1) : Vec3f(1, .7, .3);
            material.diffuse_color = material.diffuse_color*.3;
        }
    }
    return std::min(spheres_dist, checkerboard_dist)<1000;
}

// 2. 核心改动：cast_ray 增加了一个递归深度 depth 参数，默认值为 0
Vec3f cast_ray(const Vec3f &orig, const Vec3f &dir, const std::vector<Sphere> &spheres, const std::vector<Light> &lights, size_t depth = 0) {
    Vec3f point, N;
    Material material;

    if (depth > 3 || !scene_intersect(orig, dir, spheres, point, N, material)) {
        return Vec3f(0.2, 0.7, 0.8); // 背景色
    }

    float diffuse_light_intensity = 0, specular_light_intensity = 0;

    for (size_t i = 0; i < lights.size(); i++) {
        Vec3f light_dir = (lights[i].position - point).normalize();
        float light_distance = (lights[i].position - point).norm();

        Vec3f shadow_orig = light_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; 
        Vec3f shadow_pt, shadow_N;
        Material tmpmaterial;

        if (scene_intersect(shadow_orig, light_dir, spheres, shadow_pt, shadow_N, tmpmaterial) && (shadow_pt - shadow_orig).norm() < light_distance) {
            continue;
        }

        diffuse_light_intensity += lights[i].intensity * std::max(0.0f, light_dir * N);
        specular_light_intensity += powf(std::max(0.0f, -reflect(-light_dir, N) * dir), material.specular_exponent) * lights[i].intensity;
    }

    // 3. 核心新增：计算反射方向，并递归调用 cast_ray 去找反射的颜色
    Vec3f reflect_dir = reflect(dir, N).normalize();
    Vec3f refract_dir = refract(dir, N, material.refractive_index).normalize();
    Vec3f reflect_orig = reflect_dir * N < 0 ? point - N * 1e-3 : point + N * 1e-3; // 同样需要偏移防伪影
    Vec3f refract_orig = refract_dir*N < 0 ? point - N*1e-3 : point + N*1e-3;
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);
    Vec3f refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1); // 折射颜色

    // 4. 颜色混合：漫反射色 + 高光色 + 反射色
    return material.diffuse_color * diffuse_light_intensity * material.albedo[0] + 
           Vec3f(1., 1., 1.) * specular_light_intensity * material.albedo[1] +
           reflect_color * material.albedo[2] + refract_color * material.albedo[3];
}

void render(const std::vector<Sphere> &spheres, const std::vector<Light> &lights) {
    const int width  = 1024;
    const int height = 768;
    const float aspect_ratio = (float)width / (float)height;
    const float PI = 3.14159265358979323846f;
    
    std::vector<Vec3f> framebuffer(width * height);

        // 循环遍历每个像素
    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            
            Vec3f final_color(0, 0, 0); 
            int samples = 64; // ⚠️ 采样次数：4意味着 2x2=4 条光线（想要更平滑可以改成 16，但会变慢）
            int sqrt_samples = (int)sqrt(samples);

            // 在一个像素内部，再分成几份去采样
            for (int m = 0; m < sqrt_samples; m++) {
                for (int n = 0; n < sqrt_samples; n++) {
                    // 计算偏移量，不再是死死盯住中心 0.5，而是在像素内部随机游走
                    float dx = (m + (float)rand() / RAND_MAX) / sqrt_samples;
                    float dy = (n + (float)rand() / RAND_MAX) / sqrt_samples;

                    float x = (2 * (i + dx) / (float)width - 1) * aspect_ratio * tan(PI / 6.0);
                    float y = -(2 * (j + dy) / (float)height - 1) * tan(PI / 6.0);
                    Vec3f dir = Vec3f(x, y, -1).normalize(); 
                    
                    // 把这 4 条光线的颜色加起来
                    final_color = final_color + cast_ray(Vec3f(0, 0, 0), dir, spheres, lights);
                }
            }
            
            // 取平均值，存入画板
            framebuffer[i + j * width] = final_color * (1.0f / samples);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm", std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
        Vec3f &c = framebuffer[i];
        float max = std::max(c[0], std::max(c[1], c[2]));
        if (max > 1) c = c * (1. / max);
        for (size_t j = 0; j < 3; ++j) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, framebuffer[i][j])));
        }
    }
    ofs.close();
}

int main() {
        // 1. 材质调优：让玻璃更透亮，高光更强
    // 参数：[漫反射比例, 高光比例, 反射比例], 颜色, 高光指数
    Material      ivory(1.0, Vec4f(0.6,  0.3, 0.1, 0.0), Vec3f(0.4, 0.4, 0.3),   50.);
    Material      glass(1.5, Vec4f(0.0,  0.5, 0.1, 0.8), Vec3f(0.6, 0.7, 0.8),  125.);
    Material red_rubber(1.0, Vec4f(0.9,  0.1, 0.0, 0.0), Vec3f(0.3, 0.1, 0.1),   10.);
    Material     mirror(1.0, Vec4f(0.0, 10.0, 0.8, 0.0), Vec3f(1.0, 1.0, 1.0), 1425.);
    std::vector<Sphere> spheres;
    
    cylinders.push_back(Cylinder(Vec3f(-5.5f, -1.0f, -14.0f), 1.2f, 2.0f, cyl_mat));
    boxes.push_back(Box(Vec3f(2.0, -2.0, -12.0), Vec3f(5.0, 1.0, -9.0), my_box_mat));
    spheres.push_back(Sphere(Vec3f(-3,    0,   -16), 2,      ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2,      glass));
    spheres.push_back(Sphere(Vec3f( 1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f( 7,    5,   -18), 4,     mirror));

    // 3. 光源位置（不变）
    std::vector<Light> lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));

    render(spheres, lights);
    return 0;
}
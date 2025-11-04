#include "hw2.h"
#include "hw2_scenes.h"

using namespace hw2;
inline Real cross2(const Vector2& a, const Vector2& b) {
    return a.x * b.y - a.y * b.x;
}

Vector3 compute_barycentric_2d(const Vector2 &p, const Vector2 &p0, const Vector2 &p1, const Vector2 &p2) {
    Vector2 e0 = p1 - p0;
    Vector2 e1 = p2 - p0;
    Real denom = cross2(e0, e1);  

    Real b1 = cross2(p - p0, e1) / denom;   
    Real b2 = cross2(e0, p - p0) / denom;   
    Real b0 = Real(1) - b1 - b2;           
    return Vector3(b0, b1, b2);
}

bool inside_triangle(const Vector3 &bary) {
    return bary.x >= 0 && bary.y >= 0 && bary.z >= 0;
}

std::vector<Vector3> clip_triangle_against_near(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, Real z_near) {
    std::vector<Vector3> result;
    bool inside0 = (p0.z <= -z_near);
    bool inside1 = (p1.z <= -z_near);
    bool inside2 = (p2.z <= -z_near);
    
    int num_inside = inside0 + inside1 + inside2;
    auto intersect = [z_near](const Vector3 &a, const Vector3 &b) -> Vector3 {
        Real t = (-z_near - a.z) / (b.z - a.z);
        return a + (b - a) * t;
    };
    if (num_inside == 3) {
        result = {p0, p1, p2};
    }
    else if (num_inside == 2) {
        Vector3 in1, in2, out;
        if (!inside0) {
            out = p0; in1 = p1; in2 = p2;
        }
        else if (!inside1) { 
            out = p1; in1 = p2; in2 = p0; 
        }
        else { 
            out = p2; in1 = p0; in2 = p1; 
        }
        Vector3 i1 = intersect(out, in1);
        Vector3 i2 = intersect(out, in2);
        result = {in1, in2, i1, i1, in2, i2};   
    }
    else if (num_inside == 1) {
        Vector3 in, out1, out2;
        if (inside0) { 
            in = p0; out1 = p1; out2 = p2; 
        }
        else if (inside1) { 
            in = p1; out1 = p2; out2 = p0; 
        }
        else { 
            in = p2; out1 = p0; out2 = p1; 
        }
        Vector3 i1 = intersect(in, out1);
        Vector3 i2 = intersect(in, out2);
        result = {in, i1, i2};
    }
    
    return result;
}

Image3 hw_2_1(const std::vector<std::string> &params) {
    // Homework 2.1: render a single 3D triangle

    Image3 img(640 /* width */, 480 /* height */);

    Vector3 p0{0, 0, -1};
    Vector3 p1{1, 0, -1};
    Vector3 p2{0, 1, -1};
    Real s = 1; // scaling factor of the view frustrum
    Vector3 color = Vector3{1.0, 0.5, 0.5};
    Real z_near = 1e-6; // distance of the near clipping plane
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-p0") {
            p0.x = std::stof(params[++i]);
            p0.y = std::stof(params[++i]);
            p0.z = std::stof(params[++i]);
        } else if (params[i] == "-p1") {
            p1.x = std::stof(params[++i]);
            p1.y = std::stof(params[++i]);
            p1.z = std::stof(params[++i]);
        } else if (params[i] == "-p2") {
            p2.x = std::stof(params[++i]);
            p2.y = std::stof(params[++i]);
            p2.z = std::stof(params[++i]);
        } else if (params[i] == "-color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            color = Vector3{r, g, b};
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        }
    }
    std::vector<Vector3> clipped_vertices = clip_triangle_against_near(p0, p1, p2, z_near);
    Image3 supersampled(img.width * 4, img.height * 4);

    // Bkg
    for (int y = 0; y < img.height * 4; y++) {
        for (int x = 0; x < img.width * 4; x++) {
            supersampled(x, y) = Vector3{0.5, 0.5, 0.5};
        }
    }

    if (clipped_vertices.size() >= 3) {
        Real aspect_ratio = Real(img.width) / Real(img.height);
        auto project_to_screen = [&](const Vector3 &p) {
            Vector2 proj;
            Vector2 screen;
            proj.x = -p.x / p.z;
            proj.y = -p.y / p.z;
        
            screen.x = Real(img.width) * (proj.x + s * aspect_ratio) / (2.0 * s * aspect_ratio);
            screen.y = Real(img.height) * (s - proj.y) / (2.0 * s);
            return screen;
        };

        auto rasterize_triangle = [&](const Vector2 &a, const Vector2 &b, const Vector2 &c) {
            for (int y = 0; y < img.height * 4; y++) {
                for (int x = 0; x < img.width * 4; x++) {
                    Vector3 bary = compute_barycentric_2d(Vector2(Real(x) + 0.5, Real(y) + 0.5), a * Real(4), b * Real(4), c * Real(4));
                    if (inside_triangle(bary)) {
                        supersampled(x, y) = color;
                    }
                }
            }
        };

        std::vector<Vector2> projected;
        for (const Vector3 &v : clipped_vertices) {
            projected.push_back(project_to_screen(v));
        }
        if (clipped_vertices.size() == 3) {
            std::cout << "Render only 1 triangle" << std::endl;
            rasterize_triangle(projected[0], projected[1], projected[2]);
        } else if (clipped_vertices.size() == 6) {
            std::cout << "Render quad as 2 triangles" << std::endl;
            rasterize_triangle(projected[0], projected[1], projected[2]);
            rasterize_triangle(projected[3], projected[4], projected[5]);
        }
    }
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 sum(0, 0, 0);
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    sum += supersampled(x * 4 + dx, y * 4 + dy);
                }
            }
            img(x, y) = sum / Real(16);
        }
    }
    return img;
}

Image3 hw_2_2(const std::vector<std::string> &params) {
    // Homework 2.2: render a triangle mesh

    Image3 img(640 /* width */, 480 /* height */);

    Real s = 1; // scaling factor of the view frustrum
    Real z_near = 1e-6; // distance of the near clipping plane
    int scene_id = 0;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        } else if (params[i] == "-scene_id") {
            scene_id = std::stoi(params[++i]);
        }
    }

    TriangleMesh mesh = meshes[scene_id];
    
    Image3 supersampled(img.width * 4, img.height * 4);
    std::vector<Real> zbuffer((size_t)img.width * (size_t)img.height * 16, std::numeric_limits<Real>::infinity());
    for (int y = 0; y < img.height * 4; ++y) {
        for (int x = 0; x < img.width * 4; ++x) {
            supersampled(x, y) = Vector3(0.5, 0.5, 0.5);
        }
    }

    Real aspect = (Real)img.width / (Real)img.height;

    // Project a point in camera space to projected cam space 
    auto cam_to_projected_cam = [](const Vector3 &p) -> Vector2 {
        return Vector2{-p.x / p.z, -p.y / p.z};
    };

    auto screen_to_projected_cam = [&](const Vector2 &scr) -> Vector2 {
        Real x = (2.0 * s * aspect) * (scr.x / (Real)img.width) - s * aspect;
        Real y = s - (2.0 * s) * (scr.y / (Real)img.height);
        return Vector2{x, y};
    };

    for (size_t f = 0; f < mesh.faces.size(); ++f) {
        Vector3i tri = mesh.faces[f];
        Vector3 col = mesh.face_colors[f]; 
        const Vector3 &p0 = mesh.vertices[tri.x];
        const Vector3 &p1 = mesh.vertices[tri.y];
        const Vector3 &p2 = mesh.vertices[tri.z];
        if (p0.z > -z_near || p1.z > -z_near || p2.z > -z_near)
            continue;

        Vector2 p0p = cam_to_projected_cam(p0);
        Vector2 p1p = cam_to_projected_cam(p1);
        Vector2 p2p = cam_to_projected_cam(p2);
        for (int y = 0; y < img.height * 4; ++y) {
            for (int x = 0; x < img.width * 4; ++x) {
                Vector2 scr((Real(x) + 0.5) / 4, (Real(y) + 0.5) / 4);
                Vector2 pp = screen_to_projected_cam(scr);
                Vector3 b_prime = compute_barycentric_2d(pp, p0p, p1p, p2p);
                if (!inside_triangle(b_prime)) 
                    continue;
                // Convert to original barycentrics 
                Real invz0 = 1.0 / p0.z, invz1 = 1.0 / p1.z, invz2 = 1.0 / p2.z;
                Real denom_b = b_prime.x * invz0 + b_prime.y * invz1 + b_prime.z * invz2;
                Real b0 = (b_prime.x * invz0) / denom_b, b1 = (b_prime.y * invz1) / denom_b, b2 = (b_prime.z * invz2) / denom_b;
                Real z_val = b0 * p0.z + b1 * p1.z + b2 * p2.z; 
                if (-z_val < z_near)
                    continue; 
                Real depth = -z_val;
                size_t idx = (size_t)y * (size_t)img.width * 4 + (size_t)x;
                if (depth < zbuffer[idx]) {
                    zbuffer[idx] = depth;
                    supersampled(x, y) = col;
                }
            }
        }
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 sum(0, 0, 0);
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    sum += supersampled(x * 4 + dx, y * 4 + dy);
                }
            }
            img(x, y) = sum / Real(16);
        }
    }
    return img;
}

Image3 hw_2_3(const std::vector<std::string> &params) {
    // Homework 2.3: render a triangle mesh with vertex colors

    Image3 img(640 /* width */, 480 /* height */);

    Real s = 1; // scaling factor of the view frustrum
    Real z_near = 1e-6; // distance of the near clipping plane
    int scene_id = 0;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-s") {
            s = std::stof(params[++i]);
        } else if (params[i] == "-znear") {
            z_near = std::stof(params[++i]);
        } else if (params[i] == "-scene_id") {
            scene_id = std::stoi(params[++i]);
        }
    }

    TriangleMesh mesh = meshes[scene_id];
    Image3 supersampled_img(img.width * 4, img.height * 4);
    std::vector<Real> zbuffer((size_t)img.width * (size_t)img.height * 16, std::numeric_limits<Real>::infinity());

    for (int y = 0; y < img.height * 4; ++y) {
        for (int x = 0; x < img.width * 4; ++x) {
            supersampled_img(x, y) = Vector3(0.5, 0.5, 0.5);
        }
    }
    Real aspect = (Real)img.width / (Real)img.height;
    auto cam_to_projected_cam = [](const Vector3 &p) -> Vector2 {
        return Vector2{-p.x / p.z, -p.y / p.z};
    };

    auto screen_to_projected_cam = [&](const Vector2 &scr) -> Vector2 {
        Real x = (2.0 * s * aspect) * (scr.x / (Real)img.width) - s * aspect;
        Real y = s - (2.0 * s) * (scr.y / (Real)img.height);
        return Vector2{x, y};
    };

    for (size_t f = 0; f < mesh.faces.size(); ++f) {
        Vector3i tri = mesh.faces[f];
        const Vector3 &p0 = mesh.vertices[tri.x];
        const Vector3 &p1 = mesh.vertices[tri.y];
        const Vector3 &p2 = mesh.vertices[tri.z];
        const Vector3 &C0 = mesh.vertex_colors[tri.x];
        const Vector3 &C1 = mesh.vertex_colors[tri.y];
        const Vector3 &C2 = mesh.vertex_colors[tri.z];
        if (p0.z > -z_near || p1.z > -z_near || p2.z > -z_near)
            continue;

        Vector2 p0p = cam_to_projected_cam(p0);
        Vector2 p1p = cam_to_projected_cam(p1);
        Vector2 p2p = cam_to_projected_cam(p2);
        for (int y = 0; y < img.height * 4; ++y) {
            for (int x = 0; x < img.width * 4; ++x) {
                Vector2 scr((Real(x) + 0.5) / 4, (Real(y) + 0.5) / 4);
                Vector2 pp = screen_to_projected_cam(scr);
                Vector3 b_prime = compute_barycentric_2d(pp, p0p, p1p, p2p);
                if (!inside_triangle(b_prime))
                    continue;
                // Convert to original barycentrics
                Real invz0 = 1.0 / p0.z, invz1 = 1.0 / p1.z, invz2 = 1.0 / p2.z;
                Real denom_b = b_prime.x * invz0 + b_prime.y * invz1 + b_prime.z * invz2;
                Real b0 = (b_prime.x * invz0) / denom_b, b1 = (b_prime.y * invz1) / denom_b, b2 = (b_prime.z * invz2) / denom_b;
                Real z_val = b0 * p0.z + b1 * p1.z + b2 * p2.z; 
                if (-z_val < z_near)
                    continue; 
                Real depth = -z_val;
                size_t idx = (size_t)y * (size_t)img.width * 4 + (size_t)x;
                if (depth < zbuffer[idx]) {
                    zbuffer[idx] = depth;
                    Vector3 color = b0 * C0 + b1 * C1 + b2 * C2;
                    supersampled_img(x, y) = color;
                }
            }
        }
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 sum(0, 0, 0);
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    sum += supersampled_img(x * 4 + dx, y * 4 + dy);
                }
            }
            img(x, y) = sum / Real(16);
        }
    }
    return img;
}

Image3 hw_2_4(const std::vector<std::string> &params) {
    // Homework 2.4: render a scene with transformation
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.camera.resolution.x,
               scene.camera.resolution.y);

    Real s = scene.camera.s;
    Real z_near = scene.camera.z_near;
    Image3 supersampled(img.width * 4, img.height * 4);
    std::vector<Real> zbuffer((size_t)img.width * (size_t)img.height * 16, std::numeric_limits<Real>::infinity());
    const Vector3 background = scene.background;
    for (int y = 0; y < img.height * 4; ++y) {
        for (int x = 0; x < img.width * 4; ++x) {
            supersampled(x, y) = background;
        }
    }

    const Real aspect = (Real)img.width / (Real)img.height;
    auto invert_cam_to_world = [](const Matrix4x4 &T) {
        Matrix4x4 M = Matrix4x4::identity();
        M(0,0) = T(0,0); M(0,1) = T(1,0); M(0,2) = T(2,0); 
        M(1,0) = T(0,1); M(1,1) = T(1,1); M(1,2) = T(2,1); 
        M(2,0) = T(0,2); M(2,1) = T(1,2); M(2,2) = T(2,2); 
        M(3,0) = 0; M(3,1) = 0; M(3,2) = 0; M(3,3) = 1;
        Vector3 tp{
            -(M(0,0) * T(0,3) + M(0,1) * T(1,3) + M(0,2) * T(2,3)),
            -(M(1,0) * T(0,3) + M(1,1) * T(1,3) + M(1,2) * T(2,3)),
            -(M(2,0) * T(0,3) + M(2,1) * T(1,3) + M(2,2) * T(2,3))
        };
        M(0,3) = tp.x; M(1,3) = tp.y; M(2,3) = tp.z;
        return M;
    };

    auto transform_point = [](const Matrix4x4 &M, const Vector3 &p) -> Vector3 {
        Real x = M(0,0) * p.x + M(0,1) * p.y + M(0,2) * p.z + M(0,3);
        Real y = M(1,0) * p.x + M(1,1) * p.y + M(1,2) * p.z + M(1,3);
        Real z = M(2,0) * p.x + M(2,1) * p.y + M(2,2) * p.z + M(2,3);
        Real w = M(3,0) * p.x + M(3,1) * p.y + M(3,2) * p.z + M(3,3);
        if (std::abs(w) > 1e-30) {
            x /= w; 
            y /= w; 
            z /= w; 
        }
        return Vector3{x, y, z};
    };
    Matrix4x4 world_to_cam = invert_cam_to_world(scene.camera.cam_to_world);
    auto cam_to_projected_cam = [](const Vector3 &p) -> Vector2 {
        return Vector2{-p.x / p.z, -p.y / p.z};
    };
    auto screen_to_projected_cam = [&](const Vector2 &scr) -> Vector2 {
        Real x = (2.0 * s * aspect) * (scr.x / (Real)img.width) - s * aspect;
        Real y = s - (2.0 * s) * (scr.y / (Real)img.height);
        return Vector2{x, y};
    };

    for (const auto &mesh : scene.meshes) {
        Matrix4x4 M = world_to_cam * mesh.model_matrix;
        for (size_t f = 0; f < mesh.faces.size(); ++f) {
            Vector3i tri = mesh.faces[f];
            const Vector3 &C0 = mesh.vertex_colors[tri.x];
            const Vector3 &C1 = mesh.vertex_colors[tri.y];
            const Vector3 &C2 = mesh.vertex_colors[tri.z];
            Vector3 p0_cam = transform_point(M, mesh.vertices[tri.x]);
            Vector3 p1_cam = transform_point(M, mesh.vertices[tri.y]);
            Vector3 p2_cam = transform_point(M, mesh.vertices[tri.z]);
            if (!(p0_cam.z < 0 && p1_cam.z < 0 && p2_cam.z < 0)) {
                continue;
            }

            Vector2 p0p = cam_to_projected_cam(p0_cam);
            Vector2 p1p = cam_to_projected_cam(p1_cam);
            Vector2 p2p = cam_to_projected_cam(p2_cam);
            for (int ys = 0; ys < img.height * 4; ++ys) {
                for (int xs = 0; xs < img.width * 4; ++xs) {
                    Vector2 scr((Real(xs) + 0.5) / 4, (Real(ys) + 0.5) / 4);
                    Vector2 pp = screen_to_projected_cam(scr);
                    Vector3 b_prime = compute_barycentric_2d(pp, p0p, p1p, p2p);
                    if (!inside_triangle(b_prime)) 
                        continue;
                    // original barycentrics
                    Real invz0 = 1.0 / p0_cam.z, invz1 = 1.0 / p1_cam.z, invz2 = 1.0 / p2_cam.z;
                    Real denom_b = b_prime.x * invz0 + b_prime.y * invz1 + b_prime.z * invz2;
                    Real b0 = (b_prime.x * invz0) / denom_b, b1 = (b_prime.y * invz1) / denom_b, b2 = (b_prime.z * invz2) / denom_b;
                    Real z_val = b0 * p0_cam.z + b1 * p1_cam.z + b2 * p2_cam.z;
                    if (-z_val < z_near) 
                        continue; 

                    Real depth = -z_val; 
                    size_t idx = (size_t)ys * (size_t)img.width * 4 + (size_t)xs;
                    if (depth < zbuffer[idx]) {
                        zbuffer[idx] = depth;
                        Vector3 color = b0 * C0 + b1 * C1 + b2 * C2;
                        supersampled(xs, ys) = color;
                    }
                }
            }
        }
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 sum(0, 0, 0);
            for (int dy = 0; dy < 4; dy++) {
                for (int dx = 0; dx < 4; dx++) {
                    sum += supersampled(x * 4 + dx, y * 4 + dy);
                }
            }
            img(x, y) = sum / Real(16);
        }
    }

    return img;
}

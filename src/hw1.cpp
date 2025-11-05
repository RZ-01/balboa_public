#include "hw1.h"
#include "hw1_scenes.h"
#include <algorithm>
#include "timer.h"
#include <omp.h>
#include <fstream>
#include "3rdparty/json.hpp"

using json = nlohmann::json;
using namespace hw1;

inline state rayintersects(Real x, Real y, Real x0, Real y0, Real x1, Real y1){
    Real dy = y1 - y0;
    if (abs(dy) < 1e-12)
        return state::None;
    Real inv_dy = 1.0 / dy;
    Real t = (y - y0) * inv_dy;
    if (t < 0.0 || t > 1.0)
        return state::None; 
    Real s = x0 - x + t * (x1 - x0);
    if (s < 0)
        return state::None;
    if (dy > 0)
        return state::Up;
    return state::Down;

}

void render_circle(Image3 &img, const Vector2 &center, Real radius, std::optional<Vector3> fill_color, const std::optional<Vector3> &stroke_color, Real stroke_width, const std::optional<Matrix3x3> &trans) {
    Real outer_radius = radius + stroke_width / Real(2);
    Real inner_radius = radius - stroke_width / Real(2);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Real canvas_x = x + Real(0.5);
            Real canvas_y = img.height - y - Real(0.5);
            if(trans){
                Matrix3x3 inv_trans = inverse(trans.value());
                Vector3 transformed = inv_trans * Vector3{canvas_x, canvas_y, 1.0};
                canvas_x = transformed.x;
                canvas_y = transformed.y;
            }
            Real dist_x = canvas_x - center.x;
            Real dist_y = canvas_y - center.y;
            Real dist = sqrt(dist_x * dist_x + dist_y * dist_y);
            if (fill_color && dist < radius)
                img(x, y) = fill_color.value();
            if (stroke_color && dist <= outer_radius && dist >= inner_radius){
                img(x, y) = stroke_color.value();
            }
               
        }
    }
}

void render_circle_antialiasing(Image3 &img, const Vector2 &center, Real radius, std::optional<Vector3> fill_color, const std::optional<Vector3> &stroke_color, 
                                Real stroke_width, const std::optional<Matrix3x3> &trans, int antialiasing, std::optional<Real> fill_alpha, std::optional<Real> stroke_alpha) {
    assert(antialiasing >= 1);
    Real outer_radius = radius + stroke_width / Real(2);
    Real inner_radius = radius - stroke_width / Real(2);
    Real s = antialiasing;
    // Real step = 1.0 / s;
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 acc_color = {0.0, 0.0, 0.0};
            for (int i = 0; i < s; i++) {
                for (int j = 0; j < s; j++) {
                    Real offset_x = (i + 0.5) / s;  
                    Real offset_y = (j + 0.5) / s;
                    Real canvas_x = x + offset_x;
                    Real canvas_y = img.height - y - offset_y;
                    if(trans){
                        Matrix3x3 inv_trans = inverse(trans.value());
                        Vector3 transformed = inv_trans * Vector3{canvas_x, canvas_y, 1.0};
                        canvas_x = transformed.x;
                        canvas_y = transformed.y;
                    }
                    Real dist_x = canvas_x - center.x;
                    Real dist_y = canvas_y - center.y;
                    Real dist = sqrt(dist_x * dist_x + dist_y * dist_y);
                    Vector3 subpixel_color = img(x, y); 
                    if (fill_color && dist < radius)
                        subpixel_color = fill_color.value() * (fill_alpha.value_or(1.0)) + subpixel_color * (1 - fill_alpha.value_or(1.0));
                    if (stroke_color && dist <= outer_radius && dist >= inner_radius)
                        subpixel_color = stroke_color.value() * (stroke_alpha.value_or(1.0)) + subpixel_color * (1 - stroke_alpha.value_or(1.0));

                    acc_color += subpixel_color;
                }
            }
            img(x, y) = acc_color / (s * s);
        }
    }
}


void render_polyline_antialiasing(Image3 &img, const std::vector<Vector2> &polyline, bool is_closed, 
                                const std::optional<Vector3> &fill_color, const std::optional<Vector3> &stroke_color, Real stroke_width, 
                                std::optional<Matrix3x3> trans, int antialiasing, std::optional<Real> fill_alpha, std::optional<Real> stroke_alpha) {
    assert(antialiasing >= 1);
    Real s = antialiasing;
    //Real step = 1.0 / s;
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Vector3 acc_color = {0.0, 0.0, 0.0};
            for (int i = 0; i < s; i++) {
                for (int j = 0; j < s; j++) {
                    Real offset_x = (i + 0.5) / s;  
                    Real offset_y = (j + 0.5) / s;
                    Real canvas_x = x + offset_x;
                    Real canvas_y = img.height - y - offset_y;
                    if(trans){
                        Matrix3x3 inv_trans = inverse(trans.value());
                        Vector3 transformed = inv_trans * Vector3{canvas_x, canvas_y, 1.0};
                        canvas_x = transformed.x;
                        canvas_y = transformed.y;
                    }
                    Vector2 q = {canvas_x, canvas_y};
                    Vector3 subpixel_color = img(x, y);

                    if(fill_color){
                        int count=0;
                        for (size_t pos = 0; pos < polyline.size(); pos++){
                            size_t next_pos = (pos + 1) % polyline.size();
                            state currentstate = rayintersects(canvas_x, canvas_y, polyline[pos].x, polyline[pos].y, polyline[next_pos].x, polyline[next_pos].y);
                            switch (currentstate)
                            {
                                case state::None:
                                    continue;
                                case state::Up:
                                    count++;
                                    break;
                                case state::Down:
                                    count--;
                                    break;
                                default:
                                    break;
                            }
                        }
                        if (count != 0)
                            subpixel_color = fill_color.value() * (fill_alpha.value_or(1.0)) + subpixel_color * (1 - fill_alpha.value_or(1.0));
                    }
                    if (stroke_color) {
                        bool stroked = false;
                        size_t n = is_closed ? polyline.size() : polyline.size() - 1;

                        for (size_t pos = 0; pos < n; ++pos) {
                            size_t next_pos = is_closed ? (pos + 1) % polyline.size() : pos + 1;
                            Real l = dot(polyline[next_pos] - polyline[pos], q - polyline[pos]) / length(polyline[next_pos] - polyline[pos]);
                            Vector2 q_prime = polyline[pos] + l * normalize(polyline[next_pos] - polyline[pos]);
                            if (l > 0 && l < length(polyline[next_pos] - polyline[pos]) && stroke_width > 2 * length(q - q_prime)) {
                                stroked = true;
                                break;
                            }
                        }

                        for (size_t i = 0; i < polyline.size(); ++i) {
                            if (!is_closed && (i == 0 || i == polyline.size() - 1)) 
                                continue;
                            if (stroke_width > 2 * length(q - polyline[i])) {
                                stroked = true;
                            }
                        }

                        if (stroked) {
                            Real alpha = stroke_alpha.value_or(1.0);
                            subpixel_color = stroke_color.value() * alpha + subpixel_color * (1 - alpha);
                        }
                    }
                    acc_color += subpixel_color;
                }
            }
            img(x, y) = acc_color / (s * s);
        }
    }
}


Vector2 evaluate_bezier(const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, Real t) {
    return (1.0 - t) * (1.0 - t) * p0 + 2.0 * (1.0 - t) * t * p1 + t * t * p2;
}

void render_bezier(Image3 &img, const Vector2 &p0, const Vector2 &p1, const Vector2 &p2,
                   const std::optional<Vector3> &stroke_color, Real stroke_width,
                   const std::optional<Matrix3x3> &trans) {
    
    const int num_samples = 2000;
    std::vector<Vector2> curve_points;

    // for performance
    curve_points.reserve(num_samples + 1);
    
    for (int i = 0; i <= num_samples; i++) {
        Real t = Real(i) / Real(num_samples);
        curve_points.push_back(evaluate_bezier(p0, p1, p2, t));
    }
    
    Real half = stroke_width / 2.0;
    
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Real canvas_x = x + Real(0.5);
            Real canvas_y = img.height - (y + Real(0.5));
            
            // no transform at this time
            if (trans) {
                Matrix3x3 inv_trans = inverse(trans.value());
                Vector3 transformed = inv_trans * Vector3{canvas_x, canvas_y, 1.0};
                canvas_x = transformed.x;
                canvas_y = transformed.y;
            }
            
            Vector2 pixel_pos{canvas_x, canvas_y};
            
            Real min_dist_2 = std::numeric_limits<Real>::infinity();
            for (const auto &curve_pt : curve_points) {
                Real dx = pixel_pos.x - curve_pt.x;
                Real dy = pixel_pos.y - curve_pt.y;
                Real dist_2 = dx * dx + dy * dy;
                min_dist_2 = std::min(min_dist_2, dist_2);
            }

            if (sqrt(min_dist_2) <= half) {
                img(x, y) = stroke_color.value();
            }
        }
    }
}
   

void render_polyline(Image3 &img, const std::vector<Vector2> &polyline, bool is_closed, const std::optional<Vector3> &fill_color, const std::optional<Vector3> &stroke_color, Real stroke_width, std::optional<Matrix3x3> trans) {
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            Real canvas_x = x + Real(0.5);
            Real canvas_y = img.height - y - Real(0.5);
            if(trans){
                Matrix3x3 inv_trans = inverse(trans.value());
                Vector3 transformed = inv_trans * Vector3{canvas_x, canvas_y, 1.0};
                canvas_x = transformed.x;
                canvas_y = transformed.y;
            }
            Vector2 q = {canvas_x, canvas_y};

            if(fill_color){
                int count=0;
                for (size_t pos = 0; pos < polyline.size(); pos++){
                    size_t next_pos = (pos + 1) % polyline.size();
                    state currentstate = rayintersects(canvas_x, canvas_y, polyline[pos].x, polyline[pos].y, polyline[next_pos].x, polyline[next_pos].y);
                    switch (currentstate)
                    {
                        case state::None:
                            continue;
                        case state::Up:
                            count++;
                            break;
                        case state::Down:
                            count--;
                            break;
                        default:
                            break;
                    }
                }
                if (count != 0)
                    img(x, y) = fill_color.value();
            }
            if(stroke_color){
                size_t n = is_closed ? polyline.size() : polyline.size() - 1;
                for (size_t pos = 0; pos < n; pos++){
                    size_t next_pos = is_closed ? (pos + 1) % polyline.size() : pos + 1;
                    Real l = dot(polyline[next_pos] - polyline[pos], q - polyline[pos]) / length(polyline[next_pos] - polyline[pos]);
                    Vector2 q_prime = polyline[pos] + l * normalize(polyline[next_pos] - polyline[pos]);
                    if (l > 0 && l < length(polyline[next_pos] - polyline[pos]) && stroke_width > 2 * length(q - q_prime))
                        img(x, y) = stroke_color.value();
                }

                for (size_t i = 0; i < polyline.size(); i++)
                {
                    if(!is_closed && (i == 0 || i == polyline.size() - 1))
                        continue;

                    if(stroke_width > 2 * length(q - polyline[i]))
                        img(x, y) = stroke_color.value();
                }
                
            }
        }
    }
}

void render_circle_fast(Image3 &img, const Vector2 &center, Real radius, std::optional<Vector3> fill_color, const std::optional<Vector3> &stroke_color, Real stroke_width, const std::optional<Matrix3x3> &trans, const boundingbox &bbox) {
    Real outer_radius = radius + stroke_width / Real(2);
    Real inner_radius = radius - stroke_width / Real(2);
    Real radius_sq = radius * radius;
    Real outer_radius_sq = outer_radius * outer_radius;
    Real inner_radius_sq = inner_radius * inner_radius;
    
    std::optional<Matrix3x3> inv_trans = std::nullopt;
    if(trans) {
        inv_trans = inverse(trans.value());
    }

    // compute integer, clamped loop bounds for OpenMP (must be integer "controlling predicate")
    int y0 = std::max(0, static_cast<int>(std::floor(bbox.min.y)));
    int y1 = std::min(static_cast<int>(img.height), static_cast<int>(std::ceil(bbox.max.y)));
    int x0 = std::max(0, static_cast<int>(std::floor(bbox.min.x)));
    int x1 = std::min(static_cast<int>(img.width), static_cast<int>(std::ceil(bbox.max.x)));

    #pragma omp parallel for schedule(guided)
    for (int y = y0; y < y1; y++) {
        for (int x = x0; x < x1; x++) {
            Real canvas_x = x + Real(0.5);
            Real canvas_y = img.height - y - Real(0.5);
            if(inv_trans){
                Vector3 transformed = inv_trans.value() * Vector3{canvas_x, canvas_y, 1.0};
                canvas_x = transformed.x;
                canvas_y = transformed.y;
            }
            Real dist_x = canvas_x - center.x;
            Real dist_y = canvas_y - center.y;
            Real dist_sq = dist_x * dist_x + dist_y * dist_y;
            if (fill_color && dist_sq < radius_sq)
                img(x, y) = fill_color.value();
            if (stroke_color && dist_sq <= outer_radius_sq && dist_sq >= inner_radius_sq){
                img(x, y) = stroke_color.value();
            }
               
        }
    }
}
void render_polyline_fast(Image3 &img, const std::vector<Vector2> &polyline, bool is_closed, const std::optional<Vector3> &fill_color, const std::optional<Vector3> &stroke_color, Real stroke_width, std::optional<Matrix3x3> trans, const boundingbox &bbox) {
    
    std::optional<Matrix3x3> inv_trans = std::nullopt;
    if(trans) {
        inv_trans = inverse(trans.value());
    }
    // compute integer, clamped loop bounds for OpenMP (must be integer "controlling predicate")
    int py0 = std::max(0, static_cast<int>(std::floor(bbox.min.y)));
    int py1 = std::min(static_cast<int>(img.height), static_cast<int>(std::ceil(bbox.max.y)));
    int px0 = std::max(0, static_cast<int>(std::floor(bbox.min.x)));
    int px1 = std::min(static_cast<int>(img.width), static_cast<int>(std::ceil(bbox.max.x)));

    #pragma omp parallel for schedule(dynamic)
    for (int y = py0; y < py1; y++) {
        for (int x = px0; x < px1; x++) {
            Real canvas_x = x + Real(0.5);
            Real canvas_y = img.height - y - Real(0.5);
            if(inv_trans){
                Vector3 transformed = inv_trans.value() * Vector3{canvas_x, canvas_y, 1.0};
                canvas_x = transformed.x;
                canvas_y = transformed.y;
            }
            Vector2 q = {canvas_x, canvas_y};

            if(fill_color){
                int count=0;
                for (size_t pos = 0; pos < polyline.size(); pos++){
                    size_t next_pos = (pos + 1) % polyline.size();
                    state currentstate = rayintersects(canvas_x, canvas_y, polyline[pos].x, polyline[pos].y, polyline[next_pos].x, polyline[next_pos].y);
                    switch (currentstate)
                    {
                        case state::None:
                            continue;
                        case state::Up:
                            count++;
                            break;
                        case state::Down:
                            count--;
                            break;
                        default:
                            break;
                    }
                }
                if (count != 0)
                    img(x, y) = fill_color.value();
            }
            if(stroke_color){
                size_t n = is_closed ? polyline.size() : polyline.size() - 1;
                for (size_t pos = 0; pos < n; pos++){
                    size_t next_pos = is_closed ? (pos + 1) % polyline.size() : pos + 1;
                    Real l = dot(polyline[next_pos] - polyline[pos], q - polyline[pos]) / length(polyline[next_pos] - polyline[pos]);
                    Vector2 q_prime = polyline[pos] + l * normalize(polyline[next_pos] - polyline[pos]);
                    if (l > 0 && l < length(polyline[next_pos] - polyline[pos]) && stroke_width > 2 * length(q - q_prime))
                        img(x, y) = stroke_color.value();
                }

                for (size_t i = 0; i < polyline.size(); i++)
                {
                    if(!is_closed && (i == 0 || i == polyline.size() - 1))
                        continue;

                    if(stroke_width > 2 * length(q - polyline[i]))
                        img(x, y) = stroke_color.value();
                }
                
            }
        }
    }
}


Image3 hw_1_1(const std::vector<std::string> &params) {
    // Homework 1.1: render a circle at the specified
    // position, with the specified radius and color.

    Image3 img(640 /* width */, 480 /* height */);

    Vector2 center = Vector2{img.width / 2 + Real(0.5), img.height / 2 + Real(0.5)};
    Real radius = 100.0;
    Vector3 color = Vector3{1.0, 0.5, 0.5};
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-center") {
            Real x = std::stof(params[++i]);
            Real y = std::stof(params[++i]);
            center = Vector2{x, y};
        } else if (params[i] == "-radius") {
            radius = std::stof(params[++i]);
        } else if (params[i] == "-color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            color = Vector3{r, g, b};
        }
    }
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = Vector3{0.5, 0.5, 0.5};
        }
    }
    render_circle(img, center, radius, color, std::nullopt, 1.0, std::nullopt);
    return img;
}

Image3 hw_1_2(const std::vector<std::string> &params) {
    // Homework 1.2: render polylines
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Image3 img(640 /* width */, 480 /* height */);
    std::vector<Vector2> polyline;
    // is_closed = true indicates that the last point and
    // the first point of the polyline are connected
    bool is_closed = false;
    std::optional<Vector3> fill_color;
    std::optional<Vector3> stroke_color;
    Real stroke_width = 1;
    for (int i = 0; i < (int)params.size(); i++) {
        if (params[i] == "-points") {
            while (params.size() > i+1 &&
                    params[i+1].length() > 0 &&
                    params[i+1][0] != '-') {
                Real x = std::stof(params[++i]);
                Real y = std::stof(params[++i]);
                polyline.push_back(Vector2{x, y});
            }
        } else if (params[i] == "--closed") {
            is_closed = true;
        } else if (params[i] == "-fill_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            fill_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_color") {
            Real r = std::stof(params[++i]);
            Real g = std::stof(params[++i]);
            Real b = std::stof(params[++i]);
            stroke_color = Vector3{r, g, b};
        } else if (params[i] == "-stroke_width") {
            stroke_width = std::stof(params[++i]);
        }
    }
    // silence warnings, feel free to remove it
    UNUSED(stroke_width);

    if (fill_color && !is_closed) {
        std::cout << "Error: can't have a non-closed shape with fill color." << std::endl;
        return Image3(0, 0);
    }

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = Vector3{0.5, 0.5, 0.5};
        }
    } 

    render_polyline(img, polyline, is_closed, fill_color, stroke_color, stroke_width, std::nullopt);
    return img;
}

Image3 hw_1_3(const std::vector<std::string> &params) {
    // Homework 1.3: render multiple shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }

    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto circle = std::get_if<Circle>(&shape)){
            render_circle(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, std::nullopt);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            render_polyline(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, std::nullopt);
        } 
    }
    return img;
}

Image3 hw_1_4(const std::vector<std::string> &params) {
    // Homework 1.4: render transformed shapes
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto circle = std::get_if<Circle>(&shape)){
            render_circle(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, circle->transform);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            render_polyline(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, polyline->transform);
        } 
    }
    return img;
}

Image3 hw_1_5(const std::vector<std::string> &params) {
    // Homework 1.5: antialiasing
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto circle = std::get_if<Circle>(&shape)){
            render_circle_antialiasing(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, circle->transform, 4, std::nullopt, std::nullopt);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            render_polyline_antialiasing(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, polyline->transform, 4, std::nullopt, std::nullopt);
        } 
    }
    return img;
}

Image3 hw_1_6(const std::vector<std::string> &params) {
    // Homework 1.6: alpha blending
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto circle = std::get_if<Circle>(&shape)){
            render_circle_antialiasing(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, circle->transform, 8, circle->fill_alpha, circle->stroke_alpha);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            render_polyline_antialiasing(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, polyline->transform, 8, polyline->fill_alpha, polyline->stroke_alpha);
        } 
    }
    return img;
}

Image3 hw_1_7(const std::vector<std::string> &params) {
    // Bonus for render quadratic Bezier curves
    if (params.size() == 0) {
        return Image3(0, 0);
    }

    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;

    Image3 img(scene.resolution.x, scene.resolution.y);

    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto bezier = std::get_if<BezierCurve>(&shape)){
            render_bezier(img, bezier->p0, bezier->p1, bezier->p2, bezier->stroke_color, bezier->stroke_width, bezier->transform);
        } else if(auto circle = std::get_if<Circle>(&shape)){
            render_circle(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, circle->transform);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            render_polyline(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, polyline->transform);
        }
    }
    return img;
}

Image3 render_scene(const Scene &scene) {
    Image3 img(scene.resolution.x, scene.resolution.y);

    #pragma omp parallel for schedule(static)
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            img(x, y) = scene.background;
        }
    }
    for(auto it = scene.shapes.rbegin(); it != scene.shapes.rend(); ++it){
        auto &shape = *it;
        if(auto circle = std::get_if<Circle>(&shape)){
            Real outer_r = circle->radius + circle->stroke_width / 2.0;
            Vector2 geom_min = circle->center - Vector2(outer_r, outer_r);
            Vector2 geom_max = circle->center + Vector2(outer_r, outer_r);

            // convert to pixel coordinates
            int x_min = std::max(0, static_cast<int>(std::floor(geom_min.x - 0.5)));
            int x_max = std::min(img.width, static_cast<int>(std::ceil(geom_max.x + 0.5)));
            int y_min = std::max(0, static_cast<int>(std::floor(img.height - geom_max.y - 0.5)));
            int y_max = std::min(img.height, static_cast<int>(std::ceil(img.height - geom_min.y + 0.5)));

            boundingbox bbox;
            bbox.min = Vector2(x_min, y_min);
            bbox.max = Vector2(x_max, y_max);
            
            render_circle_fast(img, circle->center, circle->radius, circle->fill_color, circle->stroke_color, circle->stroke_width, circle->transform, bbox);
        } else if(auto polyline = std::get_if<Polyline>(&shape)){
            Vector2 geom_min = polyline->points[0];
            Vector2 geom_max = polyline->points[0];
            for (auto &p : polyline->points) {
                geom_min = min(geom_min, p);
                geom_max = max(geom_max, p);
            }
            Real expand = polyline->stroke_width / 2.0;
            geom_min = geom_min - Vector2(expand, expand);
            geom_max = geom_max + Vector2(expand, expand);

            int x_min = std::max(0, static_cast<int>(std::floor(geom_min.x - 0.5)));
            int x_max = std::min(img.width, static_cast<int>(std::ceil(geom_max.x + 0.5)));
            int y_min = std::max(0, static_cast<int>(std::floor(img.height - geom_max.y - 0.5)));
            int y_max = std::min(img.height, static_cast<int>(std::ceil(img.height - geom_min.y + 0.5)));
            boundingbox bbox;
            bbox.min = Vector2(x_min, y_min);
            bbox.max = Vector2(x_max, y_max);
            render_polyline_fast(img, polyline->points, polyline->is_closed, polyline->fill_color, polyline->stroke_color, polyline->stroke_width, polyline->transform, bbox);
        }
    }
    return img;
}

Image3 hw_1_8(const std::vector<std::string> &params) {
    // Bonus for Performance optimization
    if (params.size() == 0) {
        return Image3(0, 0);
    }
    omp_set_num_threads(omp_get_max_threads());

    Timer timer;
    tick(timer);
    std::cout << "Parsing and constructing scene " << params[0] << "." << std::endl;
    Scene scene = parse_scene(params[0]);
    std::cout << "Done. Took " << tick(timer) << " seconds." << std::endl;
    std::cout << "Rendering..." << std::endl;
    Image3 img = render_scene(scene);
    std::cout << "Done. Took " << tick(timer) << " seconds." << std::endl;
    return img;
}

std::vector<Image3> animation(const std::vector<std::string> &params) {
    // Animation
    if (params.size() == 0) {
        return {};
    }

    std::ifstream f(params[0]);
    json scenes_array = json::parse(f);
    
    std::vector<Image3> frames;
    for (const auto& scene_json : scenes_array) {
        Scene scene;
        auto res = scene_json["resolution"];
        scene.resolution = Vector2i{res[0], res[1]};
        auto bg = scene_json["background"];
        scene.background = Vector3{bg[0], bg[1], bg[2]};
        
        for (const auto& obj : scene_json["objects"]) {
            if (obj["type"] == "circle") {
                Circle circle;
                auto c = obj["center"];
                circle.center = Vector2{c[0], c[1]};
                circle.radius = obj["radius"];
                if (obj.contains("fill_color")) {
                    auto fc = obj["fill_color"];
                    circle.fill_color = Vector3{fc[0], fc[1], fc[2]};
                }
                if (obj.contains("stroke_color")) {
                    auto sc = obj["stroke_color"];
                    circle.stroke_color = Vector3{sc[0], sc[1], sc[2]};
                }
                circle.stroke_width = obj.value("stroke_width", 1.0);
                circle.transform = Matrix3x3::identity();
                scene.shapes.push_back(circle);
            }
        }
        
        frames.push_back(render_scene(scene));
    }
    
    return frames;
}
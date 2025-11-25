#include "hw3.h"
#include "3rdparty/glad.h" // needs to be included before GLFW!
#include "3rdparty/glfw/include/GLFW/glfw3.h"
#include "hw3_scenes.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <image.h>


using namespace hw3;

static glm::mat4 to_glm_mat4(const Matrix4x4f &matrix) {
    glm::mat4 out(1.0f);
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            out[col][row] = static_cast<float>(matrix(row, col));
        }
    }
    return out;
}
void framebuffer_size_callback0(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 
void processInput0(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) 
        glfwSetWindowShouldClose(window, true);
}
void hw_3_1(const std::vector<std::string> &params) {

    // HW 3.1: Open a window using GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "hw_3_1", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }    
    glViewport(0, 0, 800, 600);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback0);  
    while(!glfwWindowShouldClose(window))
    {
        processInput0(window);

        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    glfwTerminate();

    return;
    
}

void hw_3_2(const std::vector<std::string> &params) {
    // HW 3.2: Render a single 2D triangle
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(600, 600, "hw_3_1", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }    

    Shader ourShader("../src/3_2_shader.vs", "../src/3_2_shader.fs");

    float vertices[] = {
        // positions         // colors
        0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
        0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
    }; 
    
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO); 
    glGenBuffers(1, &VBO);  
    glBindVertexArray(VAO);
    
    // vertex buffer obj
    glBindBuffer(GL_ARRAY_BUFFER, VBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3* sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind it
    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    // unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO
    glBindVertexArray(0); 

    while(!glfwWindowShouldClose(window))
    {
        processInput0(window);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height); 
        float aspect = (float)width / (float)height;
        glm::mat4 projection = glm::ortho(-aspect, aspect, -1.0f, 1.0f);

        glClearColor(0.6f, 0.8f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();
        float timeValue = glfwGetTime();
        float angle = glm::radians(timeValue * 30.0f); // 60 degrees per second

        glm::mat4 rotation_matrix = glm::mat4(1.0f);
        rotation_matrix = glm::translate(rotation_matrix, glm::vec3(0.0f, -0.5f, 0.0f));
        rotation_matrix = glm::rotate(rotation_matrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
        rotation_matrix = glm::translate(rotation_matrix, glm::vec3(0.0f, 0.5f, 0.0f));

        ourShader.setMat4("rotation_matrix", glm::value_ptr(rotation_matrix));
        ourShader.setMat4("projection", glm::value_ptr(projection));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

}


struct CameraControl {
    glm::mat4 base_cam_to_world{1.0f};
    float yaw = 0.0f;
    float pitch = 0.0f;
    double lastX = 0.0;
    double lastY = 0.0;
    bool firstMouse = true;
    float sensitivity = 0.1f;
    float moveSpeed = 0.05f;
};

static glm::mat4 rotation_from_yaw_pitch(float yaw_deg, float pitch_deg) {
    glm::mat4 yaw_mat = glm::rotate(glm::mat4(1.0f), glm::radians(yaw_deg), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 pitch_mat = glm::rotate(glm::mat4(1.0f), glm::radians(pitch_deg), glm::vec3(1.0f, 0.0f, 0.0f));
    return yaw_mat * pitch_mat;
}

static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    auto state = static_cast<CameraControl*>(glfwGetWindowUserPointer(window));
    if (!state) {
        return;
    }
    if (state->firstMouse) {
        state->lastX = xpos;
        state->lastY = ypos;
        state->firstMouse = false;
        return;
    }
    double xoffset = xpos - state->lastX;
    double yoffset = state->lastY - ypos;
    state->lastX = xpos;
    state->lastY = ypos;
    
    // form += to -=, or it will be reversed 
    state->yaw -= static_cast<float>(xoffset) * state->sensitivity;
    state->pitch += static_cast<float>(yoffset) * state->sensitivity;
    state->pitch = std::clamp(state->pitch, -89.0f, 89.0f);
}

static void processInput(GLFWwindow* window, CameraControl* camState) {
    if (!camState) 
        return;
    glm::mat4 cam_to_world = camState->base_cam_to_world * rotation_from_yaw_pitch(camState->yaw, camState->pitch);
    glm::vec3 right = glm::vec3(cam_to_world[0]);    
    glm::vec3 forward = -glm::vec3(cam_to_world[2]); 
    float velocity = camState->moveSpeed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camState->base_cam_to_world[3] += glm::vec4(forward * velocity, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) 
        camState->base_cam_to_world[3] -= glm::vec4(forward * velocity, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
        camState->base_cam_to_world[3] -= glm::vec4(right * velocity, 0.0f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) 
        camState->base_cam_to_world[3] += glm::vec4(right * velocity, 0.0f);
}

void hw_3_3(const std::vector<std::string> &params) {
    // HW 3.3: Render a scene
    if (params.size() == 0) {
        return;
    }
    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int height = scene.camera.resolution[1];
    int width = scene.camera.resolution[0];
    GLFWwindow* window = glfwCreateWindow(width, height, "hw_3_3", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Shader shader("../src/hw3_3.vs", "../src/hw3_3.fs");

    struct OpenGLMesh {
        unsigned int vao, vbo_v, vbo_c, vbo_t, ebo;
        int nFaces;
        glm::mat4 model;
        unsigned int texture;
    };
    std::vector<OpenGLMesh> meshGLs;
    for (auto &mesh : scene.meshes) {
        OpenGLMesh m{};
        glGenVertexArrays(1, &m.vao);
        glBindVertexArray(m.vao);

        //Vertex position
        std::vector<glm::vec3> positions;
        for (auto &v : mesh.vertices)
            positions.emplace_back(v[0], v[1], v[2]);

        glGenBuffers(1, &m.vbo_v);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_v);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Vertex colors
        std::vector<glm::vec3> colors;
        if (!mesh.vertex_colors.empty()) {
            for (auto &c : mesh.vertex_colors)
                colors.emplace_back(c[0], c[1], c[2]);
        } else {
            for (size_t i = 0; i < mesh.vertices.size(); ++i)
                colors.emplace_back(1.0f, 1.0f, 1.0f);
        }

        glGenBuffers(1, &m.vbo_c);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_c);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        m.texture = 0; 
        if (!mesh.uvs.empty()) {
            std::vector<glm::vec2> texcoords;
            for (auto &uv : mesh.uvs) {
                texcoords.emplace_back(uv[0], 1.0f - uv[1]);
            }

            glGenBuffers(1, &m.vbo_t);
            glBindBuffer(GL_ARRAY_BUFFER, m.vbo_t);
            glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(glm::vec2), texcoords.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
            glEnableVertexAttribArray(2);
        }
        if (!mesh.uvs.empty()) {

            glGenTextures(1, &m.texture);
            glBindTexture(GL_TEXTURE_2D, m.texture);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Image3 img = imread3("../scenes/hw3/PanasonicGH4.jpg");
            std::cout << "Loaded texture: " << img.width << "x" << img.height << std::endl;
            int width = img.width;
            int height = img.height;

            // convert float to unsigned char 
            std::vector<unsigned char> data(3 * width * height);
            for (int i = 0; i < width * height; i++) {
                data[3*i + 0] = (unsigned char) std::clamp(int(img.data[i].x * 255.f), 0, 255);
                data[3*i + 1] = (unsigned char) std::clamp(int(img.data[i].y * 255.f), 0, 255);
                data[3*i + 2] = (unsigned char) std::clamp(int(img.data[i].z * 255.f), 0, 255);
            }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        std::vector<unsigned int> indices;
        for (auto &f : mesh.faces) {
            indices.push_back(f[0]);
            indices.push_back(f[1]);
            indices.push_back(f[2]);
        }
        glGenBuffers(1, &m.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        m.nFaces = (int)indices.size();

        // convert to glm::mat4
        m.model = to_glm_mat4(mesh.model_matrix);
        meshGLs.push_back(m);
    }

    CameraControl camState;
    camState.base_cam_to_world = to_glm_mat4(scene.camera.cam_to_world);
    glfwSetWindowUserPointer(window, &camState);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        processInput(window, &camState);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(scene.background[0], scene.background[1], scene.background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;

        float s = (float)scene.camera.s;
        float znear = (float)scene.camera.z_near;
        float zfar = (float)scene.camera.z_far;

        // Manual projection matrix 
        glm::mat4 projection(0.0f);
        projection[0][0] = 1.0f / (aspect * s);
        projection[1][1] = 1.0f / s;
        projection[2][2] = -zfar / (zfar - znear);
        projection[2][3] = -1.0f;
        projection[3][2] = -(zfar * znear) / (zfar - znear);
        projection[3][3] = 0.0f;

        glm::mat4 cam_to_world = camState.base_cam_to_world * rotation_from_yaw_pitch(camState.yaw, camState.pitch);
        glm::mat4 view = glm::inverse(cam_to_world);
        shader.use();
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));
        shader.setInt("tex", 0);

        for (auto &m : meshGLs) {
            shader.setMat4("model", glm::value_ptr(m.model));
            if (m.texture != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m.texture);
                shader.setInt("useTexture", 1);
            } else {
                shader.setInt("useTexture", 0);
            }
            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, m.nFaces, GL_UNSIGNED_INT, 0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (auto &m : meshGLs) {
        glDeleteVertexArrays(1, &m.vao);
        glDeleteBuffers(1, &m.vbo_v);
        glDeleteBuffers(1, &m.vbo_c);
        glDeleteBuffers(1, &m.ebo);
    }
    glfwTerminate();
}

void hw_3_4(const std::vector<std::string> &params) {
    // HW 3.4: Render a scene with lighting
        if (params.size() == 0) {
        return;
    }
    Scene scene = parse_scene(params[0]);
    std::cout << scene << std::endl;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    int height = scene.camera.resolution[1];
    int width = scene.camera.resolution[0];
    GLFWwindow* window = glfwCreateWindow(width, height, "hw_3_4", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return;
    }   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Shader shader("../src/hw3_4.vs", "../src/hw3_4.fs");

    struct OpenGLMesh {
        GLuint vao, vbo_v, vbo_c, vbo_n, ebo;
        int nFaces;
        glm::mat4 model;
    };
    std::vector<OpenGLMesh> meshGLs;

    for (auto &mesh : scene.meshes) {
        OpenGLMesh m{};
        glGenVertexArrays(1, &m.vao);
        glBindVertexArray(m.vao);

        std::vector<glm::vec3> positions;
        for (auto &v : mesh.vertices)
            positions.emplace_back(v[0], v[1], v[2]);

        glGenBuffers(1, &m.vbo_v);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_v);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // Vertex colors
        std::vector<glm::vec3> colors;
        if (!mesh.vertex_colors.empty()) {
            for (auto &c : mesh.vertex_colors)
                colors.emplace_back(c[0], c[1], c[2]);
        } else {
            for (size_t i = 0; i < mesh.vertices.size(); ++i)
                colors.emplace_back(1.0f, 1.0f, 1.0f);
        }

        glGenBuffers(1, &m.vbo_c);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_c);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);

        // Vertex normals
        std::vector<glm::vec3> normals;
        normals.reserve(mesh.vertices.size());
        for (auto &n : mesh.vertex_normals)   
            normals.emplace_back(n[0], n[1], n[2]);

        glGenBuffers(1, &m.vbo_n);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_n);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(2);

        std::vector<unsigned int> indices;
        for (auto &f : mesh.faces) {
            indices.push_back(f[0]);
            indices.push_back(f[1]);
            indices.push_back(f[2]);
        }
        glGenBuffers(1, &m.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        m.nFaces = (int)indices.size();

        // convert to glm::mat4
        m.model = to_glm_mat4(mesh.model_matrix);
        meshGLs.push_back(m);
    }

    CameraControl camState;
    camState.base_cam_to_world = to_glm_mat4(scene.camera.cam_to_world);
    glfwSetWindowUserPointer(window, &camState);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window)) {
        processInput(window, &camState);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(scene.background[0], scene.background[1], scene.background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;

        // === Projection matrix ===
        float s = (float)scene.camera.s;
        float znear = (float)scene.camera.z_near;
        float zfar = (float)scene.camera.z_far;
        const float vfov = 2.0f * std::atan(scene.camera.s);

        // not sure why matrix in hw3 assignment doesn't work(only se background color)
        // glm::mat4 projection(1.0f);
        // projection[0][0] = 1.0f / (aspect * s);
        // projection[1][1] = 1.0f / s;
        // projection[2][2] = -zfar / (zfar - znear);
        // projection[2][3] = -(zfar * znear) / (zfar - znear);
        // projection[3][2] = -1.0f;
        // projection[3][3] = 0.0f;
        glm::mat4 projection = glm::perspective(vfov, aspect, znear, zfar);

        glm::mat4 cam_to_world = camState.base_cam_to_world * rotation_from_yaw_pitch(camState.yaw, camState.pitch);
        glm::mat4 view = glm::inverse(cam_to_world);
        glm::vec3 cameraPos = glm::vec3(cam_to_world[3]);
        float t = (float)glfwGetTime();
        glm::vec3 cameraFront = -glm::vec3(cam_to_world[2]);
        cameraFront = glm::normalize(cameraFront);

        // rotate light around Y axis
        glm::vec3 lightDir = glm::normalize(glm::vec3(
            cos(t),   
            1.0f,     
            sin(t)   
        ));
        shader.use();
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));
        shader.setVec3("viewPos", glm::value_ptr(cameraPos));
        shader.setVec3("dirLightDir", glm::value_ptr(lightDir));
        //shader.setVec3("dirLightDir", glm::value_ptr(glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f))));
        shader.setVec3("pointLightPos", glm::value_ptr(glm::vec3(2,1,2)));
        shader.setVec3("spotLightPos", glm::value_ptr(cameraPos));
        shader.setVec3("spotLightDir", glm::value_ptr(cameraFront));
        shader.setFloat("spotLightCutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("spotLightOuterCutOff", glm::cos(glm::radians(17.5f)));
        shader.setFloat("shininess", 32.0f);

        for (auto &m : meshGLs) {
            glm::mat4 model = m.model;
            glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
            shader.setMat4("model", glm::value_ptr(model));
            shader.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
            glBindVertexArray(m.vao);
            glDrawElements(GL_TRIANGLES, m.nFaces, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    for (auto &m : meshGLs) {
        glDeleteVertexArrays(1, &m.vao);
        glDeleteBuffers(1, &m.vbo_v);
        glDeleteBuffers(1, &m.vbo_c);
        glDeleteBuffers(1, &m.vbo_n);
        glDeleteBuffers(1, &m.ebo);
    }
    glfwTerminate();
}

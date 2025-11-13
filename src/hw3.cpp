#include "hw3.h"
#include "3rdparty/glad.h" // needs to be included before GLFW!
#include "3rdparty/glfw/include/GLFW/glfw3.h"
#include "hw3_scenes.h"
#include "shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
void processInput(GLFWwindow *window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

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
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
    while(!glfwWindowShouldClose(window))
    {
        processInput(window);

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
        processInput(window);
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
    //  de-allocate all resources once they've outlived their purpose:
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();

}

struct CameraControl {
    glm::vec3 pos{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float speed = 2.0f;
};

void processInput(GLFWwindow* window, CameraControl& cam, float dt) {
    float v = cam.speed * dt;
    glm::vec3 right = glm::normalize(glm::cross(cam.front, cam.up));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cam.pos += v * cam.front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cam.pos -= v * cam.front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cam.pos -= right * v;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cam.pos += right * v;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
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
        glfwTerminate();
        return;
    }   
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);

    Shader shader("../src/hw3_3.vs", "../src/hw3_3.fs");
    shader.use();

    struct MeshGL {
        GLuint vao, vbo_v, vbo_c, ebo;
        int nFaces;
        glm::mat4 model;
    };
    std::vector<MeshGL> meshGLs;

    for (auto &mesh : scene.meshes) {
        MeshGL m{};
        glGenVertexArrays(1, &m.vao);
        glBindVertexArray(m.vao);

        // === Vertex positions ===
        std::vector<glm::vec3> positions;
        positions.reserve(mesh.vertices.size());
        for (auto &v : mesh.vertices)
            positions.emplace_back(v[0], v[1], v[2]);

        glGenBuffers(1, &m.vbo_v);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_v);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(0);

        // === Vertex colors ===
        std::vector<glm::vec3> colors;
        colors.reserve(mesh.vertices.size());
        if (!mesh.vertex_colors.empty()) {
            for (auto &c : mesh.vertex_colors)
                colors.emplace_back(c[0], c[1], c[2]);
        } else {
            // 没有颜色时填默认白色
            for (size_t i = 0; i < mesh.vertices.size(); ++i)
                colors.emplace_back(1.0f, 1.0f, 1.0f);
        }

        glGenBuffers(1, &m.vbo_c);
        glBindBuffer(GL_ARRAY_BUFFER, m.vbo_c);
        glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glEnableVertexAttribArray(1);


        std::vector<unsigned int> indices;
        for (auto &f : mesh.faces) {
            indices.push_back(f[0]);
            indices.push_back(f[1]);
            indices.push_back(f[2]);
        }
        glGenBuffers(1, &m.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                     indices.data(), GL_STATIC_DRAW);
        m.nFaces = (int)indices.size();

        // 转成 glm::mat4
        m.model = to_glm_mat4(mesh.model_matrix);

        meshGLs.push_back(m);
    }

    // ---------- Camera setup ----------
    CameraControl cam;
    float lastFrame = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        float current = glfwGetTime();
        float delta = current - lastFrame;
        lastFrame = current;
        processInput(window, cam, delta);

        glClearColor(scene.background[0], scene.background[1], scene.background[2], 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;

        // === Projection matrix ===
        float s = (float)scene.camera.s;
        float znear = (float)scene.camera.z_near;
        float zfar = (float)scene.camera.z_far;
        glm::mat4 projection(1.0f);
        projection[0][0] = 1.0f / (aspect * s);
        projection[1][1] = 1.0f / s;
        projection[2][2] = -zfar / (zfar - znear);
        projection[2][3] = -(zfar * znear) / (zfar - znear);
        projection[3][2] = -1.0f;
        projection[3][3] = 0.0f;



        // === View matrix ===
        glm::mat4 view = glm::inverse(to_glm_mat4(scene.camera.cam_to_world));
        //glm::mat4 interactive = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
        //glm::mat4 view = interactive * base_view; // keep both base & interactive motion

        shader.use();
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("view", glm::value_ptr(view));

        for (auto &m : meshGLs) {
            shader.setMat4("model", glm::value_ptr(m.model));
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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
} 
void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

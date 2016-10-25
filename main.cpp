#include <iostream>
#include <cstdlib>

#include "gl_includes.h"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <ctime>

using namespace std;
using namespace glm;

struct Vertex {
    vec3 position;
    vec3 color;
};

bool wireframe = false;

vector<vec3> sphere_positions;
int vertex_count = 0;
int index_count = 0;
int theta_n = 64;
int phi_n = 32;
int legendre_index = 0;

const char *legendre_param_names[9] = {
        "Constant",

        "L1 -Y   ",
        "L1 +Z   ",
        "L1 -X   ",

        "L2 +XY  ",
        "L2 -YZ  ",
        "L2 +ZZ  ",
        "L2 -XZ  ",
        "L2 X2Y2 "
};

float legendre_params[9] = {
        0,
        1,
        1,
        1.7f,
        0.8f,
        -1.2f,
        0,
        0,
        0
};

float *variable = &legendre_params[0];
const char *var_name = legendre_param_names[0];

static const char* vertex_shader_text =
    "uniform mat4 MVP;\n"
    "attribute vec3 vPos;\n"
    "attribute vec3 vCol;\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_Position = MVP * vec4(vPos, 1.0);\n"
    "    color = vCol;\n"
    "}\n";
static const char* fragment_shader_text =
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

void regenerateBuffer();

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (key == GLFW_KEY_W) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
        legendre_index = key - GLFW_KEY_1;
        variable = &legendre_params[legendre_index];
        var_name = legendre_param_names[legendre_index];
        cout << var_name << " = " << *variable << endl;
    }
    else if (key == GLFW_KEY_0) {
        *variable = 0;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    }
    else if (key == GLFW_KEY_UP) {
        *variable += 0.1;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    }
    else if (key == GLFW_KEY_DOWN) {
        *variable -= 0.1;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    }
    else if (key == GLFW_KEY_P) {
        for (int c = 0; c < 9; c++) {
            cout << legendre_param_names[c] << " = " << legendre_params[c] << endl;
        }
    }
}

void glfw_error_callback(int error, const char* description) {
    cerr << "GLFW Error: " << description << " (error " << error << ")" << endl;
}

void regenerateSpherePositions() {
    sphere_positions.resize(uint((phi_n + 1) * (theta_n + 1)));
    int n = 0;
    for (int phi_i = 0; phi_i <= phi_n; phi_i++) {
        float phi = float(M_PI) * phi_i / phi_n;
        float y = -cos(phi);
        float r = sin(phi); // cylindrical radius

        for (int theta_i = 0; theta_i <= theta_n; theta_i++) {
            float theta = float(M_PI) * 2 * theta_i / theta_n;
            float x = r * sin(theta);
            float z = r * cos(theta);
            sphere_positions[n++] = vec3(x, y, z);
        }
    }

    vertex_count = n;
}

void regenerateBuffer() {
//    clock_t time = clock();
    vector<Vertex> vertices(vertex_count);
    for (int n = 0; n < vertex_count; n++) {
        vec3 pos = sphere_positions[n];
        float legendre_0 = 1.f / 2.f * float(M_1_PI);
        float legendre_1_scalar = sqrt(3.f) / 2.f * float(M_1_PI);
        float legendre_1_0 = -legendre_1_scalar * pos.y;
        float legendre_1_1 =  legendre_1_scalar * pos.z;
        float legendre_1_2 = -legendre_1_scalar * pos.x;
        float legendre_2_scalar = sqrt(15.f) / 2.f * float(M_1_PI);
        float legendre_2_zz_scalar = sqrt(5.f) / 4.f * float(M_1_PI);
        float legendre_2_0 =  legendre_2_scalar * pos.y * pos.x;
        float legendre_2_1 = -legendre_2_scalar * pos.y * pos.z;
        float legendre_2_2 =  legendre_2_zz_scalar * (3 * pos.z * pos.z - 1);
        float legendre_2_3 = -legendre_2_scalar * pos.z * pos.x;
        float legendre_2_4 =  legendre_2_scalar / 2.f * (pos.x*pos.x - pos.y*pos.y);
        float total_scalar = legendre_0 * legendre_params[0] +
                             legendre_1_0 * legendre_params[1] +
                             legendre_1_1 * legendre_params[2] +
                             legendre_1_2 * legendre_params[3] +
                             legendre_2_0 * legendre_params[4] +
                             legendre_2_1 * legendre_params[5] +
                             legendre_2_2 * legendre_params[6] +
                             legendre_2_3 * legendre_params[7] +
                             legendre_2_4 * legendre_params[8];
        vertices[n].position = pos * abs(total_scalar);
        vertices[n].color = total_scalar > 0 ? vec3(0,0,1) * total_scalar : vec3(-1,0,0) * total_scalar;
    }

//    clock_t endTime = clock();
//    cout << "Generated buffer in " << endTime - time << " clocks." << endl;
//    clock_t bufferStart = clock();
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Vertex), vertices.data(), GL_DYNAMIC_DRAW);
//    clock_t bufferEnd = clock();
//    cout << "Transferred buffer to graphics card in " << bufferEnd - bufferStart << " clocks." << endl;
}

void regenerateIndices() {
    int n = 0;
    vector<ivec3> triangles(uint(phi_n * theta_n * 2));
    for (int phi_i = 0; phi_i < phi_n; phi_i++) {
        for (int theta_i = 0; theta_i < theta_n; theta_i++) {
            int tl = phi_i * (theta_n + 1) + theta_i;
            int tr = tl + 1;
            int bl = tr + theta_n;
            int br = bl + 1;
            triangles[n++] = ivec3(tr, tl, bl);
            triangles[n++] = ivec3(bl, br, tr);
        }
    }

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, n * sizeof(ivec3), &triangles[0], GL_DYNAMIC_DRAW);

    index_count = n * 3;
}

int main() {
    if (!glfwInit()) {
        cout << "Failed to init GLFW" << endl;
        exit(-1);
    }
    cout << "GLFW Successfully Started" << endl;

    glfwSetErrorCallback(glfw_error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Stupid SH Tricks", NULL, NULL);
    if (!window) {
        cout << "Failed to create window" << endl;
        exit(-1);
    }
    glfwSetKeyCallback(window, glfw_key_callback);

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSwapInterval(1);

    GLuint vertex_buffer, index_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;

    regenerateSpherePositions();

    checkError();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    checkError();

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    regenerateBuffer();
    checkError();

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    regenerateIndices();
    checkError();

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
    checkError();

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
    checkError();

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    glValidateProgram(program);
    checkError();

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");
    checkError();

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    checkError();
    glEnableVertexAttribArray(vpos_location);
    checkError();
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, position));
    checkError();
    glEnableVertexAttribArray(vcol_location);
    checkError();
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void *) offsetof(Vertex, color));
    checkError();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        checkError();
        ratio = width / (float) height;
        glViewport(0, 0, width, height);
        checkError();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        checkError();
        mat4 m = rotate(mat4(), float(glfwGetTime() * 180 / M_PI), vec3(1,0,0));
        mat4 p = ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4 mvp = p * m;

        glUseProgram(program);
        checkError();
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &mvp[0,0]);
        checkError();
        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        checkError();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        checkError();
//        glDrawArrays(GL_POINTS, 0, vertex_count);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
        checkError();
        glfwSwapBuffers(window);
        checkError();
        glfwPollEvents();
        checkError();
    }

    return 0;
}

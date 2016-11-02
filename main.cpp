#include <iostream>
#include <cstdlib>

#include <cmath>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "gl_includes.h"
#include "legendre.h"
#include "Perf.h"

using namespace std;
using namespace glm;

bool wireframe = false;

vector<vec3> sphere_positions;
vector<float> legendre_scalars;
vector<float> shape_scalars;
size_t vertex_count = 0;
size_t index_count = 0;
size_t theta_n = 64;
size_t phi_n = 32;
size_t legendre_index = 0;

int rotationFrames = 0;

struct Rotation {
    float startTime;
    vec3 axis;
};
vector<Rotation> rotations;

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
    "uniform float scale;\n"
    "attribute vec3 vPos;\n"
    "attribute float vScale;\n"
    "varying vec3 color;\n"
    "void main() {\n"
    "    float scalar = scale * vScale;\n"
    "    gl_Position = MVP * vec4(vPos * abs(scalar), 1.0);\n"
    "    color = scalar > 0.0 ? vec3(0.0, 0.0, 1.0) * scalar : vec3(-1.0, 0.0, 0.0) * scalar;\n"
    "}\n";
static const char* fragment_shader_text =
    "varying vec3 color;\n"
    "void main() {\n"
    "    gl_FragColor = vec4(color, 1.0);\n"
    "}\n";

void regenerateIndices();
void regenerateSpherePositions();
void regenerateBuffer();
void regenerateSHBuffer();
void rotateParams();

GLuint vertex_buffer, index_buffer, legendre_buffer, sphere_buffer;

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    } else if (key == GLFW_KEY_W) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    } else if (key >= GLFW_KEY_1 && key <= GLFW_KEY_9) {
        legendre_index = size_t(key - GLFW_KEY_1);
        variable = &legendre_params[legendre_index];
        var_name = legendre_param_names[legendre_index];
        cout << var_name << " = " << *variable << endl;
    } else if (key == GLFW_KEY_0) {
        *variable = 0;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    } else if (key == GLFW_KEY_UP) {
        *variable += 0.1;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    } else if (key == GLFW_KEY_DOWN) {
        *variable -= 0.1;
        cout << var_name << " = " << *variable << endl;
        regenerateBuffer();
    } else if (key == GLFW_KEY_P) {
        for (int c = 0; c < 9; c++) {
            cout << legendre_param_names[c] << " = " << legendre_params[c] << endl;
        }
    } else if (key == GLFW_KEY_KP_ADD) {
        theta_n += 2;
        phi_n += 1;
        regenerateIndices();
        regenerateSpherePositions();
        regenerateSHBuffer();
        regenerateBuffer();
    } else if (key == GLFW_KEY_KP_8) {
        rotations.push_back({float(glfwGetTime()), vec3(1, 0, 0)});
    } else if (key == GLFW_KEY_KP_4) {
        rotations.push_back({float(glfwGetTime()), vec3(0, 1, 0)});
    } else if (key == GLFW_KEY_KP_2) {
        rotations.push_back({float(glfwGetTime()), vec3(-1, 0, 0)});
    } else if (key == GLFW_KEY_KP_6) {
        rotations.push_back({float(glfwGetTime()), vec3(0, -1, 0)});
    }
    else if (key == GLFW_KEY_R) {
        rotationFrames += theta_n;
    }
}

void glfw_error_callback(int error, const char* description) {
    cerr << "GLFW Error: " << description << " (error " << error << ")" << endl;
}

void regenerateSpherePositions() {
    Perf stat("Regenerate Sphere positions");
    sphere_positions.resize(uint((phi_n + 1) * (theta_n + 1)));
    size_t n = 0;
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

    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, n * sizeof(vec3), sphere_positions.data(), GL_STATIC_DRAW);
}

void regenerateSHBuffer() {
    Perf stat("Regenerate SH Buffer");
    legendre_scalars.resize(9 * vertex_count);
    for (int n = 0; n < vertex_count; n++) {
        vec3 pos = sphere_positions[n];
        legendre_scalars[n + 0*vertex_count] = legendre_0_0(pos);
        legendre_scalars[n + 1*vertex_count] = legendre_1_0(pos);
        legendre_scalars[n + 2*vertex_count] = legendre_1_1(pos);
        legendre_scalars[n + 3*vertex_count] = legendre_1_2(pos);
        legendre_scalars[n + 4*vertex_count] = legendre_2_0(pos);
        legendre_scalars[n + 5*vertex_count] = legendre_2_1(pos);
        legendre_scalars[n + 6*vertex_count] = legendre_2_2(pos);
        legendre_scalars[n + 7*vertex_count] = legendre_2_3(pos);
        legendre_scalars[n + 8*vertex_count] = legendre_2_4(pos);
    }

    glBindBuffer(GL_ARRAY_BUFFER, legendre_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float) * 9, legendre_scalars.data(), GL_STATIC_DRAW);
}

void rotateParams() {
    Perf stat("Rotate params");
    float newParams[9] = {0,0,0,0,0,0,0,0,0};

    int baseIndex = 0;
    float d_phi = float(M_PI) / phi_n;
    float d_theta = 2 * float(M_PI) / theta_n;
    for (int param = 0; param < 9; param++) {
        int shapeIndex = 0;
        for (int phi_i = 0; phi_i < phi_n; phi_i++) {
            float sin_phi = sin(float(M_PI) * phi_i / phi_n);
            for (int theta_i = 0; theta_i < theta_n; theta_i++) {
                float integrand = legendre_scalars[baseIndex + shapeIndex + theta_i] * shape_scalars[shapeIndex + theta_i + 1]; // doesn't overflow because there's an extra on the end
                newParams[param] += integrand * d_phi * d_theta * sin_phi;
            }
            shapeIndex += theta_n + 1;
        }
        baseIndex += shapeIndex + theta_n + 1;
    }

    for (int c = 0; c < 9; c++) {
        legendre_params[c] = newParams[c] * M_PI; // For some reason it's off by a factor of M_PI. TODO: figure out why
    }

    regenerateBuffer();
}

void regenerateBuffer() {
    Perf stat("Regenerate legendre buffer");
    shape_scalars.resize(vertex_count);
    for (int n = 0; n < vertex_count; n++) {
        vec3 pos = sphere_positions[n];
        shape_scalars[n] = legendre_total(pos, legendre_params);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), shape_scalars.data(), GL_DYNAMIC_DRAW);
}

void regenerateIndices() {
    Perf stat("Regenerate indices");
    size_t n = 0;
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

    GLuint vertex_shader, fragment_shader, program;
    GLuint mvp_location, vpos_location, vscale_location, scale_location;

    initPerformanceData();

    checkError();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    checkError();

    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    regenerateSpherePositions();
    checkError();

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    regenerateBuffer();
    checkError();

    glGenBuffers(1, &legendre_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, legendre_buffer);
    regenerateSHBuffer();
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
    scale_location = glGetUniformLocation(program, "scale");
    vpos_location = glGetAttribLocation(program, "vPos");
    vscale_location = glGetAttribLocation(program, "vScale");
    checkError();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glEnable(GL_DEPTH_TEST);

    // We'll always use the sphere_buffer for vPos
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    checkError();
    glEnableVertexAttribArray(vpos_location);
    checkError();
    glVertexAttribPointer(vpos_location, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0);
    checkError();

    // We're going to use an attrib array for vScale, but it will be set up later.
    glEnableVertexAttribArray(vscale_location);
    checkError();

    // make sure performance data is clean going into main loop
    markPerformanceFrame();
    printPerformanceData();
    double lastPerfPrintTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float ratio;
        int width, height, viewportSize;
        mat4 m, p, mvp;

        {
            Perf stat("Setup frame and clear buffers");
            glfwGetFramebufferSize(window, &width, &height);
            checkError();
            ratio = width / (float) height;
            viewportSize = height / 5;
            glViewport(0, 0, width, height);
            checkError();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            checkError();

            float now = float(glfwGetTime());
            int lastValid = 0;
            for (int c = rotations.size()-1; c >= 0; c--) {
                Rotation &rot = rotations[c];
                float rotation = float((now - rot.startTime) * 180 / M_PI);
                if (rotation < 360) {
                    m = rotate(m, rotation, rot.axis);
                    lastValid = c;
                }
            }
            if (lastValid > 0) {
                rotations.erase(rotations.begin(), rotations.begin() + lastValid);
            }
        }

        {
            Perf stat("Setup vertex pointers for first draw");
            p = ortho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
            mvp = p * m;

            glUseProgram(program);
            checkError();
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) &mvp[0, 0]);
            glUniform1f(scale_location, 1.f);
            checkError();

            // draw the base buffer
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
            checkError();
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);
            checkError();
        }
        {
            Perf stat("Draw base shape");
            // glDrawArrays(GL_POINTS, 0, vertex_count);
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
        }

        // draw the components
        {
            Perf stat("Setup unforms and vertex arrays for components");
            p = ortho(-0.5f, 0.5f, -0.5f, 0.5f, 1.f, -1.f);
            mvp = p * m;
            glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat *) &mvp[0, 0]);
            checkError();

            glBindBuffer(GL_ARRAY_BUFFER, legendre_buffer);
            checkError();

            glClear(GL_DEPTH_BUFFER_BIT); // always draw components on top of base shape. Depth test still necessary since triangles are unordered
        }

        {
            Perf stat("Draw left side");
            // left side
            glViewport(0, 4 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[0]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (0 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(0, 2 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[1]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (1 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(0, 1 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[2]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (2 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(0, 0 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[3]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (3 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();
        }

        {
            Perf stat("Draw right side");
            // right side
            int right = width - viewportSize;

            glViewport(right, 4 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[4]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (4 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(right, 3 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[5]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (5 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(right, 2 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[6]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (6 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(right, 1 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[7]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (7 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();

            glViewport(right, 0 * viewportSize, viewportSize, viewportSize);
            glUniform1f(scale_location, legendre_params[8]);
            glVertexAttribPointer(vscale_location, 1, GL_FLOAT, GL_FALSE, sizeof(float), (const void *) (8 * sizeof(float) * vertex_count));
            glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
            checkError();
        }

        {
            Perf stat("Swap buffers");
            glfwSwapBuffers(window);
            checkError();
        }
        {
            Perf stat("Poll events");
            glfwPollEvents();
            checkError();
        }

        if (rotationFrames > 0) {
            rotateParams();
            rotationFrames--;
        }

        markPerformanceFrame();

        double now = glfwGetTime();
        if (now - lastPerfPrintTime > 10.0) {
            printPerformanceData();
            lastPerfPrintTime = now;
        }
    }

    return 0;
}

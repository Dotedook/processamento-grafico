#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
using namespace glm;
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
using namespace std;

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

int setupShader();
int setupSprite();
int loadTexture(string filePath);

const GLuint WIDTH = 1920, HEIGHT = 1080;

const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 
 uniform mat4 model;
 uniform mat4 projection;

 void main()
 {
    tex_coord = vec2(texc.s, 1.0 - texc.t);
    gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 void main()
 {
	 color = texture(tex_buff,tex_coord);
 }
 )";

class Sprite
{
public:
    Sprite(string path, int shaderID, float relWidth, float relHeight, float xpos = 0.0f, float ypos = 0.0f)
    {
        _sprite = loadTexture(path);
        _vao = setupSprite();
        _projMat = ortho(0.0f, float(WIDTH), 0.0f, float(HEIGHT), -1.0f, 1.0f);
        _shaderID = shaderID;

        mat4 model = mat4(1.0f);
        model = translate(model, vec3(xpos, ypos, 0.0f));
        model = scale(model, vec3(relWidth, relHeight, 1.0f));
        _modelMat = model;
    }

    void draw()
    {
        glUseProgram(_shaderID);

        glUniformMatrix4fv(glGetUniformLocation(_shaderID, "model"), 1, GL_FALSE, value_ptr(_modelMat));
        glUniformMatrix4fv(glGetUniformLocation(_shaderID, "projection"), 1, GL_FALSE, value_ptr(_projMat));

        glBindVertexArray(_vao);
        glBindTexture(GL_TEXTURE_2D, _sprite);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    void clear()
    {
        glDeleteVertexArrays(1, &_vao);
    }

private:
    GLuint _vao;
    GLuint _sprite;
    mat4 _projMat;
    mat4 _modelMat;
    int _shaderID;
};

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Triangulo! -- Rossana", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Falha ao criar a janela GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Falha ao inicializar GLAD" << std::endl;
        return -1;
    }

    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *version = glGetString(GL_VERSION);
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();

    glUseProgram(shaderID);

    double prev_s = glfwGetTime();
    double title_countdown_s = 0.1;

    float colorValue = 0.0;

    glActiveTexture(GL_TEXTURE0);

    glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    vector<Sprite> sprites = {
        Sprite("../assets/textures/sky.png", shaderID, WIDTH, HEIGHT, WIDTH / 2, HEIGHT / 2),
        Sprite("../assets/textures/floor.png", shaderID, WIDTH, HEIGHT / 2, WIDTH / 2, HEIGHT / 4),
        Sprite("../assets/textures/palace.png", shaderID, WIDTH, HEIGHT / 2 + 30, WIDTH / 2, HEIGHT - HEIGHT / 4),
        Sprite("../assets/textures/gates.png", shaderID, WIDTH, HEIGHT, WIDTH / 2, HEIGHT - HEIGHT / 2),
        Sprite("../assets/textures/woods.png", shaderID, WIDTH, HEIGHT / 2, WIDTH / 2, HEIGHT / 4),
        Sprite("../assets/textures/eye.png", shaderID, 400, 400, 500.0, 380.0),
        Sprite("../assets/textures/skulls.png", shaderID, 100, 100, 1300.0, 200.0),
        Sprite("../assets/textures/dragon.png", shaderID, 350, 350, 1200.0, 400.0),
        Sprite("../assets/textures/flower.png", shaderID, 200, 200, 750.0, 150.0),
    };

    while (!glfwWindowShouldClose(window))
    {
        {
            double curr_s = glfwGetTime();
            double elapsed_s = curr_s - prev_s;
            prev_s = curr_s;

            title_countdown_s -= elapsed_s;
            if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
            {
                double fps = 1.0 / elapsed_s;
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "Ola Triangulo! -- Rossana\tFPS %.2lf", fps);
                glfwSetWindowTitle(window, tmp);

                title_countdown_s = 0.1;
            }
        }

        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        for (auto &sprite : sprites)
        {
            sprite.draw();
        }

        glUseProgram(shaderID);

        glfwSwapBuffers(window);
    }

    for (auto &sprite : sprites)
    {
        sprite.clear();
    }

    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupSprite()
{
    GLfloat vertices[] = {
        // x   y    z    s     t
        -0.5, 0.5, 0.0, 0.0, 1.0,  // V0
        -0.5, -0.5, 0.0, 0.0, 0.0, // V1
        0.5, 0.5, 0.0, 1.0, 1.0,   // V2
        0.5, -0.5, 0.0, 1.0, 0.0   // V3
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    return VAO;
}

int loadTexture(string filePath)
{
    GLuint texID;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;

    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else

        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

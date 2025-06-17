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

const GLuint WIDTH = 600, HEIGHT = 600;

const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texc;

out vec2 tex_coord;

uniform mat4 model;
uniform mat4 projection;
uniform vec2 uvOffset;
uniform vec2 uvScale;

void main()
{
    tex_coord = texc * uvScale + uvOffset;
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
    color = texture(tex_buff, tex_coord);
}
)";

class Sprite
{
private:
    GLuint _vao, _vbo, _ebo, _texID;
    GLuint _shaderID;

    int _textureWidth, _textureHeight;
    int _frameWidth, _frameHeight;

    int _framesX, _framesY;
    int _currentFrame;
    float _timeSinceLastFrame;
    float _frameDuration;

public:
    float _relWidth, _relHeight;
    Sprite(const char *path, GLuint shaderID, int frameWidth, int frameHeight, int framesX, int framesY, float relWidth, float relHeight)
        : _shaderID(shaderID), _frameWidth(frameWidth), _frameHeight(frameHeight), _framesX(framesX), _framesY(framesY), _currentFrame(0),
          _timeSinceLastFrame(0.0f), _frameDuration(0.2f), _relWidth(relWidth), _relHeight(relHeight)
    {
        int nrChannels;
        unsigned char *data = stbi_load(path, &_textureWidth, &_textureHeight, &nrChannels, 4);
        if (!data)
        {
            std::cerr << "Failed to load texture: " << path << std::endl;
            return;
        }

        glGenTextures(1, &_texID);
        glBindTexture(GL_TEXTURE_2D, _texID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _textureWidth, _textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);

        float vertices[] = {
            // positions       // tex coords
            0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f};

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0};

        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &_vbo);
        glGenBuffers(1, &_ebo);

        glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw(int frameX, int frameY, mat4 modelMat, mat4 projMat)
    {
        glUseProgram(_shaderID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _texID);
        glUniform1i(glGetUniformLocation(_shaderID, "tex_buff"), 0);

        float offsetX = float(frameX * _frameWidth) / _textureWidth;
        float offsetY = float(frameY * _frameHeight) / _textureHeight;

        float scaleX = float(_frameWidth) / _textureWidth;
        float scaleY = float(_frameHeight) / _textureHeight;

        offsetY = 1.0f - offsetY - scaleY;

        glUniform2f(glGetUniformLocation(_shaderID, "uvOffset"), offsetX, offsetY);
        glUniform2f(glGetUniformLocation(_shaderID, "uvScale"), scaleX, scaleY);

        glUniformMatrix4fv(glGetUniformLocation(_shaderID, "model"), 1, GL_FALSE, &modelMat[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(_shaderID, "projection"), 1, GL_FALSE, &projMat[0][0]);

        glBindVertexArray(_vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

enum AnimationState
{
    IDLE,
    WALK,
    CLIMB
};

class CharacterController
{
private:
    Sprite _idleSprite;
    Sprite _walkSprite;
    Sprite _climbSprite;
    Sprite *_currentSprite;

    mat4 _modelMat;
    mat4 _projMat;
    bool _facingRight;

    float _x, _y;
    int _frameX, _frameY;

    float _animTimer;
    float _animSpeed;

public:
    CharacterController(
        const char *idlePath,
        const char *walkPath,
        const char *climbPath,
        GLuint shaderID) : _idleSprite(idlePath, shaderID, 32, 32, 4, 1, 32, 32),
                           _walkSprite(walkPath, shaderID, 32, 32, 4, 1, 32, 32),
                           _climbSprite(climbPath, shaderID, 32, 32, 4, 2, 32, 32)
    {
        _x = WIDTH / 2.0f;
        _y = HEIGHT / 2.0f;
        _frameX = 0;
        _frameY = 0;
        _currentSprite = &_idleSprite;
        _facingRight = true;
        _animTimer = 0.0f;
        _animSpeed = 0.15f;

        _projMat = glm::ortho(0.0f, float(WIDTH), float(HEIGHT), 0.0f, -1.0f, 1.0f);
    };

    void handleInput(GLFWwindow *window, float dt)
    {
        const float speed = 150.0f;
        bool isMoving = false;

        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            _x += speed * dt;
            _currentSprite = &_walkSprite;
            _facingRight = true;
            isMoving = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            _x -= speed * dt;
            _currentSprite = &_walkSprite;
            _facingRight = false;
            isMoving = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            _y -= speed * dt;
            _currentSprite = &_climbSprite;
            isMoving = true;
        }
        else
        {
            _currentSprite = &_idleSprite;
        }

        if (isMoving)
        {
            animate(dt);
            if (_currentSprite == &_climbSprite)
                animateClimb(dt);
        }
        else
        {
            animateIdle(dt);
        }
    }

    void animate(float dt)
    {
        _animTimer += dt;
        if (_animTimer >= _animSpeed)
        {
            _animTimer = 0.0f;
            _frameX++;
            if (_frameX >= 4)
                _frameX = 0;
        }
    }

    void update(float dt)
    {
        _modelMat = mat4(1.0f);
        _modelMat = translate(_modelMat, vec3(_x, _y, 0.0f));
        float scaleX = _currentSprite->_relWidth * (_facingRight ? 1.0f : -1.0f);
        float scaleY = _currentSprite->_relHeight;
        _modelMat = scale(_modelMat, vec3(scaleX, scaleY, 1.0f));
    }

    void draw()
    {
        _modelMat = glm::mat4(1.0f);
        _modelMat = glm::translate(_modelMat, glm::vec3(_x, _y, 0.0f));

        float scaleX = _facingRight ? 1.0f : -1.0f;

        _modelMat = glm::scale(_modelMat, glm::vec3(scaleX * _currentSprite->_relWidth, _currentSprite->_relHeight, 1.0f));

        _currentSprite->draw(_frameX, _frameY, _modelMat, _projMat);
    }

    void animateClimb(float dt)
    {
        _animTimer += dt;
        if (_animTimer >= _animSpeed)
        {
            _animTimer = 0.0f;
            _frameX = (_frameX + 1) % 4;
            _frameY = 2;
        }
    }

    void animateIdle(float dt)
    {
        _animTimer += dt;
        if (_animTimer >= _animSpeed)
        {
            _animTimer = 0.0f;
            _frameX = (_frameX + 1) % 4;
            _frameY = 0;
        }
    }
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

    CharacterController player("../assets/sprites/pinkMonsterIdle.png", "../assets/sprites/pinkMonsterWalk.png", "../assets/sprites/pinkMonsterClimb.png", shaderID);

    while (!glfwWindowShouldClose(window))
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

        glfwPollEvents();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glUseProgram(shaderID);

        player.handleInput(window, elapsed_s);
        player.update(elapsed_s);
        player.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
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

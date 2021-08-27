#include <windows.h>
#undef APIENTRY
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"

#define LIMIT_FPS true

class Window
{
public:
	Window(unsigned int a_width = 1920, unsigned int a_height = 1080);
	~Window();
	void run();

private:
    void update();
    void render();
	void processInput(float deltaTime);
	static void frameBufferChangedCallback(GLFWwindow* window, int width, int height);

    std::string title = "OpenGL";
    bool running = 1;
    float xPos, yPos, xSize, ySize;
    float movementCoefficient = 0.001f;
    float zoomCoefficient = 1.01f;

    unsigned int width, height;
    GLFWwindow* window = NULL;
    Shader shader;
    unsigned int vertexArrayObjectID = 0;
    unsigned int vertexBufferObjectID = 0;

    const double minFrameTimeMS = 1000.0 / 60.0;
    const float vertices[32] = {
         1.0f,  1.0f, 0.0f,     1.0f, 1.0f, // top right
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f, // bottom left

         1.0f,  1.0f, 0.0f,     1.0f, 1.0f, // top right
        -1.0f,  1.0f, 0.0f,     0.0f, 1.0f, // top left
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f  // bottom left
    };
};

Window::Window(unsigned int a_width, unsigned int a_height)
	: width(a_width), height(a_height)
{
    // initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // try to create window
    if (!LIMIT_FPS) {
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
    }
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::exception("Failed to create window");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferChangedCallback);

    // try to load GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        throw std::exception("Failed to initialize GLAD");
    }

    // load shader
    shader.load("src/vertex_shader.glsl", "src/fragment_shader.glsl");

    // create and bind vertex array and buffer
    glGenVertexArrays(1, &vertexArrayObjectID);
    glGenBuffers(1, &vertexBufferObjectID);
    glBindVertexArray(vertexArrayObjectID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObjectID);

    // create new data buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // specify position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // specify texture attribute and enable array
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    xPos = -2.86f;
    yPos = -1.13f;
    ySize = 2.3f;
    xSize = ((float)width) / height * ySize;
}

Window::~Window()
{
    // delete buffers and terminate GLFW
	glDeleteVertexArrays(1, &vertexArrayObjectID);
	glDeleteBuffers(1, &vertexBufferObjectID);
	glfwTerminate();
}

void Window::run()
{
    double lastTime = glfwGetTime();
    double timer = lastTime;
    double delta = 0;

    int updates = 0;
    int frames = 0;
    while (running) {
        double now = glfwGetTime();
        delta += ((now - lastTime) * 1000) / minFrameTimeMS;
        lastTime = now;
        while (delta >= 1) {
            update();
            updates++;
            delta--;
        }
        render();
        frames++;
        
        if (glfwGetTime() - timer >= 1.0) {
            timer += 1.0;
            std::string counter = title + std::string(" | ") + std::string("UPS: ") + std::to_string(updates)
                + std::string(", FPS: ") + std::to_string(frames);
            glfwSetWindowTitle(window, counter.c_str());
            updates = 0;
            frames = 0;
        }
    }
}

void Window::update()
{
    static double prevUpdate = 0;
    float timeDelta = (glfwGetTime() - prevUpdate) * 1000.0;
    // on update
    glfwPollEvents();
    processInput(timeDelta);
    running = !glfwWindowShouldClose(window);
    prevUpdate = glfwGetTime();
}

void Window::render()
{
    // on render
    glUniform2f(glGetUniformLocation(shader.programID, "pos"), xPos, yPos);
    glUniform2f(glGetUniformLocation(shader.programID, "size"), xSize, ySize);
    shader.useProgram();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    if (LIMIT_FPS) {
        glfwSwapBuffers(window); // double duffer, limit to 60 FPS
    }
    else {
        glFlush();
    }
}

void Window::processInput(float timeDelta) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        yPos += ySize * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        yPos -= ySize * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        xPos -= xSize * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        xPos += xSize * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        xPos += xSize * (1 - 1 / zoomCoefficient) / 2;
        yPos += ySize * (1 - 1 / zoomCoefficient) / 2;
        xSize /= zoomCoefficient;
        ySize /= zoomCoefficient;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        xPos += xSize * (1 - 1 * zoomCoefficient) / 2;
        yPos += ySize * (1 - 1 * zoomCoefficient) / 2;
        xSize *= zoomCoefficient;
        ySize *= zoomCoefficient;
    }
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS) {
        xPos = -2.86f;
        yPos = -1.13f;
        ySize = 2.3f;
        xSize = ((float)width) / height * ySize;
    }
        
    // std::cout << zoomCoefficient << ' ' << xPos << ' ' << yPos << ' ' << xSize << ' ' << ySize << '\n';
}

void Window::frameBufferChangedCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

int main()
{
    try {
        Window w(1920, 1080);
        w.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << '\n';
    }
}
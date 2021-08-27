#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "shader.h"

// glfw can help us to easily limit FPS to 60
#define LIMIT_FPS true

class Window
{
public:
	Window();
	~Window();
	void run();

private:
    void update();
    void render();
	void processInput(float deltaTime); // use delta time to move correctly
    void setDimensions(const int a_width, const int a_height);
    void changeCursorPos(float xpos, float ypos);

    // static callbacks, they refer to globalWindow pointer
	static void frameBufferChangedCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorCallback(GLFWwindow* window, double xpos, double ypos);
    
    std::string title = "Mandelbrot Set OpenGL";
    GLFWwindow* window = NULL;
    GLFWmonitor* monitor = NULL;
    Shader shader;

    // 0 - Mandelbrot, 1 - Julia sets
    bool mode = 0;
    bool modeSwitch = 0;

    bool fullscreen = 0;
    bool fullscreenSwitch = 0;

    int monitorWidth, monitorHeight;
    unsigned int windowWidth, windowHeight;

    float coursorPosX, coursorPosY;
    float posX, posY;
    float sizeX, sizeY;

    float movementCoefficient = 0.001f;
    float zoomCoefficient = 1.01f;

    unsigned int vertexArrayObjectID = 0;
    unsigned int vertexBufferObjectID = 0;

    // whole screen consists of 2 triangles
    const float vertices[30] = {
         1.0f,  1.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f, // bottom left

         1.0f,  1.0f, 0.0f, // top right
        -1.0f,  1.0f, 0.0f, // top left
        -1.0f, -1.0f, 0.0f  // bottom left
    };
};

// we need this window to manage callbacks
Window* globalWindow;

Window::Window()
{
    // initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // get display resolution
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    monitorWidth = mode->width;
    monitorHeight = mode->height;

    // set window size at quarter display size
    windowWidth = monitorWidth / 2;
    windowHeight = monitorHeight / 2;

    // try to create window
    if (!LIMIT_FPS) {
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
    }
    window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::exception("Failed to create window");
    }
    monitor = glfwGetWindowMonitor(window);
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // set default location
    posX = -2.86f;
    posY = -1.13f;
    sizeY = 2.3f;
    sizeX = ((float)windowWidth) / windowHeight * sizeY;

    // connect callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorCallback);

    // print help
    std::cout <<
        "This program supports 2 sets: Mandelbrot and Julia\n"\
        "First one is default. In Mandelbrot you can only move and zoom.\n"\
        "In Julia set mode each cursor position in window will generate different fractal.\n"\
        "You will have to move it around and find different sets for yourself.\n\n";

    std::cout <<
        "Move:              Arrows or WASD\n"\
        "Zoom in:           Z or Numpad +\n"\
        "Zoom out:          X or Numpad -\n"\
        "Toggle fullscreen: F11\n"
        "Change mode:       M\n"\
        "Close window:      Esc\n\n";
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
    // need this for UPS and FPS
    double lastTime = glfwGetTime();
    double timer = lastTime;
    double delta = 0;

    int updates = 0;
    int frames = 0;
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        delta += (now - lastTime) * 60.0;
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
    float timeDelta = (float)((glfwGetTime() - prevUpdate) * 1000.0);

    // on update
    glfwPollEvents();
    processInput(timeDelta);

    // switch fullscreen to windowed and vice versa
    if (fullscreenSwitch && !fullscreen) {
        fullscreen = 1;
        fullscreenSwitch = 0;
        glfwSetWindowMonitor(window, monitor, 0, 0, monitorWidth, monitorHeight, GLFW_DONT_CARE);
        setDimensions(monitorWidth, monitorHeight);
    }
    if (fullscreenSwitch && fullscreen) {
        fullscreen = 0;
        fullscreenSwitch = 0;
        glfwSetWindowMonitor(window, NULL, 50, 50, monitorWidth / 2, monitorHeight / 2, GLFW_DONT_CARE);
        setDimensions(monitorWidth / 2, monitorHeight / 2);
    }

    // change Mandelbrot to Julia and vice versa
    if (modeSwitch) {
        mode ^= 1;
        modeSwitch = 0;
        if (!mode) {
            title = "Mandelbrot Set OpenGL";
        }
        else {
            title = "Julia Set OpenGL";
        }
    }

    prevUpdate = glfwGetTime();
}

void Window::render()
{
    // on render
    glUniform2f(glGetUniformLocation(shader.programID, "pos"), posX, posY);
    glUniform2f(glGetUniformLocation(shader.programID, "size"), sizeX, sizeY);
    glUniform2f(glGetUniformLocation(shader.programID, "viewportSize"), (float)windowWidth, (float)windowHeight);

    // -100 is incorrect parameter, and shader will render Mandelbrot
    if (mode == 0)
        glUniform2f(glGetUniformLocation(shader.programID, "juliaConstant"), -100.0f, -1000.f);
    else 
        glUniform2f(glGetUniformLocation(shader.programID, "juliaConstant"), coursorPosX, coursorPosY);

    shader.useProgram();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // update screen
    if (LIMIT_FPS) {
        glfwSwapBuffers(window); // double duffer, limit to 60 FPS
    }
    else {
        glFlush();
    }
}

void Window::processInput(float timeDelta) {
    // exit
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
    
    // move
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        posY += sizeY * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        posY -= sizeY * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        posX -= sizeX * timeDelta * movementCoefficient;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        posX += sizeX * timeDelta * movementCoefficient;
    
    // zoom
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        posX += sizeX * (1 - 1 / zoomCoefficient) / 2;
        posY += sizeY * (1 - 1 / zoomCoefficient) / 2;
        sizeX /= zoomCoefficient;
        sizeY /= zoomCoefficient;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        posX += sizeX * (1 - 1 * zoomCoefficient) / 2;
        posY += sizeY * (1 - 1 * zoomCoefficient) / 2;
        sizeX *= zoomCoefficient;
        sizeY *= zoomCoefficient;
    }

    // reset
    if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS) {
        posX = -2.86f;
        posY = -1.13f;
        sizeY = 2.3f;
        sizeX = ((float)windowWidth) / windowHeight * sizeY;
    }
        
    // std::cout << posX << ' ' << posY << ' ' << sizeX << ' ' << sizeY << '\n';
}

void Window::setDimensions(const int a_width, const int a_height)
{
    // this function handles data from callback
    glViewport(0, 0, a_width, a_height);
    globalWindow->windowWidth = a_width;
    globalWindow->windowHeight = a_height;
    sizeX = ((float)windowWidth) / windowHeight * sizeY;
}

void Window::changeCursorPos(float xpos, float ypos)
{
    // this function handles data from callback
    coursorPosX = xpos / windowWidth * 2.0f - 1.0f;
    coursorPosY = -(ypos / windowHeight * 2.0f - 1.0f);
}

void Window::frameBufferChangedCallback(GLFWwindow* window, int width, int height)
{
    globalWindow->setDimensions(width, height);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // toggle fullscreen
    if (key == GLFW_KEY_F11 && action == GLFW_PRESS)
        globalWindow->fullscreenSwitch = 1;
    
    // toggle mode
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        globalWindow->modeSwitch = 1;
}

void Window::cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    globalWindow->changeCursorPos((float)xpos, (float)ypos);
}


int main()
{
    // launch application and catch any exceptions
    try {
        globalWindow = new Window();
        globalWindow->run();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception caught: " << e.what() << '\n';
    }
}
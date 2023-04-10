#define GLFW_INCLUDE_NONE
//#define point(x, y)  ((x) + (y) * nodes)
//#define move(x0, x) {float *temporary = x0; x0 = x; x = temporary}
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h> 
#include <math.h>
#include "linmath.h"
#define N 64
#define SCALE 4
#define ITER 4

//Vertex shader
//const char* vertexShaderSource =
//"#version 330 core\n"
//"uniform mat4 MVP;"
////"attribute vec2 pos;"
//"layout (location = 0) in vec2 pos;\n"
//"layout (location = 1) in vec4 color;\n"
//"out vec4 v_color;\n"
//"void main() {\n"
//"   gl_Position = vec4(pos, 0.0, 1.0);\n"
//"   v_color = color;\n"
//"}\n";



//const char* vertexShaderSource =
//"#version 330 core\n"
//"layout (location = 0) in vec3 pos;\n"
//"void main()\n"
//"{\n"
//"   gl_Position = vec4(pos, 1.0f);\n"
//"}\n";
//
////Fragment Shader
//const char* fragmentShaderSource = 
//"#version 330 core\n"
//"out vec4 FragColor;\n"
//"void main()\n"
//"{\n"
//"    FragColor = vec4(0.0f, 0.5f, 0.5f, 1.0f);\n"
//"}\n\0";


const char* vertexShaderSource =
"#version 330 core\n"
"layout (location = 0) in vec2 position;\n"
"out vec3 partColor;\n"
"uniform vec3 color;\n"
"void main()\n"
"{\n"
"   partColor = color;\n"
"   gl_Position = vec4(position.x/64.0f*2.0-1.0, position.y/64.0f*2.0-1.0, 0.0f, 1.0f);\n"
"}\n";

const char* fragmentShaderSource =
"#version 330 core\n"
"in vec3 partColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   color = vec4(partColor, 1.0f);\n"
"}\n";


typedef struct Particle {
    //All of this will be needed to create our particles value
    int size;

    float* density;  //Represented as rho
    //We use the m, mass Divided by V, volume to achieve density
    float* density0;

    float diffusion;
    float viscosity; //Represented as mu
    //We use mu*Delta Squared * u. mu is our friction value and (Delta Squared * u) is the Laplacian of the velocity.
    //Higher vescosity equates to slower flow

    float timeStep;
    float* VelX; //Current velocity represented as u
    float* VelY; //Current velocity represented as u

    float* VelX0; //Previous velocity represented as u
    float* VelY0; //Previous velocity represented as u

    ////location of particle
    //float* vertX;
    //float* vertY;

    ////color of particle
    //float* r;
    //float* g;
    //float* b;

};

//Struct prototypes
typedef struct Particle Particle;


//Function prototypes
Particle* ParticleCreation(int nodes, int diffusion, int viscosity, float timeS);
void addDensity(Particle* quad, int x, int y, float amount);
void addVelocity(Particle* quad, int x, int y, float xChange, float yChange);
static void lin_solve(int b, float* x, float* x0, float a, float c, int nodes);
static void diffuse(int b, float* x, float* x0, float diff, float dt, int nodes);
static void project(float* velocX, float* velocY, float* p, float* div, int nodes);
static void advect(int b, float* d, float* d0, float* velocX, float* velocY, float dt, int nodes);
void particleStep(Particle* quad);
static void error_callback(int error, const char* description);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
int index(int x, int y);
int constrain(int x, int min, int max);


//Global settings
const unsigned int SCR_WIDTH = N;
const unsigned int SCR_HEIGHT = N;
float mouseX, mouseY, mouseX0 = 0.0f, mouseY0 = 0.0f, preX, preY;
FILE* filename;


Particle* ParticleCreation(int nodes, int diffusion, int viscosity, float timeS) {
    Particle* quad = malloc(sizeof(*quad));

    quad->size = nodes;
    quad->timeStep = timeS;
    quad->diffusion = diffusion;
    quad->viscosity = viscosity;
    //Current density
    quad->density = calloc(nodes * nodes, sizeof(float));
    //previous density
    quad->density0 = calloc(nodes * nodes, sizeof(float));
    //Current velocities X and Y
    quad->VelX = calloc(nodes * nodes, sizeof(float));
    quad->VelY = calloc(nodes * nodes, sizeof(float));
    //previous velocities X and Y
    quad->VelX0 = calloc(nodes * nodes, sizeof(float));
    quad->VelY0 = calloc(nodes * nodes, sizeof(float));
    //Current vertex of pixel
    //quad->vertX = calloc(nodes * nodes, sizeof(float));
    //quad->vertY = calloc(nodes * nodes, sizeof(float));
    ////Current color of the pixle
    //quad->r = calloc(nodes * nodes, sizeof(float));
    //quad->g = calloc(nodes * nodes, sizeof(float));
    //quad->b = calloc(nodes * nodes, sizeof(float));

    return quad;
}


int constrain(int x, int min, int max) {
    if (x <= max && x >= min) {
        return x;
    }
    else if (x >= max) {
        return max;
    }
    else if (x <= min) {
        return min;
    }
}

int index(int x, int y) {
    x = constrain(x, 0, N - 1);
    y = constrain(y, 0, N - 1);
    return ((x)+(y)*N);
}


void addDensity(Particle* quad, int x, int y, float amount) {
    int location = index(x, y);
    //printf("index value, %d\n", index(x, y));
    quad->density[location] += amount;
    //printf("Density Value %f\n", quad->density[location]);
}

/// <summary>
/// This function is going to be the 
/// </summary>
/// <param name="quad"></param>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="xChange"></param>
/// <param name="yChange"></param>
void addVelocity(Particle* quad, int x, int y, float xChange, float yChange) {
    int nodes = quad->size;
    int location = index(x, y);
    //printf("Location of our particle, xChange, yChange %d, %f, %f \n", location, xChange, yChange);
    quad->VelX[location] += xChange;
    quad->VelY[location] += yChange;
    //printf("Velocity X and Velocity Y, %f, %f \n", quad->VelX[location], quad->VelY[location]);
    //printf("VelX and VelY change: %f, %f: \n", quad->VelX[location], quad->VelY[location]);
}

/// <summary>
/// This function is setting the boundaries of the "Particles". Each particle is made up of an array of tiny "boxes" or points that contain
/// vector information. The boundary's outter box is going to constantly match the outer array of boxes as to for the arrays to contain themselves.
/// </summary>
/// <param name="b"></param>
/// <param name="x"></param>
/// <param name="nodes"></param>
static void setBoundaries(int b, float* x, int nodes) {

    //handles boundaries for x axis

    for (int i = 1; i < nodes - 1; i++) {
        //we use i, 0 (0 being the top row of the array). 
        //Basically we are saying if b == 2 or y then we want to create the exact opposite velocity that of course being the negative value.
        x[index(i, 0)] = b == 2 ? -x[index(i, 1)] : x[index(i, 1)]; //Looking at the y top row
        //The nodes - 1 is looking at the bottom row of the array
        x[index(i, nodes - 1)] = b == 2 ? -x[index(i, nodes - 2)] : x[index(i, nodes - 2)]; //Looking at the y bottom row
    }


    for (int j = 1; j < nodes - 1; j++) {
        x[index(0, j)] = b == 1 ? -x[index(1, j)] : x[index(1, j)];
        x[index(nodes - 1, j)] = b == 1 ? -x[index(nodes - 2, j)] : x[index(nodes - 2, j)];
    }

    //Sets bounds for and checks each outer column and row. This includes the corners
    x[index(0, 0)] = 0.5f * (x[index(1, 0)] + x[index(0, 1)]);
    //printf("boundaries for [0,0] %f\n", x[point(0, 0)]);
    //printf("boundaries for [320, 320], %f\n", x[point(320, 320)]);
    x[index(0, nodes - 1)] = 0.5f * (x[index(1, nodes - 1)] + x[index(0, nodes - 2)]);
    //printf("boundaries for [0, 639] %f\n", x[point(0, nodes - 1)]);
    x[index(nodes - 1, 0)] = 0.5f * (x[index(nodes - 2, 0)] + x[index(nodes - 1, 1)]);
    //printf("boundaries for [640,0] %f\n", x[point(nodes - 1, 0)]);
    x[index(nodes - 1, nodes - 1)] = 0.5f * (x[index(nodes - 2, nodes - 1)] + x[index(nodes - 1, nodes - 2)]);
    //printf("boundaries for [640,640] %f\n", x[point(nodes - 1 , nodes - 1)]);

}

/// <summary>
/// This method is going to handle solving for each value by taking a combination of each value next to it and 
/// solving for the median
/// </summary>
/// <param name="b"></param>
/// <param name="x"></param>
/// <param name="x0"></param>
/// <param name="a"></param>
/// <param name="c"></param>
/// <param name="iter"></param>
/// <param name="nodes"></param>
static void lin_solve(int b, float* x, float* x0, float a, float c, int nodes)
{
    float cRecip = 1.0 / c;
    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            x[index(i, j)] =
                (x0[index(i, j)]
                    + a * (x[index(i + 1, j)]
                        + x[index(i - 1, j)]
                        + x[index(i, j + 1)]
                        + x[index(i, j - 1)]
                        )) * cRecip;
            
            //printf("x point %f\n", x[point(i, j)]);
        }
    }
    setBoundaries(b, x, nodes);
}

static void diffuse(int b, float* x, float* x0, float diff, float dt, int nodes)
{
    float a = dt * diff * (nodes - 2) * (nodes - 2);
    lin_solve(b, x, x0, a, 1 + 6 * a, ITER, nodes);
}

/// <summary>
/// 
/// </summary>
/// <param name="velocX"></param>
/// <param name="velocY"></param>
/// <param name="p"></param>
/// <param name="div"></param>
/// <param name="iter"></param>
/// <param name="nodes"></param>
static void project(float* velocX, float* velocY, float* p, float* div, int nodes)
{

    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            div[index(i, j)] = -0.5f * (
                velocX[index(i + 1, j)]
                - velocX[index(i - 1, j)]
                + velocY[index(i, j + 1)]
                - velocY[index(i, j - 1)]
                ) / nodes;
            p[index(i, j)] = 0;
        }
    }

    setBoundaries(0, div, nodes);
    setBoundaries(0, p, nodes);
    lin_solve(0, p, div, 1, 6, ITER, nodes);

    for (int j = 1; j < nodes - 1; j++) {
        for (int i = 1; i < nodes - 1; i++) {
            velocX[index(i, j)] -= 0.5f * (p[index(i + 1, j)]
                - p[index(i - 1, j)]) * nodes;
            velocY[index(i, j)] -= 0.5f * (p[index(i, j + 1)]
                - p[index(i, j - 1)]) * nodes;
        }
    }

    setBoundaries(1, velocX, nodes);
    setBoundaries(2, velocY, nodes);
}




/// <summary>
/// 
/// </summary>
/// <param name="b"></param>
/// <param name="d"></param>
/// <param name="d0"></param>
/// <param name="velocX"></param>
/// <param name="velocY"></param>
/// <param name="dt"></param>
/// <param name="nodes"></param>
static void advect(int b, float* d, float* d0, float* velocX, float* velocY,  float dt, int nodes)
{
    float i0, i1, j0, j1;

    float dtx = dt * (nodes - 2);
    float dty = dt * (nodes - 2);

    float s0, s1, t0, t1;
    float tmp1, tmp2, x, y;

    float Nfloat = nodes;
    float ifloat, jfloat;
    int i, j;


    for (j = 1, jfloat = 1; j < nodes - 1; j++, jfloat++) {
        for (i = 1, ifloat = 1; i < nodes - 1; i++, ifloat++) {
            tmp1 = dtx * velocX[index(i, j)];
            tmp2 = dty * velocY[index(i, j)];
            x = ifloat - tmp1;
            y = jfloat - tmp2;

            if (x < 0.5f) x = 0.5f;
            if (x > Nfloat + 0.5f) x = Nfloat + 0.5f;
            i0 = floorf(x);
            i1 = i0 + 1.0f;
            if (y < 0.5f) y = 0.5f;
            if (y > Nfloat + 0.5f) y = Nfloat + 0.5f;
            j0 = floorf(y);
            j1 = j0 + 1.0f;


            s1 = x - i0;
            s0 = 1.0f - s1;
            t1 = y - j0;
            t0 = 1.0f - t1;


            int i0i = i0;
            int i1i = i1;
            int j0i = j0;
            int j1i = j1;

            d[index(i, j)] =
                s0 * (t0 * d0[index(i0i, j0i)])
                + (t1 * d0[index(i0i, j1i)])
                + s1 * (t0 * d0[index(i1i, j0i)])
                + (t1 * d0[index(i1i, j1i)]);
            //printf("point in advect %f\n", d[point(i, j)]);

        }
    }
    setBoundaries(b, d, nodes);
}

void particleStep(Particle* quad)
{
    int nodes = quad->size;
    float visc = quad->viscosity;
    float diff = quad->diffusion;
    float dt = quad->timeStep;
    float* Vx = quad->VelX;
    float* Vy = quad->VelY;
    float* Vx0 = quad->VelX0;
    float* Vy0 = quad->VelY0;
    float* s = quad->density0;
    float* density = quad->density;

    //printf("Desnisty %f\n", *density);
    //printf("Time step %f\n", dt);

    diffuse(1, Vx0, Vx, visc, dt, nodes);
    diffuse(2, Vy0, Vy, visc, dt, nodes);

    project(Vx0, Vy0, Vx, Vy, nodes);

    advect(1, Vx, Vx0, Vx0, Vy0, dt, nodes);
    advect(2, Vy, Vy0, Vx0, Vy0, dt, nodes);


    project(Vx, Vy, Vx0, Vy0, nodes);

    diffuse(0, s, density, diff, dt, nodes);
    advect(0, density, s, Vx, Vy, dt, nodes);
}

static void error_callback(int error, const char* description)
{
    printf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

}

//static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
//{
//    //float xpos, ypos;
//    glfwGetCursorPos(window, &xpos, &ypos);
//    mouseX = (float) xpos;
//    mouseY = (float) ypos;
//    //printf("x and y position of mouse: %f, %f, %f, %f\n", xpos, ypos, mouseX, mouseY);
//}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &xpos, &ypos);
        mouseX = (float)xpos / N;
        mouseY = (float)ypos / N;
        //printf("Mousex and y, %f, %f\n", mouseX, mouseY);
    }
}



//void draw(Particle* p, float mX, float mY)
//{
//    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
//    // GLint mvp_location, vpos_location, vcol_location;
//
//    //For rendering densities
//    //for (int i = 0; i < p->size; i++)
//    //{
//    //    for (int j = 0; j < p->size; i++)
//    //    {
//    //        int nodes = p->size;
//    //        int x = SCR_WIDTH;
//    //        int y = SCR_WIDTH;
//    //        int d = p->density[index(x, y)];
//    //    }
//    //}
//
//    float vertices[] = {
//        mX, mY, 0.0f
//    };
//    //================================================openGL pipeline============================================
//    //STEP 1 
//    //Vertex Input
//
//    glGenBuffers(1, &vertex_buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
//    
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
//    glPointSize(10);
//    glDrawArrays(GL_POINTS, 0, 512 * 512);
//    glDisableVertexAttribArray(0);
//}

void drawDensity(Particle* p, GLfloat* alph[], GLuint* shaderProgram) {
    int i, j, nodes = p->size;
    int counter = 0;
    //GLint frag = glGetUniformLocation(shaderProgram, "color");
    /*int vertColor = glGetUniformLocation(*shaderProgram, "color");*/
    for (i = 0; i < p->size; i++) {
        for (j = 0; j < p->size; j++) {
            float x = i;
            float y = j;
            float d = p->density[index(i, j)];
            /*glUniform3f(vertColor, 0.5f, 0.5f, 0.5f);*/
            //int x = i / p->size;
            //int y = j / p->size;
            //float bottomLeft = p->density[index(x, y)];
            //float bottomRight = p->density[index(x, y)];
            //float topLeft = p->density[index(x, y)];
            //float topRight = p->density[index(x, y)];
        }
    }
}


void createShader(GLuint* shaderProgram) {

    //Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }

    //Create shader program
    *shaderProgram = glCreateProgram();

    // Attach shaders to program and link
    glAttachShader(*shaderProgram, vertexShader);
    glAttachShader(*shaderProgram, fragmentShader);
    glLinkProgram(*shaderProgram);
    glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
        printf("ERROR::LINKING::FAILED\n%s\n", infoLog);

    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

}


//void draw_density(Particle* p) {
//    int i, j, nodes = p->size;
//    glBegin(GL_QUADS);
//    for (i = 0; i <= nodes; i++) {
//        for (j = 0; j <= nodes; j++) {
//            int x = i / nodes;
//            int y = j / nodes;
//            float d00 = p->density[index(x, y)];
//            float d10 = p->density[point(i + 1, j)];
//            float d01 = p->density[point(i, j + 1)];
//            float d11 = p->density[point(i + 1, j + 1)];
//            glColor3f(d00, d00, d00); glVertex2f(x, y);
//            glColor3f(d10, d10, d10); glVertex2f(x + 1.0f / nodes, y);
//            glColor3f(d11, d11, d11); glVertex2f(x + 1.0f / nodes, y + 1.0f / nodes);
//            glColor3f(d01, d01, d01); glVertex2f(x, y + 1.0f / nodes);
//        }
//    }
//    glEnd();
//}
//
//void draw_velocity(Particle* p) {
//    int i, j, nodes = p->size;
//    int x, y;
//    float u, v, len;
//    glBegin(GL_LINES);
//    for (i = 1; i <= nodes; i++) {
//        for (j = 1; j <= nodes; j++) {
//            x = i / nodes;
//            y = j / nodes;
//            u = p->VelX0[point(i, j)];
//            v = p->VelY0[point(i, j)];
//            len = sqrt(u * u + v * v) / 10.0f;
//            glColor3f(1, 1, 1); glVertex2f(x, y);
//            glColor3f(1, 1, 1); glVertex2f(x + u / len, y + v / len);
//        }
//    }
//    glEnd();
//}


int main()
{
    printf("We are beginning the setup for drawing\n");
    //Setting up file for disection of information 
    errno_t file_error;
    file_error = fopen_s(&filename, "test.txt", "w+");
    //if (file_error = !0) {
    //    printf("Error opening file");
    //    return -1;
    //}

    //Setting up particles
    Particle* p = ParticleCreation(N, 0, 0, 0.1f);
    
    // glfw: initialize and configure
    // ------------------------------
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        exit(1);
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "SmokeSim", NULL, NULL);
    if (!window)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    //glEnable(GL_BLEND);

    //Checking for keystrokes in the window
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    //Setting window context for shaders
    glfwMakeContextCurrent(window);

    if (!gladLoadGL(glfwGetProcAddress)) {
        printf("Failed to initialize Glad\n");
        return -1;
    }

    //Setting up view port
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);


    GLfloat vertices[N * N * 2 * 4];
    printf("Total verts: %d\n", (N * N * 2 * 4));
    GLuint indices[N * N * 4];
    printf("Total inds: %d\n", (N * N * 4));
    GLuint alpha[N * N * 3];
    printf("Total colors: %d\n", (N * N * 3));
    int counter = 0;
    int counterA = 0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            alpha[counterA++] = 1.0f;
            alpha[counterA++] = 1.0f;
            alpha[counterA++] = 1.0f;
            vertices[counter++] = i;
            vertices[counter++] = j;
            vertices[counter++] = i + 1;
            vertices[counter++] = j;
            vertices[counter++] = i + 1;
            vertices[counter++] = j + 1;
            vertices[counter++] = i;
            vertices[counter++] = j + 1;
            int index = i * N + j;
            indices[index * 4] = index * 4;
            indices[index * 4 + 1] = index * 4 + 1;
            indices[index * 4 + 2] = index * 4 + 2;
            indices[index * 4 + 3] = index * 4 + 3;
        }
    }

    //float vertices[] = {
    //-0.5f, -0.5f, 0.0f
    //};

    //Setting up the VAO and VBO
    //Setting up QUADS
    GLuint shaderProg;
    createShader(&shaderProg);
    printf("shader %d\n", shaderProg);
    GLuint particle_vbo, color_vbo, particle_vao, particle_ebo;
    
    //Setup of VAO
    glGenVertexArrays(1, &particle_vao);
    glBindVertexArray(particle_vao);

    //Setup VBO
    glGenBuffers(1, &particle_vbo);
    glGenBuffers(1, &color_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, particle_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //Testing Colors
    //glBindBuffer(GL_ARRAY_BUFFER, color_vbo);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(alpha), alpha, GL_STATIC_DRAW);
    //Testing colors
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    //Setup the EBO - indicies
    glGenBuffers(1, &particle_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particle_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    //Limits the number of swapped buffers that can happen. This will reduce screen tearing
    glfwSwapInterval(1);
    
    //setting up time
    float timeStep = p->timeStep;
    float lastTime = glfwGetTime();
   
    //int counter = 0;
    while (!glfwWindowShouldClose(window))
    {
        int nodes = p->size;
        //float* testArray;
        int x = mouseX, y = mouseY, z = 0.0f;

        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        timeStep = deltaTime;
        //float ratio;
        //float tStep = (((float)rand() / (float)(RAND_MAX)) * 0.1f);
        //printf("tStep Value: %f", tStep);
        //float timeS = glfwGetTime();
        //float deltaTime = timeS - time;


        //Point
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        //glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);

        //glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProg);
        
        //int colorAtrrib = glGetAttribLocation(shaderProg, "color");
        //glEnableVertexAttribArray(colorAtrrib)
        ////g(colorAtrrib);
        //glVertexAttribPointer(colorAtrrib, 3, GL_FLOAT, GL_FALSE, 0, 0);

        //passing color location into vert and frag shader
        int vertColor = glGetUniformLocation(shaderProg, "color");
        glUniform3f(vertColor, 1.0f, 1.0f, 1.0f);

        glBindVertexArray(particle_vao);
        glDrawElements(GL_QUADS, p->size * p->size * 4, GL_UNSIGNED_INT, 0);


        //This will limit adding density to only when the mosue makes a change in location
        if (mouseX != mouseX0 || mouseY != mouseY0) {
            addDensity(p, mouseX, mouseY, 0.5f);
            glfwSetMouseButtonCallback(window, mouse_button_callback);
            //glfwSetCursorPosCallback(window, cursor_position_callback);
            preX = mouseX - mouseX0;
            preY = mouseY - mouseY0;
            //printf("density and density0 %f, %f\n", *p->density, *p->density0);
            //printf("mouseX, mouseY, mouseX0, mouseY0, preX and preY %f, %f, %f, %f, %f, %f \n", mouseX, mouseY, mouseX0, mouseY0, preX, preY);
            addVelocity(p, mouseX, mouseY, preX, preY);
            //printf("velocityX and velocityX0, velocityY & velocityY0 %f, %f, %f, %f\n", *p->VelX, *p->VelX0, *p->VelY, *p->VelY0);
            mouseX0 = mouseX;
            mouseY0 = mouseY;
        }


        printf("pointxy %d\n", index(x, y));
        printf("density %f\n", p->density[index(x, y)]);
        printf("velocityX and velocityX0, velocityY & velocityY0 %f, %f, %f, %f\n", p->VelX[index(x, y)], p->VelX0[index(x, y)], p->VelY[index(x, y)], p->VelY0[index(x, y)]);
        particleStep(p);
        drawDensity(p, alpha, shaderProg);
        printf("pointxy %d\n", index(x, y));
        printf("density %f\n", p->density[index(x, y)]);
        printf("velocityX and velocityX0, velocityY & velocityY0 %f, %f, %f, %f\n", p->VelX[index(x, y)], p->VelX0[index(x, y)], p->VelY[index(x, y)], p->VelY0[index(x, y)]);

        //printf("density and density0 %f, %f\n", *p->density, *p->density0);
        //printf("velocityX and velocityX0, velocityY & velocityY0 %f, %f, %f, %f\n", *p->VelX, *p->VelX0, *p->VelY, *p->VelY0);
        //draw(p, mouseX, mouseY);

        //SWAPPING BUFFERs
        //We are switch the back and front buffers to render the entire frame and swawpt them
        glfwSwapBuffers(window);
        //This will process events by polling and waiting for event sot happen.
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &particle_vao);
    glDeleteBuffers(1, &particle_vbo);
    glDeleteBuffers(1, &color_vbo);
    glDeleteProgram(shaderProg);
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(0);
}
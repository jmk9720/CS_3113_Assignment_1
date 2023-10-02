/**
 * Author: Jonathan Kim
 * Assignment: Simple 2D Scene
 * Date Due: 2023-09-30, 11:59pm
 * I pledge that I have completed this assignment without
 * collaborating with anyone else, in conformance with the
 * NYU School of Engineering Policies and Procedures on
 * Academic Misconduct.
 **/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#define LOG(argument) std::cout << argument << '\n'

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECONDS = 1000.0f;
const float DEGREES_PER_SECOND = 90.0f;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

const char PLAYER_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 1/CS_3113_Assignment_1/CS 3113 Project 1/CS 3113 Project 1/square.png";
const char OTHER_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 1/CS_3113_Assignment_1/CS 3113 Project 1/CS 3113 Project 1/circle.png";

SDL_Window* g_display_window;

bool g_game_is_running = true;
bool g_is_growing = true;
bool g_positive = true;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
          g_model_matrix,
          g_other_matrix,
          g_projection_matrix,
          g_model_trans_matrix,
          g_other_trans_matrix;

float g_previous_ticks = 0.0f;

GLuint model_texture_id;
GLuint other_texture_id;

float g_x = 0.0f;
float g_rotate = 0.0f;

const float GROWTH_FACTOR = 1.01f;
const float SHRINK_FACTOR = 0.99f;

GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        LOG("Unable to load image, Make sure the path is correct");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Simple 2D Scene",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    //g_model_matrix = glm::mat4(1.0f);
    //g_other_matrix = glm::mat4(1.0f);
    
    g_model_trans_matrix = g_model_matrix;
    g_other_trans_matrix = g_other_matrix;
    g_shader_program.SetProjectionMatrix(g_projection_matrix);
    g_shader_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_shader_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    model_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(OTHER_SPRITE_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            g_game_is_running = false;
        }
    }
}

void update() {
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECONDS;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    std::cout << g_x << std::endl;
    if (g_x > 3.0f) {
        g_positive = !g_positive;
    }
    else if (g_x < -3.0f) {
        g_positive = !g_positive;
    }
    if (g_positive) {
        g_x += 1.0f * delta_time;
    }
    else {
        g_x -= 1.0f * delta_time;
    }
    
    g_rotate += DEGREES_PER_SECOND * delta_time;
    g_model_matrix = glm::mat4(1.0f);
    g_other_matrix = glm::mat4(1.0f);
    
    glm::vec3 scale_vector;
    
    scale_vector = glm::vec3(GROWTH_FACTOR,
                             GROWTH_FACTOR,
                             1.0f);
    
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(g_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    g_model_matrix = glm::translate(g_model_matrix, glm::vec3(g_x, g_x, 0.0f));
    g_other_matrix = glm::translate(g_model_matrix, glm::vec3(g_x, -g_x, 0.0f));
    g_model_matrix = glm::rotate(g_model_matrix, glm::radians(g_rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    //g_other_matrix = glm::scale(g_other_matrix, scale_vector);
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id) {
    g_shader_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
    
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };
    
    glVertexAttribPointer(g_shader_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.positionAttribute);
    
    glVertexAttribPointer(g_shader_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.texCoordAttribute);
    
    draw_object(g_model_matrix, model_texture_id);
    draw_object(g_other_matrix, other_texture_id);
    
    glDisableVertexAttribArray(g_shader_program.positionAttribute);
    glDisableVertexAttribArray(g_shader_program.texCoordAttribute);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();
}

int main(int argc, const char * argv[]) {
    initialize();
    
    while (g_game_is_running) {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}

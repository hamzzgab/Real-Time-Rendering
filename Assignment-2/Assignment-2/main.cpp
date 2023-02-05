// Std. Includes
#include <string>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "Text.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "SOIL2/SOIL2.h"

// Text Rendering
#include <ft2build.h>
#include FT_FREETYPE_H

#include "Texture.h"


#define IMGUI_GLSL_VERSION      "#version 330 core"


// Properties
const GLuint WIDTH = 800, HEIGHT = 600;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement( );

void InitializeImGui();
void RenderImGui();

// Camera
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

glm::vec3 RefractiveIndexColor = glm::vec3(0.0f, 0.0f, 0.0f);
GLfloat RefractiveIndex = 1.52f;
GLfloat bias = 0.0f;
GLfloat scale = 0.0f;
GLfloat power = 0.0f;

bool refractIt = true;

static GLFWwindow *window = nullptr;

int main( )
{
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_FALSE );
    
    window = glfwCreateWindow( WIDTH, HEIGHT, "Reflect On Yo Self", nullptr, nullptr );
    
    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        
        return EXIT_FAILURE;
    }
    
    glfwMakeContextCurrent( window );
    
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    
    // Set the required callback functions
    glfwSetKeyCallback( window, KeyCallback );
//    glfwSetCursorPosCallback( window, MouseCallback );
//    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    
    // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
    glewExperimental = GL_TRUE;
    // Initialize GLEW to setup the OpenGL Function pointers
    if ( GLEW_OK != glewInit( ) )
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    
    // Define the viewport dimensions
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    
    // OpenGL options
    glEnable( GL_DEPTH_TEST );
    
    // Setup and compile our shaders
    Shader Reflection( "res/shaders/reflection.vs", "res/shaders/reflection.frag" );
    Shader Refraction( "res/shaders/refraction.vs", "res/shaders/refraction.frag" );
    Shader skyboxShader( "res/shaders/skybox.vs", "res/shaders/skybox.frag" );

    GLfloat skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
    
    // Setup skybox VAO
    GLuint skyboxVAO, skyboxVBO;
    glGenVertexArrays( 1, &skyboxVAO );
    glGenBuffers( 1, &skyboxVBO );
    glBindVertexArray( skyboxVAO );
    glBindBuffer( GL_ARRAY_BUFFER, skyboxVBO );
    glBufferData( GL_ARRAY_BUFFER, sizeof( skyboxVertices ), &skyboxVertices, GL_STATIC_DRAW );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), ( GLvoid * ) 0 );
    glBindVertexArray(0);
    
    // Cubemap (Skybox)
    vector<const GLchar*> faces;
    faces.push_back( "res/images/skybox/px.jpg" );
    faces.push_back( "res/images/skybox/nx.jpg" );
    faces.push_back( "res/images/skybox/py.jpg" );
    faces.push_back( "res/images/skybox/ny.jpg" );
    faces.push_back( "res/images/skybox/pz.jpg" );
    faces.push_back( "res/images/skybox/nz.jpg" );
    GLuint cubemapTexture = TextureLoading::LoadCubemap( faces );

    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 1000.0f );
    
    Model Monkey( "res/models/Monkey.obj" );
    Model Cube( "res/models/Cube.obj" );
    Model Circle( "res/models/Circle.obj" );
    Model Bunny( "res/models/Bunny.obj" );
    Model f16( "res/models/f16.obj" );
    
    InitializeImGui();
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Game loop
    while( !glfwWindowShouldClose( window ) )
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime( );
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        glfwPollEvents( );
        DoMovement( );
        
        glClearColor( 0.05f, 0.05f, 0.05f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        if (refractIt){
            Refraction.Use( );
            glUniformMatrix4fv( glGetUniformLocation( Refraction.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
            glUniformMatrix4fv( glGetUniformLocation( Refraction.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
            model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.9f));
            model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate( model, glm::vec3( 0.0f, 0.0f, 0.0f ) );
            glUniformMatrix4fv( glGetUniformLocation( Refraction.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
            GLint camLoc = glGetUniformLocation( Refraction.Program, "cameraPos" );
            glUniform3f( glGetUniformLocation( Refraction.Program, "refractiveIndexRGB" ), RefractiveIndexColor.r, RefractiveIndexColor.g, RefractiveIndexColor.b );
            glUniform1f( glGetUniformLocation( Reflection.Program, "refractiveIndex" ), RefractiveIndex );
            glUniform1f( glGetUniformLocation( Refraction.Program, "bias" ), bias );
            glUniform1f( glGetUniformLocation( Refraction.Program, "scale" ), scale );
            glUniform1f( glGetUniformLocation( Refraction.Program, "power" ), power );
            glUniformMatrix3fv( camLoc, 1, GL_FALSE, glm::value_ptr( camera.GetPosition() ) );
            Monkey.Draw( Refraction );
        }else{
            Reflection.Use( );
            glUniformMatrix4fv( glGetUniformLocation( Reflection.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
            glUniformMatrix4fv( glGetUniformLocation( Reflection.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
            model = glm::mat4(1.0f);
            model = glm::scale(model, glm::vec3(0.9f));
            model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::translate( model, glm::vec3( 0.0f, 0.0f, 0.0f ) );
            glUniformMatrix4fv( glGetUniformLocation( Reflection.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
            GLint camLoc = glGetUniformLocation( Reflection.Program, "cameraPos" );
            glUniform3f( glGetUniformLocation( Reflection.Program, "refractiveIndexRGB" ), RefractiveIndexColor.r, RefractiveIndexColor.g, RefractiveIndexColor.b );
            glUniform1f( glGetUniformLocation( Reflection.Program, "refractiveIndex" ), RefractiveIndex );
            glUniform1f( glGetUniformLocation( Reflection.Program, "bias" ), bias );
            glUniform1f( glGetUniformLocation( Reflection.Program, "scale" ), scale );
            glUniform1f( glGetUniformLocation( Reflection.Program, "power" ), power );
            glUniformMatrix3fv( camLoc, 1, GL_FALSE, glm::value_ptr( camera.GetPosition() ) );
            Monkey.Draw( Reflection );
        }

        
        // Draw skybox as last
        glDepthFunc( GL_LEQUAL );
        skyboxShader.Use( );
        view = glm::mat4( glm::mat3( camera.GetViewMatrix( ) ) );
        glUniformMatrix4fv( glGetUniformLocation( skyboxShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        glUniformMatrix4fv( glGetUniformLocation( skyboxShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glBindVertexArray( skyboxVAO );
        glBindTexture( GL_TEXTURE_CUBE_MAP, cubemapTexture );
        glDrawArrays( GL_TRIANGLES, 0, 36 );
        glBindVertexArray( 0 );
        glDepthFunc( GL_LESS );
        
        RenderImGui();
        
        glfwSwapBuffers( window );
    }
    
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    
 
    
    glfwTerminate( );
    return 0;
}


// Moves/alters the camera positions based on user input
void DoMovement( )
{
    // Camera controls
    if ( keys[GLFW_KEY_W] || keys[GLFW_KEY_UP] )
    {
        camera.ProcessKeyboard( FORWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN] )
    {
        camera.ProcessKeyboard( BACKWARD, deltaTime );
    }
    
    if ( keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT] )
    {
        camera.ProcessKeyboard( LEFT, deltaTime );
    }
    
    if ( keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT] )
    {
        camera.ProcessKeyboard( RIGHT, deltaTime );
    }
}

// Is called whenever a key is pressed/released via GLFW
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode )
{
    if ( GLFW_KEY_ESCAPE == key && GLFW_PRESS == action )
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    
    if ( key >= 0 && key < 1024 )
    {
        if ( action == GLFW_PRESS )
        {
            keys[key] = true;
        }
        else if ( action == GLFW_RELEASE )
        {
            keys[key] = false;
        }
    }
}

void MouseCallback( GLFWwindow *window, double xPos, double yPos )
{
    if ( firstMouse )
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }
    
    GLfloat xOffset = xPos - lastX;
    GLfloat yOffset = lastY - yPos;
    
    lastX = xPos;
    lastY = yPos;
    
    camera.ProcessMouseMovement( xOffset, yOffset );
}


void InitializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFontAtlas* atlas = io.Fonts;
    ImFontConfig* config = new ImFontConfig;
    config->GlyphRanges = atlas->GetGlyphRangesDefault();
    config->PixelSnapH = true;
    //atlas->AddFontFromFileTTF("./GameResources/Fonts/CascadiaMono.ttf", 14, config);
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(IMGUI_GLSL_VERSION);
}

void RenderImGui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("ImGui Debugger");
    if (ImGui::CollapsingHeader("Global Light")) {
        if (ImGui::TreeNode("Refractive Index Color")) {
            if (ImGui::BeginTable("RefractionColor", 3))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("R", &RefractiveIndexColor.r, -1.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("G", &RefractiveIndexColor.g, -1.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("B", &RefractiveIndexColor.b, -1.0, 1.0f);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Refractive Index Params")) {
            if (ImGui::BeginTable("RefractionParams", 1))
            {
                ImGui::TableNextColumn();
                ImGui::Checkbox("Refraction", &refractIt);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Refractive Index", &RefractiveIndex, -2.0, 2.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Bias", &bias, -1.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Scale", &scale, -1.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Power", &power, -1.0, 1.0f);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        
    }
 
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

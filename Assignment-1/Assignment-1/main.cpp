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


#define IMGUI_GLSL_VERSION      "#version 330 core"

// Properties
const GLuint WIDTH = 1600, HEIGHT = 1000;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Function prototypes
void KeyCallback( GLFWwindow *window, int key, int scancode, int action, int mode );
void MouseCallback( GLFWwindow *window, double xPos, double yPos );
void DoMovement( );

void InitializeImGui();
void RenderImGui();

typedef struct light_colors {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
}
LightColor;

typedef struct material_colors {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
}
MaterialColor;

typedef struct light_property {
    glm::vec3 light_direction;
}
LightDirection;

static LightColor DefaultLightColor;
static MaterialColor DefaultMaterialColor;
static LightDirection DefaultLightDirection;

static bool BlinnEnabled = false;

static float DarkenCoefficient = 0.8f;
static float MaterialShininess = 32.0f;

// Camera
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void phongLighting(Shader shader){
    GLint lightDirLoc = glGetUniformLocation( shader.Program, "light.direction" );
    GLint viewPosLoc  = glGetUniformLocation( shader.Program, "viewPos" );
    
    glUniform3f( lightDirLoc, DefaultLightDirection.light_direction.x, DefaultLightDirection.light_direction.y, DefaultLightDirection.light_direction.z);
    glUniform3f( viewPosLoc,  camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
    
    // LIGHT PROPERTIES
    glUniform3f( glGetUniformLocation( shader.Program, "light.ambient" ),   DefaultLightColor.ambient_color.r, DefaultLightColor.ambient_color.g, DefaultLightColor.ambient_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "light.diffuse" ),   DefaultLightColor.diffuse_color.r, DefaultLightColor.diffuse_color.g, DefaultLightColor.diffuse_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "light.specular" ),  DefaultLightColor.specular_color.r, DefaultLightColor.specular_color.g, DefaultLightColor.specular_color.b );

    // MATERIAL PROPERTIES
    glUniform1f( glGetUniformLocation( shader.Program, "shininess" ), MaterialShininess );
}

void blinnPhongLighting(Shader shader){
    GLint lightDirLoc = glGetUniformLocation( shader.Program, "light.direction" );
    GLint viewPosLoc  = glGetUniformLocation( shader.Program, "viewPos" );
    
    glUniform3f( lightDirLoc, DefaultLightDirection.light_direction.x, DefaultLightDirection.light_direction.y, DefaultLightDirection.light_direction.z);
    glUniform3f( viewPosLoc,  camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
    
    // LIGHT PROPERTIES
    glUniform3f( glGetUniformLocation( shader.Program, "light.ambient" ),   DefaultLightColor.ambient_color.r, DefaultLightColor.ambient_color.g, DefaultLightColor.ambient_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "light.diffuse" ),   DefaultLightColor.diffuse_color.r, DefaultLightColor.diffuse_color.g, DefaultLightColor.diffuse_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "light.specular" ),  DefaultLightColor.specular_color.r, DefaultLightColor.specular_color.g, DefaultLightColor.specular_color.b );

    glUniform1i( glGetUniformLocation( shader.Program, "blinn" ), BlinnEnabled ? 1 : 0 );
    
    // MATERIAL PROPERTIES
    // MATERIAL PROPERTIES
    glUniform1f( glGetUniformLocation( shader.Program, "shininess" ), MaterialShininess );
}

void toonLighting(Shader shader){
    GLint lightDirLoc = glGetUniformLocation( shader.Program, "light.direction" );
    GLint viewPosLoc  = glGetUniformLocation( shader.Program, "viewPos" );
    
    glUniform3f( lightDirLoc, DefaultLightDirection.light_direction.x, DefaultLightDirection.light_direction.y, DefaultLightDirection.light_direction.z);
    glUniform3f( viewPosLoc,  camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
}


void minnaertLighting(Shader shader){
    GLint lightDirLoc = glGetUniformLocation( shader.Program, "light.direction" );
    GLint viewPosLoc  = glGetUniformLocation( shader.Program, "viewPos" );
    
    glUniform3f( lightDirLoc, DefaultLightDirection.light_direction.x, DefaultLightDirection.light_direction.y, DefaultLightDirection.light_direction.z);
    glUniform3f( viewPosLoc,  camera.GetPosition( ).x, camera.GetPosition( ).y, camera.GetPosition( ).z );
    
    glUniform3f( glGetUniformLocation( shader.Program, "light.color" ),   0.5f, 0.5f, 0.5f );
    
    glUniform1f( glGetUniformLocation( shader.Program, "darkenCoefficient" ), DarkenCoefficient);
    
    glUniform3f( glGetUniformLocation( shader.Program, "material.ambient" ),   DefaultMaterialColor.ambient_color.r, DefaultMaterialColor.ambient_color.g, DefaultMaterialColor.ambient_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "material.diffuse" ),   DefaultMaterialColor.diffuse_color.r, DefaultMaterialColor.diffuse_color.g, DefaultMaterialColor.diffuse_color.b  );
    glUniform3f( glGetUniformLocation( shader.Program, "material.specular" ),  DefaultMaterialColor.specular_color.r, DefaultMaterialColor.specular_color.g, DefaultMaterialColor.specular_color.b );
    
    
//    glUniform3f( glGetUniformLocation( shader.Program, "material.ambient" ), 0.2f, 0.15f, 0.1f );
//    glUniform3f( glGetUniformLocation( shader.Program, "material.diffuse" ), 0.0f, 1.0f, 1.0f );
//    glUniform3f( glGetUniformLocation( shader.Program, "material.specular" ), 0.8, 0.8f, 0.8f );
    glUniform1f( glGetUniformLocation( shader.Program, "material.shininess" ), MaterialShininess );
}


static GLFWwindow *window = nullptr;
int main( )
{
    
    DefaultLightColor.ambient_color     = glm::vec3(0.0f, 0.5f, 0.1f);
    DefaultLightColor.diffuse_color     = glm::vec3(0.5f, 0.5f, 0.5f);
    DefaultLightColor.specular_color    = glm::vec3(1.0f, 1.0f, 1.0f);
    
    DefaultMaterialColor.ambient_color     = glm::vec3(0.0f, 0.5f, 0.1f);
    DefaultMaterialColor.diffuse_color     = glm::vec3(0.0f, 1.0f, 1.0f);
    DefaultMaterialColor.specular_color    = glm::vec3(0.8, 0.8f, 0.8f);
    
    DefaultLightDirection.light_direction = glm::vec3(1.0f, 0.5f, -1.5f);
    
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    
    // Create a GLFWwindow object that we can use for GLFW's functions
    window = glfwCreateWindow( WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr );
    
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
    Shader phongShader( "res/shaders/phongVS.vs", "res/shaders/phongFS.frag" );
    Shader blinnPhongShader( "res/shaders/blinnPhonVS.vs", "res/shaders/blinnPhonFS.frag" );
    Shader toonShader( "res/shaders/toonVS.vs", "res/shaders/toonFS.frag" );
    Shader minnaertShader( "res/shaders/minnaertVS.vs", "res/shaders/minnaertFS.frag" );
    
    // Load models
    Model chaiKaBatila( "res/models/ChaiKaBatila.obj" );
    Model boll( "res/models/boll.obj" );
    
    // Draw in wireframe
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 100.0f );
    
    InitializeImGui();
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // compile and setup the shader
    // ----------------------------
    Shader textShader("res/shaders/text.vs", "res/shaders/text.frag");
    textShader.Use();
    glm::mat4 projection_text = glm::ortho(0.0f, (float)800.0f, 0.0f, (float)600.0f);
    glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_text));
    
    Text text;
    
    // Game loop
    while( !glfwWindowShouldClose( window ) )
    {
        // Set frame time
        GLfloat currentFrame = glfwGetTime( );
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check and call events
        glfwPollEvents( );
        DoMovement( );
        
        // Clear the colorbuffer
        glClearColor(0.75f, 0.75f, 0.75f, 1.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        
        glm::mat4 view = camera.GetViewMatrix( );
        
        GLfloat degRotation = -30.0f;
        GLfloat amtScaling = 0.65f;
        
        glm::mat4 model = glm::mat4(1.0f);
        
        if (!BlinnEnabled){
            text.RenderText(textShader, "Phong Model", 115.0f, 500.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        }else{
            text.RenderText(textShader, "Blinn-Phong Model", 100.0f, 500.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        }
        
        text.RenderText(textShader, "Toon Model", 350.0f, 500.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        text.RenderText(textShader, "Minnaert Model", 550.0f, 500.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
        
        
        
        // TEA POT
        blinnPhongShader.Use( );
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        model = glm::mat4(1.0f);
        model = glm::translate( model, glm::vec3( -1.5f, 0.75f, 0.0f ) ); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(amtScaling));
        model = glm::rotate(model, glm::radians(degRotation), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
        chaiKaBatila.Draw( blinnPhongShader );
        blinnPhongLighting( blinnPhongShader );
        
        // TEA POT
        toonShader.Use( );
        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        model = glm::mat4(1.0f);
        model = glm::translate( model, glm::vec3( 0.0f, 0.75f, 0.0f ) ); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(amtScaling));
        model = glm::rotate(model, glm::radians(degRotation), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
        chaiKaBatila.Draw( toonShader );
        toonLighting( toonShader );
        
        // TEA POT
        minnaertShader.Use( );
        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        model = glm::mat4(1.0f);
        model = glm::translate( model, glm::vec3( 1.5f, 0.75f, 0.0f ) ); // Translate it down a bit so it's at the center of the scene
        model = glm::scale(model, glm::vec3(amtScaling));
        model = glm::rotate(model, glm::radians(degRotation), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
        chaiKaBatila.Draw( minnaertShader );
        minnaertLighting( minnaertShader );
        
        // -------------------------------------------------------------------------------------------------------------------------------------------------
        
        // -------------------------------------------------------------------------------------------------------------------------------------------------
        
//        // BALL
//        blinnPhongShader.Use( );
//        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
//        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
//        model = glm::mat4(1.0f);
//        model = glm::translate( model, glm::vec3( -1.5f, -0.75f, 0.0f ) );
//        model = glm::scale(model, glm::vec3(amtScaling));
//        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
//        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
//        boll.Draw( blinnPhongShader );
//        blinnPhongLighting( blinnPhongShader );
//
//        // BALL
//        toonShader.Use( );
//        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
//        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
//        model = glm::mat4(1.0f);
//        model = glm::translate( model, glm::vec3( 0.0f, -0.75f, 0.0f ) );
//        model = glm::scale(model, glm::vec3(amtScaling));
//        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
//        glUniformMatrix4fv( glGetUniformLocation( toonShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
//        boll.Draw( toonShader );
//        toonLighting( toonShader );
//
//        // BALL
//        minnaertShader.Use( );
//        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
//        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
//        model = glm::mat4(1.0f);
//        model = glm::translate( model, glm::vec3( 1.5f, -0.75f, 0.0f ) );
//        model = glm::scale(model, glm::vec3(amtScaling));
//        model = glm::rotate(model, (GLfloat)glfwGetTime()*0.75f, glm::vec3(0.0f, 1.0f, 0.0f));
//        glUniformMatrix4fv( glGetUniformLocation( minnaertShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
//        boll.Draw( minnaertShader );
//        minnaertLighting( minnaertShader );
        
        RenderImGui();
        // Swap the buffers
        glfwSwapBuffers( window );
    }
    
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
    GLfloat yOffset = lastY - yPos;  // Reversed since y-coordinates go from bottom to left
    
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
    ImGui::Begin("Game Debugging Window");
    if (ImGui::CollapsingHeader("Global Light")) {
        
        if (ImGui::TreeNode("Light Color")) {
            if (ImGui::TreeNode("Ambient Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultLightColor.ambient_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultLightColor.ambient_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultLightColor.ambient_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Diffuse Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultLightColor.diffuse_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultLightColor.diffuse_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultLightColor.diffuse_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Specular Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultLightColor.specular_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultLightColor.specular_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultLightColor.specular_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Material Color")) {
            if (ImGui::TreeNode("Ambient Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultMaterialColor.ambient_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultMaterialColor.ambient_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultMaterialColor.ambient_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Diffuse Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultMaterialColor.diffuse_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultMaterialColor.diffuse_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultMaterialColor.diffuse_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Specular Color")) {
                if (ImGui::BeginTable("ColorAttributes", 3))
                {
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("R", &DefaultMaterialColor.specular_color.r, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("G", &DefaultMaterialColor.specular_color.g, -1.0, 1.0);
                    ImGui::TableNextColumn();
                    ImGui::SliderFloat("B", &DefaultMaterialColor.specular_color.b, -1.0, 1.0);
                    ImGui::EndTable();
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }
        
        
        
        if (ImGui::TreeNode("Light Direction")) {
            if (ImGui::BeginTable("ColorAttributes", 3))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("X", &DefaultLightDirection.light_direction.x, -2.0, 2.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Y", &DefaultLightDirection.light_direction.y, -2.0, 2.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Z", &DefaultLightDirection.light_direction.z, -2.0, 2.0f);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Parameters")) {
            if (ImGui::BeginTable("LightAttributes", 3))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Material Shininess", &MaterialShininess, 0.0, 250.0);
                ImGui::Checkbox("Blinn", &BlinnEnabled);
                ImGui::SliderFloat("Darken Coeff", &DarkenCoefficient, -2.0, 2.0);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }

 
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

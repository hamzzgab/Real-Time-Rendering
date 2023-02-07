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
const GLuint WIDTH = 1600, HEIGHT = 1000;
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

glm::vec3 RefractiveIndexColor = glm::vec3(0.65f, 0.67f, 0.69f);
GLfloat FresnelPower = 0.0f;

bool mouseEnabled = true;

bool CircleDraw = true;
bool CubeDraw = false;
bool MonkeyDraw = false;
bool TeaPotDraw = false;

static GLFWwindow *window = nullptr;


void refractance_with_chromatic_abberation(Shader shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection)
{
    glUniformMatrix4fv( glGetUniformLocation( shader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
    glUniformMatrix4fv( glGetUniformLocation( shader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
    glUniformMatrix4fv( glGetUniformLocation( shader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
    
    glm::vec3 camPos = camera.GetPosition();
    glUniform3f( glGetUniformLocation( shader.Program, "fCameraPosition" ), camPos.x, camPos.y, camPos.z);
    glUniform3f( glGetUniformLocation( shader.Program, "refractiveIndexRGB" ), RefractiveIndexColor.r, RefractiveIndexColor.g, RefractiveIndexColor.b );
    glUniform1f( glGetUniformLocation( shader.Program, "FresnelPower" ), FresnelPower );
}



int main( )
{
    // Init GLFW
    glfwInit( );
    // Set all the required options for GLFW
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
    glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
    
    window = glfwCreateWindow( WIDTH, HEIGHT, "Transmittance Effects", nullptr, nullptr );
    
    if ( nullptr == window )
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate( );
        
        return EXIT_FAILURE;
    }
    
    glfwMakeContextCurrent( window );
    
    glfwGetFramebufferSize( window, &SCREEN_WIDTH, &SCREEN_HEIGHT );
    
    glfwSetKeyCallback( window, KeyCallback );
    glfwSetCursorPosCallback( window, MouseCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    glewExperimental = GL_TRUE;
    if ( GLEW_OK != glewInit( ) )
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }
    
    glViewport( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT );
    
    glEnable( GL_DEPTH_TEST );
    
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
    faces.push_back( "res/images/skybox/Tower/px.png" );
    faces.push_back( "res/images/skybox/Tower/nx.png" );
    faces.push_back( "res/images/skybox/Tower/py.png" );
    faces.push_back( "res/images/skybox/Tower/ny.png" );
    faces.push_back( "res/images/skybox/Tower/pz.png" );
    faces.push_back( "res/images/skybox/Tower/nz.png" );
    GLuint cubemapTexture = TextureLoading::LoadCubemap( faces );

    
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 1000.0f );
    
    Model Monkey( "res/models/Monkey.obj" );
    Model Cube( "res/models/Cube.obj" );
    Model Circle( "res/models/Circle.obj" );
    Model Bunny( "res/models/Bunny.obj" );
    Model f16( "res/models/f16.obj" );
    Model TeaPot( "res/models/TeaPot_Plane.obj" );
    
    InitializeImGui();
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    
    Shader textShader("res/shaders/text.vs", "res/shaders/text.frag");
    textShader.Use();
    glm::mat4 projection_text = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_text));
    
    Text text;
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
        
        if (!mouseEnabled)
        {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
        }
        else
        {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        }
        
        
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        GLfloat amtScaling = 0.7f;
        
        glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 1.0f);
        GLfloat textScaling = 0.3f;
        
        Refraction.Use( );
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(amtScaling));
        model = glm::rotate(model, (GLfloat)glfwGetTime() * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f));
        
        refractance_with_chromatic_abberation(Refraction, model, view, projection);
        
        if (CubeDraw)
        {
            Cube.Draw( Refraction );
        }
        
        if (CircleDraw)
        {
            Circle.Draw( Refraction );
        }
        
        if (MonkeyDraw)
        {
            Monkey.Draw( Refraction );
        }
        
        if (TeaPotDraw)
        {
            TeaPot.Draw( Refraction );
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

        text.RenderText(textShader, "Fresnel: " + to_string(FresnelPower), 10.0f, 30.0f, textScaling, textColor);
        text.RenderText(textShader, "Refrative Index: (" +
                        to_string(RefractiveIndexColor.r) +", "+
                        to_string(RefractiveIndexColor.g) +", "+
                        to_string(RefractiveIndexColor.b) +")" , 10.0f, 10.0f, textScaling, textColor);
    
        
        

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
    
    if ( keys[GLFW_KEY_1] )
    {
        CircleDraw = true;
        CubeDraw = false;
        MonkeyDraw = false;
        TeaPotDraw = false;
    }
    
    if ( keys[GLFW_KEY_2] )
    {
        CircleDraw = false;
        CubeDraw = true;
        MonkeyDraw = false;
        TeaPotDraw = false;
    }
    
    if ( keys[GLFW_KEY_3] )
    {
        CircleDraw = false;
        CubeDraw = false;
        MonkeyDraw = true;
        TeaPotDraw = false;
    }
    
    if ( keys[GLFW_KEY_4] )
    {
        CircleDraw = false;
        CubeDraw = false;
        MonkeyDraw = false;
        TeaPotDraw = true;
    }
    
    if ( keys[GLFW_KEY_M] )
    {
        mouseEnabled = false;
    }
    if ( keys[GLFW_KEY_C] )
    {
        mouseEnabled = true;
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
    if (mouseEnabled)
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
    if (ImGui::CollapsingHeader("Refraction")) {
        if (ImGui::TreeNode("Params")) {
            if (ImGui::BeginTable("RefractionColor", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("R", &RefractiveIndexColor.r, 0.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("G", &RefractiveIndexColor.g, 0.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("B", &RefractiveIndexColor.b, 0.0, 1.0f);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Fresnel Power", &FresnelPower, 0.0f, 5.0f);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }
 
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

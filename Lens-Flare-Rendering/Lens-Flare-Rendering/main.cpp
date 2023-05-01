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

typedef struct light_colors {
    glm::vec3 ambient_color;
    glm::vec3 diffuse_color;
    glm::vec3 specular_color;
}
LightColor;

typedef struct light_property {
    glm::vec3 light_direction;
}
LightDirection;

static LightColor DefaultLightColor;
static LightDirection DefaultLightDirection;

static bool BlinnEnabled = false;
static float MaterialShininess = 12.0f;

// Camera
bool cameraOn = true;
Camera camera( glm::vec3( 0.0f, 0.0f, 3.0f ) );
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

float rectangleVertices[] =
{
    // Coords    // texCoords
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,

     1.0f,  1.0f,  1.0f, 1.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f
};

// DOWNSAMPLING
GLfloat scaleFloat = 0.5f;
GLfloat biasFloat = 0.5f;

glm::vec4 scale = glm::vec4(scaleFloat);
glm::vec4 bias  = glm::vec4(biasFloat);

// GHOSTS
GLint numGhosts = 2;
GLfloat ghostDispersal = 0.033f;
GLfloat ghostWeight = 20.0f;

// HALO
GLfloat haloWidth = 0.091f;
GLfloat haloWeight = 20.0f;

// DISTORTION
GLfloat distortion = 0.001f;

// BLURRING
bool blurIt = false;

// LIGHTING FUNCTION
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
    
    glUniform1f( glGetUniformLocation( shader.Program, "shininess" ), MaterialShininess );
}


static GLFWwindow *window = nullptr;
int main( )
{
    
    DefaultLightColor.ambient_color     = glm::vec3(0.5f, 0.5f, 0.5f);
    DefaultLightColor.diffuse_color     = glm::vec3(0.5f, 0.5f, 0.5f);
    DefaultLightColor.specular_color    = glm::vec3(1.0f, 1.0f, 1.0f);
    
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
    window = glfwCreateWindow( WIDTH, HEIGHT, "Lens Flare Rendering", nullptr, nullptr );
    
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
    glfwSetCursorPosCallback( window, MouseCallback );
    glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
    
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
    
    // Shaders
    Shader blinnPhongShader( "res/shaders/lensflare.vs", "res/shaders/lensflare.frag" );
    Shader framebufferProgram( "res/shaders/frameBuffer.vs", "res/shaders/frameBuffer.frag" );
    Shader skyboxShader( "res/shaders/skybox.vs", "res/shaders/skybox.frag" );

    // Skybox START
    // ------------
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
    faces.push_back( "res/images/skybox/Beach/px.png" );
    faces.push_back( "res/images/skybox/Beach/nx.png" );
    faces.push_back( "res/images/skybox/Beach/py.png" );
    faces.push_back( "res/images/skybox/Beach/ny.png" );
    faces.push_back( "res/images/skybox/Beach/pz.png" );
    faces.push_back( "res/images/skybox/Beach/nz.png" );
    GLuint cubemapTexture = TextureLoading::LoadCubemap( faces );
    // ----------
    // Skybox END
    

    // Load models
    Model TeaPot_Plane( "res/models/TeaPot.obj" );
    glm::mat4 projection = glm::perspective( camera.GetZoom( ), ( float )SCREEN_WIDTH/( float )SCREEN_HEIGHT, 0.1f, 100.0f );
    
    InitializeImGui();
    
    // OpenGL state
    // ------------
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Shader Setup for Rendering Text
    // -------------------------------
    Shader textShader("res/shaders/text.vs", "res/shaders/text.frag");
    textShader.Use();
    glm::mat4 projection_text = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(textShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection_text));
    
    Text text;
    
    // Framebuffer Setup START
    // -----------------------
    framebufferProgram.Use();
    glUniform1i(glGetUniformLocation(framebufferProgram.Program, "screenTexture"), 0);
    
    // Prepare framebuffer rectangle VBO and VAO
    unsigned int rectVAO, rectVBO;
    glGenVertexArrays(1, &rectVAO);
    glGenBuffers(1, &rectVBO);
    glBindVertexArray(rectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
    unsigned int FBO;
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    unsigned int framebufferTexture;
    glGenTextures(1, &framebufferTexture);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

    unsigned int RBO;
    glGenRenderbuffers(1, &RBO);
    glBindRenderbuffer(GL_RENDERBUFFER, RBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCREEN_WIDTH, SCREEN_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

    auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Framebuffer error: " << fboStatus << std::endl;
    }
    // -----------------------
    // Framebuffer Setup END
    
    
    // Game loop
    while( !glfwWindowShouldClose( window ) )
    {
        // Camera On Off Logic
        if (!cameraOn)
        {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
        }
        else
        {
            glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        }
        
        // Set Frame Time
        GLfloat currentFrame = glfwGetTime( );
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Check and call events
        glfwPollEvents( );
        DoMovement( );
        
        // Bind the custom framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        // Specify the color of the background
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        // Clean the back buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // Enable depth testing since it's disabled when drawing the framebuffer rectangle
        glEnable(GL_DEPTH_TEST);
        

        glm::mat4 view = camera.GetViewMatrix( );
        glm::mat4 model = glm::mat4(1.0f);
       
        // Drawing the model
        blinnPhongShader.Use( );
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "projection" ), 1, GL_FALSE, glm::value_ptr( projection ) );
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "view" ), 1, GL_FALSE, glm::value_ptr( view ) );
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.5f));
        glUniformMatrix4fv( glGetUniformLocation( blinnPhongShader.Program, "model" ), 1, GL_FALSE, glm::value_ptr( model ) );
        TeaPot_Plane.Draw( blinnPhongShader );
        blinnPhongLighting( blinnPhongShader );
        
        
        // Drawing skybox
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
                
        // Binding the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        framebufferProgram.Use();
        
        scale = glm::vec4(scaleFloat);
        bias = glm::vec4(biasFloat);
        
        // Setting the values in Framebuffer
        glUniform4f( glGetUniformLocation( framebufferProgram.Program, "uScale" ), scale.x, scale.y, scale.z, scale.w );
        glUniform4f( glGetUniformLocation( framebufferProgram.Program, "uBias" ), bias.x, bias.y, bias.z, bias.w );
        
        glUniform1i( glGetUniformLocation( framebufferProgram.Program, "uGhosts" ), numGhosts );
        glUniform1f( glGetUniformLocation( framebufferProgram.Program, "uGhostDispersal" ), ghostDispersal );
        glUniform1f( glGetUniformLocation( framebufferProgram.Program, "uGhostWeight" ), ghostWeight );
        
        glUniform1f( glGetUniformLocation( framebufferProgram.Program, "uHaloWidth" ), haloWidth );
        glUniform1f( glGetUniformLocation( framebufferProgram.Program, "uHaloWeight" ), haloWeight );
        
        glUniform1f( glGetUniformLocation( framebufferProgram.Program, "uDistortion" ), distortion );
        
        glUniform1i( glGetUniformLocation( framebufferProgram.Program, "uBlurIt" ), blurIt ? 1 : 0 );
        
        
        glBindVertexArray(rectVAO);
        glBindTexture(GL_TEXTURE_2D, framebufferTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // ImGui
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
    
    if ( keys[GLFW_KEY_M] )
    {
        cameraOn = false;
    }
    if ( keys[GLFW_KEY_C] )
    {
        cameraOn = true;
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
    if (cameraOn)
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
    if (ImGui::CollapsingHeader("Lens Flare")) {
        
        if (ImGui::TreeNode("Scale")) {
            
            if (ImGui::BeginTable("ThresholdAttrib_1", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Scale", &scaleFloat, 0, 1);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Bias")) {
            
            if (ImGui::BeginTable("ThresholdAttrib_2", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Bias", &biasFloat, 0, 1);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Ghost Params")) {
            
            if (ImGui::BeginTable("Ghosts", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderInt("Number", &numGhosts, 1, 20);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Dispersal", &ghostDispersal, -1.0, 0.5);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Weight", &ghostWeight, 2.0, 20.0);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Halo Params")) {
            
            if (ImGui::BeginTable("Halo", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Width", &haloWidth, 0, 0.5);
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Weidht", &haloWeight, 5.0, 20.0);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Distortion Params")) {
            
            if (ImGui::BeginTable("Distortion", 1))
            {
                ImGui::TableNextColumn();
                ImGui::SliderFloat("Distortion", &distortion, -0.05, 0.05);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        
        if (ImGui::TreeNode("Blurring")) {
            
            if (ImGui::BeginTable("Blurring", 1))
            {
                ImGui::TableNextColumn();
                ImGui::Checkbox("Blur", &blurIt);
                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }

 
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

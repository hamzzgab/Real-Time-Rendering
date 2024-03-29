#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
GLfloat LodBias = 0.0f;

bool NearNear = false;
bool NearLinear = false;

bool LinearNear = false;
bool LinearLinear = true;

bool MagLinear = false;
bool MagNearest = true;

// [2] Farhaan Hussain. (2016, August 28). Modern OpenGL 3.0+ Tutorials [Playlist]. YouTube. https://www.youtube.com/playlist?list=PLRtjMdoYXLf6zUMDJVRZYV-6g6n62vet8
struct Vertex
{
    // POSITION
    glm::vec3 Position;
    
    // NORMAL
    glm::vec3 Normal;
    
    // TEXTURE COORDINATES
    glm::vec2 TexCoords;
    
    // Tangent
    glm::vec3 Tangent;
};

struct Texture
{
    GLuint id;
    string type;
    aiString path;
};

class Mesh
{
public:
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vector<Texture> textures;

    Mesh( vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures )
    {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        
        this->setupMesh( );
    }
    
    void Draw( Shader shader )
    {
        GLuint diffuseNr = 1;
        GLuint specularNr = 1;
        GLuint normalNr = 1;
        
        for( GLuint i = 0; i < this->textures.size( ); i++ )
        {
            glActiveTexture( GL_TEXTURE0 + i );
            
            stringstream ss;
            string number;
            string name = this->textures[i].type;
            
            if( name == "texture_diffuse" )
            {
                ss << diffuseNr++;
            }
            else if( name == "texture_specular" )
            {
                ss << specularNr++;
            }else if ( name == "texture_normal" )
            {
                ss << normalNr++;
            }
            
            number = ss.str( );
            
            glUniform1i( glGetUniformLocation( shader.Program, ( name + number ).c_str( ) ), i );
            glBindTexture( GL_TEXTURE_2D, this->textures[i].id );
            
        }
        
        
        glUniform1f( glGetUniformLocation( shader.Program, "material.shininess" ), 16.0f );
        
        glBindVertexArray( this->VAO );
        glDrawElements( GL_TRIANGLES, this->indices.size( ), GL_UNSIGNED_INT, 0 );
        glBindVertexArray( 0 );
        
        for ( GLuint i = 0; i < this->textures.size( ); i++ )
        {
            glActiveTexture( GL_TEXTURE0 + i );
            glBindTexture( GL_TEXTURE_2D, 0 );
        }
    }
    
private:
    GLuint VAO, VBO, EBO;
    
    void setupMesh( )
    {
        glGenVertexArrays( 1, &this->VAO );
        glGenBuffers( 1, &this->VBO );
        glGenBuffers( 1, &this->EBO );
        
        glBindVertexArray( this->VAO );
        
        glBindBuffer( GL_ARRAY_BUFFER, this->VBO );
        
        glBufferData( GL_ARRAY_BUFFER, this->vertices.size( ) * sizeof( Vertex ), &this->vertices[0], GL_STATIC_DRAW );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->EBO );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, this->indices.size( ) * sizeof( GLuint ), &this->indices[0], GL_STATIC_DRAW );
        
        glEnableVertexAttribArray( 0 );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid * )0 );
        
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid * )offsetof( Vertex, Normal ) );
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid * )offsetof( Vertex, TexCoords ) );
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer( 3, 2, GL_FLOAT, GL_FALSE, sizeof( Vertex ), ( GLvoid * )offsetof( Vertex, Tangent ) );
        
        glBindVertexArray( 0 );
    }
};



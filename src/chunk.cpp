#include "chunk.h"
#include <la.h>
#include <glwidget277.h>
#include <shaderprogram.h>
#include <iostream>

#define CX 16
#define CY 16
#define CZ 16

#define SCX 64
#define SCY 64
#define SCZ 64

struct VertexData{
    glm::vec4 pos;
    glm::vec4 nor;
    glm::vec2 uv;
    glm::vec3 tang;
    glm::vec3 bitang;
};

chunk::chunk(GLWidget277* context)
{
    this->context = context;
    memset(block, 0, sizeof(block));
    elements = 0;
    changed = true;
    this->context->glGenBuffers(1, &vbo);
}

chunk::~chunk(){
    context->glDeleteBuffers(1,&vbo);
}
int chunk::get(int x, int y, int z){
    return block[x][y][z];
}

void chunk::set(int x, int y, int z, int type){
    block[x][y][z] = type;
    changed = true;
}

void chunk::update(){
    changed = false;
    std::vector<VertexData> vertex;
    int i = 0;
    for(int x = 0; x < CX; x++)
        for(int y = 0; y < CY; y++)
            for(int z = 0; z < CZ; z++){
                int type = block[x][y][z];
                VertexData vert;
                //Empty Block?
                if(type == 0)
                    continue;
                //create a offset to decide real UV
                glm::vec2 offset_top = glm::vec2(0.0f, 0.0f);
                glm::vec2 offset_bottom = glm::vec2(0.0f, 0.0f);
                glm::vec2 offset_side = glm::vec2(0.0f, 0.0f);
                if(type == 1){
                    //DIRT:
                    offset_top += 2.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_top += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    offset_side += 2.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_side += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    offset_bottom += 2.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_bottom += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                }
                else if(type == 2){
                    //Grass:
                    //Grass Top
                    offset_top += 8.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_top += 2.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    //Grass Side
                    offset_side += 3.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_side += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    //Grass Bottom
                    offset_bottom += 2.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_bottom += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                }
                else if(type == 3){
                    //Stone:
                    offset_top += 1.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_top += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    offset_side += 1.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_side += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                    offset_bottom += 1.0f * glm::vec2(1.0f / 16.0f, 0.0f);
                    offset_bottom += 0.0f * glm::vec2(0.0f, 1.0f / 16.0f);
                }

                glm::vec3 pos1, pos2, pos3, edge1, edge2;
                glm::vec2 uv1, uv2, uv3, deltaUV1, deltaUV2;
                GLfloat f;
                //View from negative x
                if(x > 0 && !block[x-1][y][z] || x == 0){
                    vert.nor = (glm::vec4(-1.0f  ,0.0f  ,0.0f  ,      1));
                    //first triangle
                    pos1 = glm::vec3(x-0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x-0.5f   ,y-0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y+0.5f  ,z-0.5f);
                    uv1  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_side;

                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x-0.5f   ,y+0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x-0.5f   ,y-0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y+0.5f  ,z+0.5f);
                    uv1  = glm::vec2(0.0f, 0.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv3  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_side;

                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
                //View from positive x
                if(x < CX-1 && !block[x+1][y][z] || x == CX-1){
                    vert.nor = (glm::vec4(1.0f   ,0  ,0  ,      1));
                    //first triangle
                    pos1 = glm::vec3(x+0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y-0.5f  ,z+0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_side;

                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y+0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y-0.5f  ,z+0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_side;
                    uv2  = glm::vec2(0.0f, 0.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_side;

                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
                //View from negative y ------ Bottom
                if(y > 0 && !block[x][y-1][z] || y == 0){
                    vert.nor = (glm::vec4(0   ,-1.0f ,0  ,      1));
                    //first triangle
                    pos1 = glm::vec3(x-0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y-0.5f  ,z-0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y-0.5f  ,z+0.5f);
                    uv1  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_bottom;
                    uv2  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_bottom;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_bottom;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x+0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y-0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y-0.5f  ,z+0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_bottom;
                    uv2  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_bottom;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_bottom;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);
                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
                //View from positive y ------ Top
                if(y < CY-1 && !block[x][y+1][z] || y == CY-1){
                    vert.nor = (glm::vec4(0   ,1.0f  ,0  ,      1));
                    //first triangle
                    pos1 = glm::vec3(x-0.5f   ,y+0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x-0.5f   ,y+0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    uv1  = glm::vec2(0.0f, 0.0f) + offset_top;
                    uv2  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_top;
                    uv3  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_top;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x-0.5f   ,y+0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y+0.5f  ,z+0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_top;
                    uv2  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_top;
                    uv3  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_top;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
                //View from nagative z
                if(z > 0 && !block[x][y][z-1] || z == 0){
                    vert.nor = (glm::vec4(0   ,0  ,-1.0f ,      1));
                    //first triangle
                    pos1 = glm::vec3(x-0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x-0.5f   ,y+0.5f  ,z-0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_side;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x-0.5f   ,y-0.5f  ,z-0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y+0.5f  ,z-0.5f);
                    pos3 = glm::vec3(x+0.5f   ,y-0.5f  ,z-0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(0.0f, 0.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_side;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
                //View from positive z
                if(z < CZ-1 && !block[x][y][z+1] || z == CZ-1){
                    vert.nor = (glm::vec4(0   ,0  ,1.0f  ,      1));
                    //first triangle
                    pos1 = glm::vec3(x-0.5f   ,y-0.5f  ,z+0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y-0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y+0.5f  ,z+0.5f);
                    uv1  = glm::vec2(0.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_side;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                    //second triangle
                    pos1 = glm::vec3(x+0.5f   ,y-0.5f  ,z+0.5f);
                    pos2 = glm::vec3(x+0.5f   ,y+0.5f  ,z+0.5f);
                    pos3 = glm::vec3(x-0.5f   ,y+0.5f  ,z+0.5f);
                    uv1  = glm::vec2(1.0f / 16.0f, 1.0f / 16.0f) + offset_side;
                    uv2  = glm::vec2(1.0f / 16.0f, 0.0f) + offset_side;
                    uv3  = glm::vec2(0.0f, 0.0f) + offset_side;
                    //Compute tang and bitang:
                    edge1 = pos2 - pos1;
                    edge2 = pos3 - pos1;
                    deltaUV1 = uv2 - uv1;
                    deltaUV2 = uv3 - uv1;
                    f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
                    vert.tang.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
                    vert.tang.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
                    vert.tang.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
                    vert.tang = glm::normalize(vert.tang);

                    vert.bitang.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
                    vert.bitang.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
                    vert.bitang.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
                    vert.bitang = glm::normalize(vert.bitang);

                    vert.pos = (glm::vec4(pos1  ,   1));
                    vert.uv  = uv1;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos2  ,   1));
                    vert.uv  = uv2;
                    vertex.push_back(vert);
                    vert.pos = (glm::vec4(pos3  ,   1));
                    vert.uv  = uv3;
                    vertex.push_back(vert);
                }
            }
    elements = vertex.size();
    context->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    context->glBufferData(GL_ARRAY_BUFFER, elements * sizeof(VertexData), vertex.data(), GL_STATIC_DRAW);
}

void chunk::render(ShaderProgram prog){
    if(this->changed)
        this->update();

    //If the chunk is empty, we don't draw
    if(!elements)
        return;

    prog.useMe();
    context->glEnable(GL_CULL_FACE);
    context->glEnable(GL_DEPTH_TEST);

    context->glBindBuffer(GL_ARRAY_BUFFER, vbo);
    GLint attribute_coord = -1, attribute_normal = -1, attribute_uv = -1, attribute_tangent = -1, attribute_bitangent = -1;
    attribute_coord  = context->glGetAttribLocation(prog.prog, "vs_coord");
    attribute_normal = context->glGetAttribLocation(prog.prog, "vs_nor");
    attribute_uv     = context->glGetAttribLocation(prog.prog, "vs_UV");
    attribute_tangent= context->glGetAttribLocation(prog.prog, "vs_tangent");
    attribute_bitangent= context->glGetAttribLocation(prog.prog, "vs_bitangent");

    if (attribute_coord != -1) {
        context->glEnableVertexAttribArray(attribute_coord);
        context->glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)0);
    }
    if (attribute_normal != -1) {
        context->glEnableVertexAttribArray(attribute_normal);
        context->glVertexAttribPointer(attribute_normal, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)sizeof(glm::vec4));
    }
    if (attribute_uv != -1) {
        context->glEnableVertexAttribArray(attribute_uv);
        context->glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(glm::vec4) * 2));
    }
    if (attribute_tangent != -1) {
        context->glEnableVertexAttribArray(attribute_tangent);
        context->glVertexAttribPointer(attribute_tangent, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(glm::vec4) * 2 + sizeof(glm::vec2)));
    }
    if (attribute_bitangent != -1) {
        context->glEnableVertexAttribArray(attribute_bitangent);
        context->glVertexAttribPointer(attribute_bitangent, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)(sizeof(glm::vec4) * 2 + sizeof(glm::vec2) + sizeof(glm::vec3)));
    }
    context->glDrawArrays(GL_TRIANGLES, 0, elements);

    if (attribute_coord != -1) context->glDisableVertexAttribArray(attribute_coord);
    if (attribute_normal != -1) context->glDisableVertexAttribArray(attribute_normal);
    if (attribute_uv != -1) context->glDisableVertexAttribArray(attribute_uv);
    if (attribute_tangent != -1) context->glDisableVertexAttribArray(attribute_tangent);
    if (attribute_bitangent != -1) context->glDisableVertexAttribArray(attribute_bitangent);

    context->glDisable(GL_CULL_FACE);
    context->glDisable(GL_DEPTH_TEST);
}

superchunk::superchunk(GLWidget277 *context){
    this->context = context;
    memset(cl, 0, sizeof cl);
}

superchunk::~superchunk(){
    for(int x = 0; x < SCX; x++)
          for(int y = 0; y < SCX; y++)
            for(int z = 0; z < SCX; z++)
              delete cl[x][y][z];
}

int superchunk::get(int x, int y, int z){
    //convert the position with respect to the left-bottom corner chunk, cl[0][0][0]
    x = x - start_pos[0];
    y = y - start_pos[1];
    z = z - start_pos[2];

    int cx = x / CX;
    int cy = y / CY;
    int cz = z / CZ;

    x %= CX;
    y %= CY;
    z %= CZ;

    if(!cl[cx][cy][cz])
        return 0;
    else
        return cl[cx][cy][cz]->get(x, y, z);
}

void superchunk::set(int x, int y, int z, int type){
    //convert the position with respect to the left-bottom corner chunk, cl[0][0][0]
    x = x - start_pos[0];
    y = y - start_pos[1];
    z = z - start_pos[2];

    int cx = x / CX;
    int cy = y / CY;
    int cz = z / CZ;

    x %= CX;
    y %= CY;
    z %= CZ;

    if(!cl[cx][cy][cz])
        cl[cx][cy][cz] = new chunk(context);
    cl[cx][cy][cz]->set(x, y, z, type);
}

void superchunk::render(ShaderProgram prog, glm::mat4 VP){
    for(int x = 0; x < SCX; x++)
        for(int y = 0; y < SCY; y++)
            for(int z = 0; z < SCZ; z++){
                //compute distance between the chunk right now and the chunk of camera
                int distance = abs(x - 32) + abs(y - 32) + abs(z - 32);
                if(distance > 32)
                    continue;
                if(cl[x][y][z]){
                    //std::cout<<x<<" "<<y<<" "<<z<<"\n";
                    glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(x * CX, y * CY, z * CZ) + start_pos);
                    prog.setModelMatrix(model);
//                    //If the chunk is out of the screen, then we don't render it
//                    glm::vec3 center = start_pos + glm::vec3(x*16, y*16, z*16) + glm::vec3(8, 8, 8);
//                    std::cout<<center.x<<" "<<center.y<<" "<<center.z<<" \n";
//                    glm::vec4 coords = VP * model * glm::vec4(center, 1);
//                    coords.x /= coords.w;
//                    coords.y /= coords.w;
//                    float diameter = sqrtf(CX * CX + CY * CY + CZ * CZ);
////                    if(coords[2] < -diameter){
////                        std::cout<<x<<" "<<y<<" "<<z<<"\n";
////                        continue;
////                    }
//                    diameter /= fabsf(coords.w);
//                    if(fabsf(coords.x) > 1 + diameter || fabsf(coords.y) > 1 + diameter){
//                        std::cout<<coords.x<<" "<<coords.y<<"    dasdasdasd:"<<x<<" "<<y<<" "<<z<<"\n";
//                        continue;
//                    }

                    //Reset the type(mainly for stone and grass or dirt)
                    glm::vec3 pos_origin = glm::vec3(x * CX, y * CY, z * CZ) + start_pos;
                    for(int m = 0; m < CX; m++)
                        for(int n = 0; n < CY; n++)
                            for(int k = 0; k < CZ; k++){
                                int block_type = cl[x][y][z]->block[m][n][k];

                                if(block_type == 0){
                                    continue;
                                }
                                //if(block_type is regular type(supposed <= 3))
                                //DIRT is 1, GRASS is 2, STONE is 3
                                if(block_type <= 3){
                                    //if y < -16, then STONE
                                    if((pos_origin.y + n) < -16 && block_type != 3){
                                        cl[x][y][z]->set(m,n,k,3);
                                    }
                                    //else if there is a block above, DIRT
                                    else if(get(pos_origin.x + m, pos_origin.y + n + 1, pos_origin.z + k) && block_type != 1){
                                        cl[x][y][z]->set(m,n,k,1);
                                    }
                                    //else GRASS
                                    else if(!get(pos_origin.x + m, pos_origin.y + n + 1, pos_origin.z + k) && block_type != 2){
                                        cl[x][y][z]->set(m,n,k,2);
                                    }
                                }
                            }

                    cl[x][y][z]->render(prog);
                }
            }
}

void superchunk::MoveUpdate(int m, std::map<tuple, Block*> map){
    if(m == 0){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(16,0,0);
        //delete the -x-end line
        for(int y = 0; y < SCY; y++)
            for(int z = 0; z < SCZ; z++){
                if(cl[0][y][z]){
                    delete cl[0][y][z];
                    cl[0][y][z] = nullptr;
                }
            }
        //shift to -x
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++)
                for(int z = 0; z < SCZ; z++){
                    if(x == SCX-1){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x+1][y][z];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= (start_pos[0] + 63 * 16) && x <= (start_pos[0] + 64 * 16) &&
                    y >= start_pos[1] && y <= (start_pos[1] + 64 * 16) &&
                    z >= start_pos[2] && z <= (start_pos[2] + 64 * 16)){
                set(x,y,z,int(1));
            }
        }
    }
    else if(m == 1){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(-16,0,0);
        //delete the +x-end line
        for(int y = 0; y < SCY; y++)
            for(int z = 0; z < SCZ; z++){
                if(cl[63][y][z]){
                    cl[63][y][z] = nullptr;
                }
            }
        //shift to +x
        for(int x = SCX - 1; x >= 0; x--)
            for(int y = 0; y < SCY; y++)
                for(int z = 0; z < SCZ; z++){
                    if(x == 0){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x-1][y][z];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= start_pos[0] && x <= (start_pos[0] + 16) &&
                    y >= start_pos[1] && y <= (start_pos[1] + 64 * 16) &&
                    z >= start_pos[2] && z <= (start_pos[2] + 64 * 16)){
                set(x,y,z,int(1));
            }
        }
    }
    else if(m == 2){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(0,16,0);
        //delete the -y-end line
        for(int x = 0; x < SCX; x++)
            for(int z = 0; z < SCZ; z++){
                if(cl[x][0][z]){
                    cl[x][0][z] = nullptr;
                }
            }
        //shift to -y
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++)
                for(int z = 0; z < SCZ; z++){
                    if(y == SCY-1){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x][y+1][z];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= start_pos[0] && x <= (start_pos[0] + 64 * 16) &&
                    y >= (start_pos[1] + 63 * 16) && y <= (start_pos[1] + 64 * 16) &&
                    z >= start_pos[2] && z <= (start_pos[2] + 64 * 16)){
                set(x,y,z,int(1));
            }
        }
    }
    else if(m == 3){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(0,-16,0);
        //delete the +y-end line
        for(int x = 0; x < SCX; x++)
            for(int z = 0; z < SCZ; z++){
                if(cl[x][63][z]){
                    cl[x][63][z] = nullptr;
                }
            }
        //shift to +x
        for(int x = 0; x < SCX; x++)
            for(int y = SCY - 1; y >= 0; y--)
                for(int z = 0; z < SCZ; z++){
                    if(y == 0){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x][y-1][z];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= start_pos[0] && x <= (start_pos[0] + 64 * 16) &&
                    y >= start_pos[1] && y <= (start_pos[1] + 16) &&
                    z >= start_pos[2] && z <= (start_pos[2] + 64 * 16)){
                set(x,y,z,int(1));
            }
        }
    }
    else if(m == 4){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(0,0,16);
        //delete the -z-end line
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++){
                if(cl[x][y][0]){
                    cl[x][y][0] = nullptr;
                }
            }
        //shift to -z
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++)
                for(int z = 0; z < SCZ; z++){
                    if(z == SCZ-1){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x][y][z+1];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= start_pos[0] && x <= (start_pos[0] + 64 * 16) &&
                    y >= start_pos[1] && y <= (start_pos[1] + 64 * 16) &&
                    z >= (start_pos[2] + 63 * 16) && z <= (start_pos[2] + 64 * 16)){
                set(x,y,z,int(1));
            }
        }
    }
    else if(m == 5){
        //set the new start_pos
        start_pos = start_pos + glm::vec3(0,0,-16);
        //delete the +z-end line
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++){
                if(cl[x][y][63]){
                    cl[x][y][63] = nullptr;
                }
            }
        //shift to +z
        for(int x = 0; x < SCX; x++)
            for(int y = 0; y < SCY; y++)
                for(int z = SCZ - 1; z >= 0; z--){
                    if(z == 0){
                        cl[x][y][z] = nullptr;
                    }
                    else{
                        cl[x][y][z] = cl[x][y][z-1];
                    }
                }
        //set new line
        std::map<tuple, Block*>::iterator iter;
        for (iter = map.begin(); iter != map.end(); iter++)
        {
            // iter->first is a tuple
            // std::get<index>(tuple) can get the elements in the tuple
            int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
            if(x >= start_pos[0] && x <= (start_pos[0] + 64 * 16) &&
                    y >= start_pos[1] && y <= (start_pos[1] + 64 * 16) &&
                    z >= start_pos[2] && z <= (start_pos[2] + 16)){
                set(x,y,z,int(1));
            }
        }
    }
}

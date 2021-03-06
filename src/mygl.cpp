#define _USE_MATH_DEFINES
#include "mygl.h"
#include <la.h>

#include <iostream>
#include <QApplication>
#include <QKeyEvent>
#include <math.h>
#include<QMessageBox>
#include <QDir>
#include<time.h>

#include <SOIL.h>

const int MaxReachDistance=8;
MyGL::MyGL(QWidget *parent)
    : GLWidget277(parent),
      gl_camera(), gl_skyboxCamera(), geom_cube(this),Rivers(&scene),center(this),T(this),
      prog_lambert(this), prog_flat(this), prog_new(this), prog_shadow(this),prog_skybox(this), prog_particle(this), rainParticle(this),
      grid(this),mousemove(false),game_begin(false),jump_state(false),block_type(DIRT),block_type_Origin(DIRT),
      Step_type(DIRT),Fetch_Mode(PICKMODE),g_velocity(0),external_force_acceleration(-gravity_acceleration),
      character_size(0,0,0),StepSound("../miniminecraft/sound/step2.wav"),
      WaterStepSound("../miniminecraft/sound/stepwater.wav"),
      Ground("../miniminecraft/sound/ground.wav"),WaterGround("../miniminecraft/sound/waterground.wav"),
      RiverSound(NULL),FireSound(NULL),skybox(this)
{
    ItemMenu.resize(12);
    Item.resize(12);
    ItemNumber.resize(24);
    for(int i=0;i<12;i++)
    {
        ItemMenu[i]=new ItemList(this);
        Item[i]=new ItemList(this);
        ItemNumber[2*i]=new ItemList(this);
        ItemNumber[2*i+1]=new ItemList(this);
    }

    // Connect the timer to a function so that when the timer ticks the function is executed
    connect(&timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    // Tell the timer to redraw 60 times per second
    timer.start(16);
    setFocusPolicy(Qt::ClickFocus);

}

MyGL::~MyGL()
{
    makeCurrent();
    glDeleteVertexArrays(1, &vao);
    for(int i=0;i<12;i++)
    {
        delete ItemMenu[i];
        delete Item[i];
        delete ItemNumber[2*i];
        delete ItemNumber[2*i+1];
    }
    geom_cube.destroy();
    center.destroy();
    T.destroy();
}

void MyGL::initializeGL()
{
    // Create an OpenGL context using Qt's QOpenGLFunctions_3_2_Core class
    // If you were programming in a non-Qt context you might use GLEW (GL Extension Wrangler)instead
    initializeOpenGLFunctions();
    // Print out some information about the current OpenGL context
    debugContextVersion();

    // Set a few settings/modes in OpenGL rendering
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    // Set the size with which points should be rendered
    glPointSize(5);
    // Set the color with which the screen is filled at the start of each render call.
//25.25.112
    glClearColor(0.37f, 0.74f, 1.0f, 1);

    printGLErrorLog();

    // Create a Vertex Attribute Object
    glGenVertexArrays(1, &vao);

    //Create the instance of Cube
    geom_cube.create();

    // Create and set up the diffuse shader
    prog_lambert.create(":/glsl/lambert.vert.glsl", ":/glsl/lambert.frag.glsl");
    // Create and set up the flat lighting shader
    prog_flat.create(":/glsl/flat.vert.glsl", ":/glsl/flat.frag.glsl");

    prog_new.create(":/glsl/new.vert.glsl", ":/glsl/new.frag.glsl");

    prog_shadow.create(":/glsl/shadow.vert.glsl", ":/glsl/shadow.frag.glsl");

    prog_skybox.create(":/glsl/skybox.vert.glsl", ":/glsl/skybox.frag.glsl");

    prog_particle.create(":/glsl/particle.vert.glsl", ":/glsl/particle.geom.glsl", ":/glsl/particle.frag.glsl");

    // Set a color with which to draw geometry since you won't have one
    // defined until you implement the Node classes.
    // This makes your geometry render green.
    prog_lambert.setGeometryColor(glm::vec4(0,1,0,1));
    prog_new.setGeometryColor(glm::vec4(0,1,0,1));
    prog_shadow.setGeometryColor(glm::vec4(0,1,0,1));
    //prog_new.setTexture();
    // We have to have a VAO bound in OpenGL 3.2 Core. But if we're not
    // using multiple VAOs, we can just bind one once.
//    vao.bind();
    glBindVertexArray(vao);

    RiverSound.setSource(QUrl::fromLocalFile("../miniminecraft/sound/rivervoice.wav"));
    FireSound.setSource(QUrl::fromLocalFile("../miniminecraft/sound/fire.wav"));
    character_size=glm::vec3(0.6,2,0.6);
    for(int i=0;i<25;i++)
        keyboard[i]=false;
    for(int i=0;i<15;i++)
        Resource_Num[i]=0;
    Resource_Num[4]=50;
    Resource_Digit_Num[4].setDecade(5);
    Resource_Digit_Num[4].setUnit(0);
    scene.Create();
    Rivers.L_System_River(glm::vec3(70,0,70),10,150,LEFT,STRAIGHT_RIVER);
    Rivers.L_System_River(glm::vec3(0,0,-70),3,120,FORWARD,WINDING_RIVER);
    waterblocks=Rivers.GetRiverBlocks();

    for(int i=0;i<6;i++)
    {
        ItemMenu[i]->Initialize(width(),height(),glm::vec4(50,height()-50-i*75,0,1),35);
        ItemMenu[i]->create();
        Item[i]->Initialize(width(),height(),glm::vec4(50,height()-50-i*75,0,1),30);
        Item[i]->create();
        Item[i]->ChangeColor(glm::vec4(1,1,1,1));
        Item[i]->BuildBuffer();

        ItemNumber[i*2]->Initialize(width(),height(),glm::vec4(50+8,height()-50-i*75-20,0,1),10);
        ItemNumber[i*2+1]->Initialize(width(),height(),glm::vec4(50+20,height()-50-i*75-20,0,1),10);
        ItemNumber[i*2]->create();
        ItemNumber[i*2+1]->create();
        ItemNumber[i*2]->ChangeColor(glm::vec4(0,0,0,1));
        ItemNumber[i*2]->BuildBuffer();
        ItemNumber[i*2+1]->ChangeColor(glm::vec4(0,0,0,1));
        ItemNumber[i*2+1]->BuildBuffer();
    }
    for(int i=6;i<9;i++)
    {
        ItemMenu[i]->Initialize(width(),height(),glm::vec4(125,height()-50-(i-6)*75,0,1),35);
        ItemMenu[i]->create();
        Item[i]->Initialize(width(),height(),glm::vec4(125,height()-50-(i-6)*75,0,1),30);
        Item[i]->create();
        Item[i]->ChangeColor(glm::vec4(1,1,1,1));
        Item[i]->BuildBuffer();

        ItemNumber[i*2]->Initialize(width(),height(),glm::vec4(125+8,height()-50-(i-6)*75-20,0,1),10);
        ItemNumber[i*2+1]->Initialize(width(),height(),glm::vec4(125+20,height()-50-(i-6)*75-20,0,1),10);
        ItemNumber[i*2]->create();
        ItemNumber[i*2+1]->create();
        ItemNumber[i*2]->ChangeColor(glm::vec4(0,0,0,1));
        ItemNumber[i*2]->BuildBuffer();
        ItemNumber[i*2+1]->ChangeColor(glm::vec4(0,0,0,1));
        ItemNumber[i*2+1]->BuildBuffer();
    }
    ItemMenu[0]->ChangeColor(glm::vec4(1,0,0,1));
    ItemMenu[0]->BuildBuffer();
    center.InitializeScreenSize(width(),height());
    T.InitializeScreenSize(width(),height());
    center.create();
    T.create();
    initializeGrid();

    skybox.create();
    rainParticle.create();

    game_begin = true;
}

void MyGL::resizeGL(int w, int h)
{
    //This code sets the concatenated view and perspective projection matrices used for
    //our scene's camera view.
//    gl_camera = Camera(w, h);
    gl_camera = Camera(w, h, glm::vec3(0, 20, 0),
                       glm::vec3(1, 20, 1), glm::vec3(0,1,0));
    glm::mat4 viewproj = gl_camera.getViewProj();

    gl_skyboxCamera = Camera(w, h, glm::vec3(0, 0, 0), glm::vec3(1, 0, 1), glm::vec3(0, 1, 0));
    // Upload the view-projection matrix to our shaders (i.e. onto the graphics card)

    prog_lambert.setViewProjMatrix(viewproj);
    prog_flat.setViewProjMatrix(viewproj);
    prog_particle.setViewProjMatrix(viewproj);

    prog_skybox.setViewProjMatrix(gl_skyboxCamera.getViewProj());

    printGLErrorLog();
}

//This function is called by Qt any time your GL window is supposed to update
//For example, when the function updateGL is called, paintGL is called implicitly.
//DO NOT CONSTRUCT YOUR SCENE GRAPH IN THIS FUNCTION!
void MyGL::paintGL()
{
    // Clear the screen so that we only see newly drawn images
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//Shadow Pass
    prog_lambert.setViewProjMatrix(gl_camera.getViewProj());
    prog_shadow.setShadowTexture();
    if(OpenDNcycle == 1)
        prog_shadow.ComputeLightPVMatrix(Daytime);
    else
        prog_shadow.ComputeLightPVMatrix(0);
    prog_shadow.setShadowBias_PVmatrix(Daytime);
    glViewport(0, 0, prog_shadow.SHADOW_WIDTH, prog_shadow.SHADOW_HEIGHT);
    prog_shadow.context->glBindFramebuffer(GL_FRAMEBUFFER, prog_shadow.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);
    grid.render(prog_shadow, gl_camera.getViewProj(), false);

//Render Pass
    glViewport(0, 0, this->width(), this->height());
    prog_new.context->glBindFramebuffer(GL_FRAMEBUFFER, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog_new.setViewProjMatrix(gl_camera.getViewProj());
    prog_new.setViewPos(gl_camera.eye);
    prog_new.setTexture(prog_shadow.depthMap);
    //Open DNcycle or not
    float skyColorFactor = 1.f;
    if(OpenDNcycle == 1){
        prog_new.setDNcycle(OpenDNcycle);
        skyColorFactor = prog_new.ComputeLightPVMatrix(Daytime);
    }
    else{
        prog_new.setDNcycle(OpenDNcycle);
        prog_new.ComputeLightPVMatrix(0);
    }
    prog_new.setShadowBias_PVmatrix(Daytime);

    glDisable(GL_DEPTH_TEST);
    prog_skybox.setSkyColorFactor(skyColorFactor);
    prog_skybox.setViewProjMatrix(gl_skyboxCamera.getViewProj());
    prog_skybox.setSkyboxTexture();
    prog_skybox.draw(skybox);
    glEnable(GL_DEPTH_TEST);

    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    grid.render(prog_new, gl_camera.getViewProj(), true);
//Release the memory
    glDisable(GL_CULL_FACE);
    prog_new.deleteTexture(prog_shadow.depthMap);
    prog_shadow.context->glDeleteFramebuffers(1, &prog_shadow.depthMapFBO);

    prog_flat.setModelMatrix(glm::mat4(1));

    glDisable(GL_DEPTH_TEST);
    prog_flat.setTextureFlag(0);
    prog_flat.draw(center);
    prog_flat.draw(T);
    for(int i=0;i<12;i++)
        prog_flat.draw(*(ItemMenu[i]));

    prog_flat.setTextureFlag(1);
    prog_flat.setFlatTexture(1);
    Item[0]->SetTexture(DIRT,1);
    prog_flat.draw(*(Item[0]));
    Item[1]->SetTexture(GRASS,1);
    prog_flat.draw(*(Item[1]));
    Item[2]->SetTexture(STONE,1);
    prog_flat.draw(*(Item[2]));
    Item[3]->SetTexture(WOOD,1);
    prog_flat.draw(*(Item[3]));
    Item[4]->SetTexture(LEAF,1);
    prog_flat.draw(*(Item[4]));
    Item[5]->SetTexture(BEDROCK,1);
    prog_flat.draw(*(Item[5]));
    Item[6]->SetTexture(COAL,1);
    prog_flat.draw(*(Item[6]));
    Item[7]->SetTexture(IRON_ORE,1);
    prog_flat.draw(*(Item[7]));
    Item[8]->SetTexture(LAVA,1);
    prog_flat.draw(*(Item[8]));


    prog_flat.setFlatTexture(2);
    for(int i=0;i<9;i++)
    {
        if(i==8)
        {
            ItemNumber[2*i]->SetTexture(-1,2);
            ItemNumber[2*i+1]->SetTexture(-1,2);
        }
        else
        {
            ItemNumber[2*i]->SetTexture(Resource_Digit_Num[i+1].getDecade(),2);
            ItemNumber[2*i+1]->SetTexture(Resource_Digit_Num[i+1].getUnit(),2);
        }
        prog_flat.draw(*(ItemNumber[2*i]));
        prog_flat.draw(*(ItemNumber[2*i+1]));
    }
    glEnable(GL_DEPTH_TEST);    prog_flat.deleteTexture(prog_shadow.depthMap);
    prog_flat.setViewProjMatrix(glm::mat4(1));


    prog_particle.setViewProjMatrix(gl_camera.getViewProj());
    prog_particle.setCameraPos(gl_camera.eye, fTime);
    prog_particle.drawParticle(rainParticle);
}

void MyGL::GLDrawScene()
{
//    tuple t(1,2,3);
//    scene.mSceneMap.find(t) != scene.mSceneMap.end();
    std::map<tuple, Block*>::iterator iter;
    for (iter = scene.mSceneMap.begin(); iter != scene.mSceneMap.end(); iter++)
    {
        // iter->first is a tuple
        // std::get<index>(tuple) can get the elements in the tuple
        glm::vec3 trans(std::get<0>(iter->first), std::get<1>(iter->first), std::get<2>(iter->first));
        prog_lambert.setModelMatrix(glm::translate(glm::mat4(), trans));
        prog_lambert.draw(geom_cube);
    }
}

//initialize the grid by the
void MyGL::initializeGrid(){
    int x0 = gl_camera.eye.x ,y0 = gl_camera.eye.y ,z0 = gl_camera.eye.z;
    grid.start_pos = glm::vec3(x0 - 32 * 16, y0 - 32 * 16, z0 - 32 * 16);
    memset(grid.cl, 0, sizeof grid.cl);
    std::map<tuple, Block*>::iterator iter;
    for (iter = scene.mSceneMap.begin(); iter != scene.mSceneMap.end(); iter++)
    {
        // iter->first is a tuple
        // std::get<index>(tuple) can get the elements in the tuple
        int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
//        std::cout<<x<<" "<<y<<" "<<z<<" "<<"\n";
        if(x >= grid.start_pos[0] && x <= (grid.start_pos[0] + 64 * 16) &&
                y >= grid.start_pos[1] && y <= (grid.start_pos[1] + 64 * 16) &&
                z >= grid.start_pos[2] && z <= (grid.start_pos[2] + 64 * 16)){
            //grid.set(x,y,z,int(1));
            grid.set(x, y, z, (int)(iter->second->mType));
        }
//        glm::vec3 trans(std::get<0>(iter->first), std::get<1>(iter->first), std::get<2>(iter->first));
//        prog_lambert.setModelMatrix(glm::translate(glm::mat4(), trans));
//        prog_lambert.draw(geom_cube);
    }
}
void MyGL::UpdateWhenNewTerrain(std::map<tuple, Block *> &New_map){
    std::map<tuple, Block*>::iterator iter;
    for (iter = New_map.begin(); iter != New_map.end(); iter++)
    {
        // iter->first is a tuple
        // std::get<index>(tuple) can get the elements in the tuple
        int x = std::get<0>(iter->first), y = std::get<1>(iter->first), z = std::get<2>(iter->first);
//        std::cout<<x<<" "<<y<<" "<<z<<" "<<"\n";
        if(x >= grid.start_pos[0] && x <= (grid.start_pos[0] + 64 * 16) &&
                y >= grid.start_pos[1] && y <= (grid.start_pos[1] + 64 * 16) &&
                z >= grid.start_pos[2] && z <= (grid.start_pos[2] + 64 * 16)){
            grid.set(x,y,z,(int)(iter->second->mType));
        }
    }
}


void MyGL::breakblocks(QPoint screen_pos, int distance_max, int Mode)
{
    float NDC_X=2*float(screen_pos.x())/float(width())-1;
    float NDC_Y=1-2*float(screen_pos.y())/float(height());
    glm::vec3 p=gl_camera.ref+NDC_X*gl_camera.H+NDC_Y*gl_camera.V;
    std::map<tuple, Block*>::iterator iter;
    for(float i=0;i<distance_max;i+=0.05)
    {
        int x=round(p[0]),y=round(p[1]),z=round(p[2]);
        tuple temp(x,y,z);
        iter=scene.mSceneMap.find(temp);
        if(iter!=scene.mSceneMap.end())
        {
            if(Mode==PICKMODE)
            {
                BLOCK_TYPE type=iter->second->mType;
                if(block_type!=LAVA)
                {
                    if(type!=LAVA)
                    {
                        if(int(type)<9)
                        {
                            if(Resource_Num[int(type)]<99)
                            {
                                Resource_Num[int(type)]++;
                                int decade=Resource_Digit_Num[int(type)].getDecade();
                                int unit=Resource_Digit_Num[int(type)].getUnit();
                                if(unit<9||decade<9)
                                {
                                    unit++;
                                    if(unit==10)
                                    {
                                        unit=0;
                                        decade++;
                                    }
                                    Resource_Digit_Num[int(type)].setDecade(decade);
                                    Resource_Digit_Num[int(type)].setUnit(unit);
                                }
                            }
                        }
                        scene.mSceneMap.erase(iter);
                        grid.set(x,y,z,0);
                    }
                }
                else
                {
                    if(type==GRASS||type==WOOD)
                    {
                        grid.set(x,y,z,9);
                        Fire F;
                        F.setpos(glm::vec3(x,y,z));
                        Fireblocks.push_back(F);
                    }
                }
            }
            else if(Mode==COLLECTMODE)
            {
                std::map<tuple, Block*>::iterator iter_temp;
                for(int px=x-1;px<x+2;px++)
                    for(int py=y-1;py<y+2;py++)
                        for(int pz=z-1;pz<z+2;pz++)
                        {
                            tuple pos(px,py,pz);
                            iter_temp=scene.mSceneMap.find(pos);
                            if(iter_temp!=scene.mSceneMap.end())
                            {

                                BLOCK_TYPE type=iter_temp->second->mType;
                                if(block_type!=LAVA)
                                {
                                    if(type!=LAVA)
                                    {
                                        if(int(type)<9)
                                        {
                                            if(Resource_Num[int(type)]<99)
                                            {
                                                Resource_Num[int(type)]++;
                                                int decade=Resource_Digit_Num[int(type)].getDecade();
                                                int unit=Resource_Digit_Num[int(type)].getUnit();
                                                if(unit<9||decade<9)
                                                {
                                                    unit++;
                                                    if(unit==10)
                                                    {
                                                        unit=0;
                                                        decade++;
                                                    }
                                                    Resource_Digit_Num[int(type)].setDecade(decade);
                                                    Resource_Digit_Num[int(type)].setUnit(unit);
                                                }
                                            }
                                        }
                                        scene.mSceneMap.erase(iter_temp);
                                        grid.set(px,py,pz,0);
                                    }
                                }
                                else
                                {
                                    if(type==GRASS||type==WOOD)
                                    {
                                        iter_temp->second->mType=LAVA;
                                        grid.set(px,py,pz,9);
                                    }
                                }


                            }
                        }
            }
            break;
        }

        p+=(0.05f*glm::normalize(gl_camera.ref-gl_camera.eye));
    }
}

void MyGL::addblocks(QPoint screen_pos, int distance_max,BLOCK_TYPE type)
{
    if(type==LAVA)
        return;
    float NDC_X=2*float(screen_pos.x())/float(width())-1;
    float NDC_Y=1-2*float(screen_pos.y())/float(height());
    glm::vec3 p=gl_camera.ref+NDC_X*gl_camera.H+NDC_Y*gl_camera.V;
    std::map<tuple, Block*>::iterator iter;
    if(Resource_Num[int(type)]<=0)
        return;
    else
    {
        Resource_Num[int(type)]--;
        int decade=Resource_Digit_Num[int(type)].getDecade();
        int unit=Resource_Digit_Num[int(type)].getUnit();
        if(unit>0&&decade>=0)
        {
            unit--;
            if(unit==-1)
            {
                unit=9;
                decade--;
            }
            Resource_Digit_Num[int(type)].setDecade(decade);
            Resource_Digit_Num[int(type)].setUnit(unit);
        }
    }

    for(float i=0;i<distance_max;i+=0.05)
    {
        int x=round(p[0]),y=round(p[1]),z=round(p[2]);
        tuple temp(x,y,z);

        iter=scene.mSceneMap.find(temp);
        if(iter!=scene.mSceneMap.end())
        {
            glm::vec3 cube_center(x,y,z);
            float distance[6];
            distance[0]=p[0]-(cube_center[0]-0.5f);
            distance[1]=(cube_center[0]+0.5f)-p[0];
            distance[2]=p[2]-(cube_center[2]-0.5f);
            distance[3]=(cube_center[2]+0.5f)-p[2];
            distance[4]=p[1]-(cube_center[1]-0.5f);
            distance[5]=(cube_center[1]+0.5f)-p[1];
            float min=distance[0];
            int min_index=0,i=1;
            for(i=1;i<6;i++)
                if(distance[i]<min)
                {
                    min=distance[i];
                    min_index=i;
                }

            Block* pBlock=NULL;
            tuple tempTuple(0,0,0);
            if(min_index==0)
            {
                pBlock = new Block(glm::ivec3(cube_center[0]-1, cube_center[1], cube_center[2]));
                pBlock->mType=type;
                tempTuple=tuple(cube_center[0]-1, cube_center[1], cube_center[2]);
                scene.mSceneMap.insert(std::pair<tuple, Block*>(tempTuple, pBlock));
                grid.set(cube_center[0]-1, cube_center[1], cube_center[2],int(type));
                break;
            }
            if(min_index==1)
            {
                pBlock = new Block(glm::ivec3(cube_center[0]+1, cube_center[1], cube_center[2]));
                pBlock->mType=type;
                tempTuple=tuple(cube_center[0]+1, cube_center[1], cube_center[2]);
                scene.mSceneMap.insert(std::pair<tuple, Block*>(tempTuple, pBlock));
                grid.set(cube_center[0]+1, cube_center[1], cube_center[2],int(type));
                break;
            }
            if(min_index==2)
            {
                pBlock = new Block(glm::ivec3(cube_center[0], cube_center[1], cube_center[2]-1));
                pBlock->mType=type;
                tempTuple=tuple(cube_center[0], cube_center[1], cube_center[2]-1);
                scene.mSceneMap.insert(std::pair<tuple, Block*>(tempTuple, pBlock));
                grid.set(cube_center[0], cube_center[1], cube_center[2]-1,int(type));
                break;
            }
            if(min_index==3)
            {
                pBlock = new Block(glm::ivec3(cube_center[0], cube_center[1], cube_center[2]+1));
                pBlock->mType=type;
                tempTuple=tuple(cube_center[0], cube_center[1], cube_center[2]+1);

                scene.mSceneMap.insert(std::pair<tuple, Block*>(tempTuple, pBlock));
                grid.set(cube_center[0], cube_center[1], cube_center[2]+1,int(type));

                break;
            }
            if(min_index==4)
            {
                //Can't add a cube below the original one
                Resource_Num[int(type)]++;
                break;
            }
            if(min_index==5)
            {
                pBlock = new Block(glm::ivec3(cube_center[0], cube_center[1]+1, cube_center[2]));
                pBlock->mType=type;
                tempTuple=tuple(cube_center[0], cube_center[1]+1, cube_center[2]);
                scene.mSceneMap.insert(std::pair<tuple, Block*>(tempTuple, pBlock));
                grid.set(cube_center[0], cube_center[1]+1, cube_center[2],int(type));
                break;
            }
        }
        p+=(0.05f*glm::normalize(gl_camera.ref-gl_camera.eye));
    }

}

bool MyGL::collision_test(int direction,float step)
{
    glm::vec3 pos1,pos2,pos3,pos4,pos5,pos6,p1,p2;
    glm::vec3 forward_direction=glm::normalize(glm::vec3(gl_camera.look[0],0,gl_camera.look[2])),\
            worldup(0,1,0);
    std::map<tuple, Block*>::iterator iter1,iter2,iter3,iter4,iter5,iter6;
    if(direction==LEFT)
    {
        p1=gl_camera.eye+step*gl_camera.right-0.5f*character_size[0]*gl_camera.right+0.5f*character_size[2]*forward_direction;
        p2=gl_camera.eye+step*gl_camera.right-0.5f*character_size[0]*gl_camera.right-0.5f*character_size[2]*forward_direction;
        pos1=p1+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos2=p2+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos3=p1-0.25f*character_size[1]*worldup;
        pos4=p2-0.25f*character_size[1]*worldup;
        pos5=p1-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos6=p2-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
    }
    if(direction==RIGHT)
    {
        p1=gl_camera.eye+step*gl_camera.right+0.5f*character_size[0]*gl_camera.right+0.5f*character_size[2]*forward_direction;
        p2=gl_camera.eye+step*gl_camera.right+0.5f*character_size[0]*gl_camera.right-0.5f*character_size[2]*forward_direction;
        pos1=p1+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos2=p2+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos3=p1-0.25f*character_size[1]*worldup;
        pos4=p2-0.25f*character_size[1]*worldup;
        pos5=p1-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos6=p2-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
    }
    if(direction==BACK)
    {
        p1=gl_camera.eye+step*forward_direction-0.5f*character_size[2]*forward_direction+0.5f*character_size[0]*gl_camera.right;
        p2=gl_camera.eye+step*forward_direction-0.5f*character_size[2]*forward_direction-0.5f*character_size[0]*gl_camera.right;
        pos1=p1+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos2=p2+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos3=p1-0.25f*character_size[1]*worldup;
        pos4=p2-0.25f*character_size[1]*worldup;
        pos5=p1-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos6=p2-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
    }
    if(direction==FORWARD)
    {
        p1=gl_camera.eye+step*forward_direction+0.5f*character_size[2]*forward_direction+0.5f*character_size[0]*gl_camera.right;
        p2=gl_camera.eye+step*forward_direction+0.5f*character_size[2]*forward_direction-0.5f*character_size[0]*gl_camera.right;;
        pos1=p1+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos2=p2+(0.25f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos3=p1-0.25f*character_size[1]*worldup;
        pos4=p2-0.25f*character_size[1]*worldup;
        pos5=p1-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
        pos6=p2-(0.75f-BODYEDGE_ERROR)*character_size[1]*worldup;
    }

    tuple temp1(round(pos1[0]),round(pos1[1]),round(pos1[2]))\
            ,temp2(round(pos2[0]),round(pos2[1]),round(pos2[2]))\
            ,temp3(round(pos3[0]),round(pos3[1]),round(pos3[2]))\
            ,temp4(round(pos4[0]),round(pos4[1]),round(pos4[2]))\
            ,temp5(round(pos5[0]),round(pos5[1]),round(pos5[2]))\
            ,temp6(round(pos6[0]),round(pos6[1]),round(pos6[2]));
    iter1=scene.mSceneMap.find(temp1);
    iter2=scene.mSceneMap.find(temp2);
    iter3=scene.mSceneMap.find(temp3);
    iter4=scene.mSceneMap.find(temp4);
    iter5=scene.mSceneMap.find(temp5);
    iter6=scene.mSceneMap.find(temp6);
    if(iter1!=scene.mSceneMap.end()|| iter2!=scene.mSceneMap.end()||\
            iter3!=scene.mSceneMap.end()||iter4!=scene.mSceneMap.end()||\
            iter5!=scene.mSceneMap.end()||iter6!=scene.mSceneMap.end())
        return true;
    else
        return false;

}
bool MyGL::bottom_test()
{
    if(g_velocity>0)
        return false;
    glm::vec3 forward_direction=glm::normalize(glm::vec3(gl_camera.look[0],0,gl_camera.look[2]));
    glm::vec3 pos1,pos2,pos3,pos4;
    std::map<tuple, Block*>::iterator iter1,iter2,iter3,iter4;
    pos1=gl_camera.eye-glm::vec3(0,(0.75f+BODYEDGE_ERROR)*character_size[1],0)+(0.5f-BLOCKEDGE_ERROR)*character_size[2]*forward_direction\
            -(0.5f-BLOCKEDGE_ERROR)*character_size[0]*gl_camera.right;
    pos2=gl_camera.eye-glm::vec3(0,(0.75f+BODYEDGE_ERROR)*character_size[1],0)+(0.5f-BLOCKEDGE_ERROR)*character_size[2]*forward_direction\
            +(0.5f-BLOCKEDGE_ERROR)*character_size[0]*gl_camera.right;
    pos3=gl_camera.eye-glm::vec3(0,(0.75f+BODYEDGE_ERROR)*character_size[1],0)-(0.5f-BLOCKEDGE_ERROR)*character_size[2]*forward_direction\
            -(0.5f-BLOCKEDGE_ERROR)*character_size[0]*gl_camera.right;
    pos4=gl_camera.eye-glm::vec3(0,(0.75f+BODYEDGE_ERROR)*character_size[1],0)-(0.5f-BLOCKEDGE_ERROR)*character_size[2]*forward_direction\
            +(0.5f-BLOCKEDGE_ERROR)*character_size[0]*gl_camera.right;
    tuple temp1(round(pos1[0]),round(pos1[1]),round(pos1[2]))\
            ,temp2(round(pos2[0]),round(pos2[1]),round(pos2[2]))\
            ,temp3(round(pos3[0]),round(pos3[1]),round(pos3[2]))\
            ,temp4(round(pos4[0]),round(pos4[1]),round(pos4[2]));
    iter1=scene.mSceneMap.find(temp1);
    iter2=scene.mSceneMap.find(temp2);
    iter3=scene.mSceneMap.find(temp3);
    iter4=scene.mSceneMap.find(temp4);

    if(iter1!=scene.mSceneMap.end())
    {
        Step_type=iter1->second->mType;
        gl_camera.TranslateAlongWorldUp(round(pos1[1])+2-gl_camera.eye[1]);
        return true;
    }
    else if(iter2!=scene.mSceneMap.end())
    {
        Step_type=iter2->second->mType;
        gl_camera.TranslateAlongWorldUp(round(pos2[1])+2-gl_camera.eye[1]);
        return true;
    }
    else if(iter3!=scene.mSceneMap.end())
    {
        Step_type=iter3->second->mType;
        gl_camera.TranslateAlongWorldUp(round(pos3[1])+2-gl_camera.eye[1]);
        return true;
    }
    else if(iter4!=scene.mSceneMap.end())
    {
        Step_type=iter4->second->mType;
        gl_camera.TranslateAlongWorldUp(round(pos4[1])+2-gl_camera.eye[1]);
        return true;
    }
    else
        Step_type=NONE;
    return false;
}

void MyGL::CurrentItemChange()
{
    int ID=int(block_type)-1;
    int ID_Orign=int(block_type_Origin)-1;
    if(ID>=0&&ID_Orign>=0)
    {
        ItemMenu[ID]->ChangeColor(glm::vec4(1,0,0,1));
        ItemMenu[ID]->BuildBuffer();
        ItemMenu[ID_Orign]->ChangeColor(DEFAULT_COLOR);
        ItemMenu[ID_Orign]->BuildBuffer();
        update();
    }
}

void MyGL::playvoice()
{
    if(gl_camera.cameramode==FLYING_MODE)
        return;
    if(Step_type==NONE)
        return;
    if(jump_state)
        return;
    if(Step_type==WATER)
    {
        if(WaterStepSound.isFinished())
            WaterStepSound.play();
    }
    else
    {
        if(StepSound.isFinished())
            StepSound.play();
    }
}
bool MyGL::boundarytest()
{
    if(fabs(gl_camera.eye[0])>500||fabs(gl_camera.eye[1])>500||\
            fabs(gl_camera.eye[2])>500)
        return true;
    return false;

}
double MyGL::WaterDistanceClose()
{
    glm::vec3 distance=gl_camera.eye-waterblocks[0];
    for(int i=1;i<waterblocks.size();i++)
    {
        glm::vec3 d=gl_camera.eye-waterblocks[i];
        if(glm::length(d)<glm::length(distance))
            distance=d;
    }
    return glm::length(distance);

}
double MyGL::FireDistanceClose()
{
    if(Fireblocks.size()==0)
        return -1;
    glm::vec3 distance=gl_camera.eye-Fireblocks[0].getpos();
    for(int i=1;i<Fireblocks.size();i++)
    {
        if(Fireblocks[i].Get_Lifetime()<=0)
            continue;
        else
        {
            glm::vec3 d=gl_camera.eye-Fireblocks[i].getpos();
            if(glm::length(d)<glm::length(distance))
                distance=d;
        }
    }
    return glm::length(distance);

}
void MyGL::FireSpread()
{
    int num=Fireblocks.size();
    for(int i=0;i<num;i++)
    {
        if(Fireblocks[i].Get_Spreadtime()==0)
        {
            glm::vec3 firepos=Fireblocks[i].getpos();
            for(int px=firepos[0]-1;px<firepos[0]+2;px++)
                for(int py=firepos[1]-1;py<firepos[1]+2;py++)
                    for(int pz=firepos[2]-1;pz<firepos[2]+2;pz++)
                    {
                        tuple pos(px,py,pz);
                        std::map<tuple, Block*>::iterator iter_temp;
                        iter_temp=scene.mSceneMap.find(pos);
                        if(iter_temp!=scene.mSceneMap.end())
                        {
                            BLOCK_TYPE type=iter_temp->second->mType;
                            if(type!=LAVA&&(type==GRASS||type==WOOD))
                            {
                                iter_temp->second->mType=LAVA;
                                grid.set(px,py,pz,9);
                                Fire F;
                                F.setpos(glm::vec3(px,py,pz));
                                Fireblocks.push_back(F);
                            }
                        }
                    }
        }
    }
}
void MyGL::Keyevents()
{
    float amount=0.25f;
    float flyingAmount = 1.0f;
    if(keyboard[0])
        amount=2.0f;
    if(keyboard[1])             //  Key_Escape
        QApplication::quit();
    if(keyboard[2])             //  Key_Right
        gl_camera.RotateAboutUp(-amount);
    if(keyboard[3])             //Key_Left
        gl_camera.RotateAboutUp(amount);
    if(keyboard[4])             //Key_Up
    {
        //gl_camera.RotateAboutRight(-amount);
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongLook(flyingAmount);
        else
        {
            if(collision_test(FORWARD,amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongLook(-amount);
            }
            else
                gl_camera.TranslateAlongLook(amount);
        }
    }
    if (keyboard[5])            //Key_Down
    {
        //gl_camera.RotateAboutRight(amount);
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongLook(-flyingAmount);
        else
        {
            if(collision_test(BACK,-amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongLook(amount);
            }
            else
                gl_camera.TranslateAlongLook(-amount);
        }
    }
    if (keyboard[6])            //Key_1
        gl_camera.fovy += amount;
    if (keyboard[7])            //Key_2
        gl_camera.fovy -= amount;
    if (keyboard[10])           //Key_W
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongLook(flyingAmount);
        else
        {
            playvoice();
            if(collision_test(FORWARD,amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongLook(-amount);
            }
            else
                gl_camera.TranslateAlongLook(amount);
        }
    }
    if (keyboard[11])           //Key_S
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongLook(-flyingAmount);
        else
        {
            playvoice();
            if(collision_test(BACK,-amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongLook(amount);
            }
            else
                gl_camera.TranslateAlongLook(-amount);
        }
    }
    if (keyboard[12])           //Key_D
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongRight(flyingAmount);
        else
        {
            playvoice();
            if(collision_test(RIGHT,amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongRight(-amount);
            }
            else
                gl_camera.TranslateAlongRight(amount);
        }
    }
    if (keyboard[13])           //Key_A
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongRight(-flyingAmount);
        else
        {
            playvoice();
            if(collision_test(LEFT,-amount))
            {
                if(!jump_state)
                    gl_camera.TranslateAlongRight(amount);
            }
            else
                gl_camera.TranslateAlongRight(-amount);
        }
    }
    if(keyboard[14])              //Key_Q
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongUp(-amount);
    }
    if(keyboard[15])              //Key_E
    {
        if(gl_camera.cameramode==FLYING_MODE)
            gl_camera.TranslateAlongUp(amount);
    }
    if(keyboard[16])              //Key_Space
    {
        if(gl_camera.cameramode==WALKING_MODE)
        {
            if(!jump_state)
            {
                g_velocity=2.0f;
                external_force_acceleration=2.0f;
                jump_state=true;
            }
        }
    }
//    if (keyboard[17])             //Key_R
//        gl_camera = Camera(this->width(), this->height(),\
//                           glm::vec3((scene.mMaxXYZ.x - scene.mMinXYZ.x)/2, (scene.mMaxXYZ.y - scene.mMinXYZ.y)/2 + 1, (scene.mMaxXYZ.z - scene.mMinXYZ.z)/2),
//                           glm::vec3((scene.mMaxXYZ.x - scene.mMinXYZ.x)/2, (scene.mMaxXYZ.y - scene.mMinXYZ.y)/2+1, (scene.mMaxXYZ.z - scene.mMinXYZ.z)/2+1),\
//                           glm::vec3(0,1,0));
    gl_camera.RecomputeAttributes();
    update();
}

void MyGL::keyPressEvent(QKeyEvent *e)
{
    if(e->modifiers() & Qt::ShiftModifier){
        keyboard[0]=true;

    // http://doc.qt.io/qt-5/qt.html#Key-enum
    // This could all be much more efficient if a switch
    // statement were used, but I really dislike their
    // syntax so I chose to be lazy and use a long
    // chain of if statements instead
    } else if (e->key() == Qt::Key_Escape) {
        keyboard[1]=true;
    } else if (e->key() == Qt::Key_Right) {
        keyboard[2]=true;
    } else if (e->key() == Qt::Key_Left) {
        keyboard[3]=true;
    } else if (e->key() == Qt::Key_Up) {
        keyboard[4]=true;
    } else if (e->key() == Qt::Key_Down) {
        keyboard[5]=true;
    } else if (e->key() == Qt::Key_Z) {
        keyboard[6]=true;
    } else if (e->key() == Qt::Key_X) {
        keyboard[7]=true;
    } else if (e->key() == Qt::Key_G) {
        gl_camera.cameramode=WALKING_MODE;
    } else if (e->key() == Qt::Key_F) {
        if(gl_camera.cameramode!=FLYING_MODE)
        {
            gl_camera.takeoff=true;
            gl_camera.cameramode=FLYING_MODE;
        }
    } else if (e->key() == Qt::Key_W) {
        keyboard[10]=true;
    } else if (e->key() == Qt::Key_S) {
        keyboard[11]=true;
    } else if (e->key() == Qt::Key_D) {
        keyboard[12]=true;
    } else if (e->key() == Qt::Key_A) {
        keyboard[13]=true;
    } else if (e->key() == Qt::Key_Q) {
        keyboard[14]=true;
    } else if (e->key() == Qt::Key_E) {
        keyboard[15]=true;
    } else if(e->key()==Qt::Key_Space){
        keyboard[16]=true;
    } else if (e->key() == Qt::Key_R) {
        keyboard[17]=true;
    } else if (e->key() == Qt::Key_1) {
        block_type_Origin=block_type;
        block_type=DIRT;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_2) {
        block_type_Origin=block_type;
        block_type=GRASS;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_3) {
        block_type_Origin=block_type;
        block_type=STONE;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_4) {
        block_type_Origin=block_type;
        block_type=WOOD;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_5) {
        block_type_Origin=block_type;
        block_type=LEAF;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_6) {
        block_type_Origin=block_type;
        block_type=BEDROCK;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_7) {
        block_type_Origin=block_type;
        block_type=COAL;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_8) {
        block_type_Origin=block_type;
        block_type=IRON_ORE;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_9) {
        block_type_Origin=block_type;
        block_type=LAVA;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_I) {
        block_type_Origin=block_type;
        if(block_type==DIRT)
            block_type=GRASS;
        else if(block_type==GRASS)
            block_type=STONE;
        else if(block_type==STONE)
            block_type=WOOD;
        else if(block_type==WOOD)
            block_type=LEAF;
        else if(block_type==LEAF)
            block_type=BEDROCK;
        else if(block_type==BEDROCK)
            block_type=COAL;
        else if(block_type==COAL)
            block_type=IRON_ORE;
        else if(block_type==IRON_ORE)
            block_type=LAVA;
        else if(block_type==LAVA)
            block_type=DIRT;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_U) {
        block_type_Origin=block_type;
        if(block_type==GRASS)
            block_type=DIRT;
        else if(block_type==STONE)
            block_type=GRASS;
        else if(block_type==WOOD)
            block_type=STONE;
        else if(block_type==LEAF)
            block_type=WOOD;
        else if(block_type==BEDROCK)
            block_type=LEAF;
        else if(block_type==COAL)
            block_type=BEDROCK;
        else if(block_type==IRON_ORE)
            block_type=COAL;
        else if(block_type==LAVA)
            block_type=IRON_ORE;
        else if(block_type==DIRT)
            block_type=LAVA;
        CurrentItemChange();
    } else if (e->key() == Qt::Key_O) {
        Fetch_Mode=COLLECTMODE;
    } else if (e->key() == Qt::Key_P) {
        Fetch_Mode=PICKMODE;
    } else if (e->key() == Qt::Key_C) {
        if(OpenDNcycle == 0)
            OpenDNcycle = 1;
        else
            OpenDNcycle = 0;
    }
}
void MyGL::mouseMoveEvent(QMouseEvent *event)
{     
    if(!mousemove)
    {
        mousemove=true;
        mouse_oldpos=event->pos();
        return;
    }
    if(mouse_oldpos.x()==width()-1||mouse_oldpos.x()==0\
            ||mouse_oldpos.y()==height()-1||mouse_oldpos.y()==0)
    {
        QCursor::setPos(width()/2,height()/2);
        mouse_oldpos=QPoint(width()/2,height()/2);
        return;
    }
    QPoint p=event->pos()-mouse_oldpos;
    mouse_oldpos=event->pos();
    float delta_w=2*float(p.x())/float(width())*glm::length(gl_camera.H);
    float delta_h=2*float(p.y())/float(height())*glm::length(gl_camera.V);
    float theta=atan(delta_w/glm::length(gl_camera.ref-gl_camera.eye))*180/M_PI;
    float fai=atan(delta_h/glm::length(gl_camera.ref-gl_camera.eye))*180/M_PI;
    gl_camera.RotateAboutUp(-theta);
    gl_camera.RotateAboutRight(-fai);
    gl_camera.RecomputeAttributes();

    gl_skyboxCamera.RotateAboutUp(-theta);
    gl_skyboxCamera.RotateAboutRight(-fai);
    //update();

    //printf("%f %f %d %f\n", gl_camera.ref.x, gl_camera.ref.z, scene.mMinXYZ.z, fabs(gl_camera.ref.z - scene.mMinXYZ.z));
}
void MyGL::keyReleaseEvent(QKeyEvent *e)
{
    if(e->modifiers() & Qt::ShiftModifier){
        keyboard[0]=false;

        // http://doc.qt.io/qt-5/qt.html#Key-enum
        // This could all be much more efficient if a switch
        // statement were used, but I really dislike their
        // syntax so I chose to be lazy and use a long
        // chain of if statements instead
    } else if (e->key() == Qt::Key_Escape) {
        keyboard[1]=false;
    } else if (e->key() == Qt::Key_Right) {
        keyboard[2]=false;
    } else if (e->key() == Qt::Key_Left) {
        keyboard[3]=false;
    } else if (e->key() == Qt::Key_Up) {
        keyboard[4]=false;
    } else if (e->key() == Qt::Key_Down) {
        keyboard[5]=false;
    } else if (e->key() == Qt::Key_Z) {
        keyboard[6]=false;
    } else if (e->key() == Qt::Key_X) {
        keyboard[7]=false;
    } else if (e->key() == Qt::Key_G) {
        keyboard[8]=false;
    } else if (e->key() == Qt::Key_F) {
        keyboard[9]=false;
    } else if (e->key() == Qt::Key_W) {
        keyboard[10]=false;
    } else if (e->key() == Qt::Key_S) {
        keyboard[11]=false;
    } else if (e->key() == Qt::Key_D) {
        keyboard[12]=false;
    } else if (e->key() == Qt::Key_A) {
        keyboard[13]=false;
    } else if (e->key() == Qt::Key_Q) {
        keyboard[14]=false;
    } else if (e->key() == Qt::Key_E) {
        keyboard[15]=false;
    } else if(e->key()==Qt::Key_Space){
        keyboard[16]=false;
    } else if (e->key() == Qt::Key_R) {
        keyboard[17]=false;
    }

}

void MyGL::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
    {
        breakblocks(QPoint(width()/2,height()/2),MaxReachDistance,Fetch_Mode);
        update();
    }
    if(block_type==0)
        return;
    if(event->button()==Qt::RightButton)
    {
        addblocks(QPoint(width()/2,height()/2),MaxReachDistance,block_type);
        update();
    }
}


void MyGL::timerUpdate()
{
    if(!game_begin)
        return;

    fTime += 1 / 60.f;
    if (fTime > 4)
        fTime = 0;
    //printf("%f\n", fTime);
    //20 seconds a Day, 10 second, 1 second = 62.5timecount
    Daytime = (++Daytime)%1250;
    timeCount = (++timeCount) % 150;
    if (timeCount % 150 == 0)
    {
        if (gl_camera.cameramode!=FLYING_MODE)
        {
            if (fabs(gl_camera.eye.x - scene.mMinXYZ.x) < scene.mRefreshDistance)
            {
               // printf("0\n");
                scene.GenerateBlocks(0);
                UpdateWhenNewTerrain(scene.New_map);
            }
            else if (fabs(gl_camera.eye.x - scene.mMaxXYZ.x) < scene.mRefreshDistance)
            {
                //printf("%f %f %f\n", gl_camera.eye.x, scene.mMaxXYZ.x, scene.mRefreshDistance);
                scene.GenerateBlocks(1);
                UpdateWhenNewTerrain(scene.New_map);

            }
            else if (fabs(gl_camera.eye.z - scene.mMinXYZ.z) < scene.mRefreshDistance)
            {
                //printf("2\n");
                scene.GenerateBlocks(2);
                UpdateWhenNewTerrain(scene.New_map);

            }
            else if (fabs(gl_camera.eye.z - scene.mMaxXYZ.z) < scene.mRefreshDistance)
            {
                //printf("3\n");
                scene.GenerateBlocks(3);
                UpdateWhenNewTerrain(scene.New_map);

            }
            //printf("x:%f z:%f\n", gl_camera.eye.x, gl_camera.eye.z);
            //Test whether need to update the superchunk
//            if(gl_camera.eye.x - grid.start_pos[0] > 33 * 16){
//                // +x out of bounds
//        //        std::cout<<"0\n";
//                grid.MoveUpdate(0, scene.mSceneMap);
//            }
//            else if(gl_camera.eye.x - grid.start_pos[0] < 32 * 16){
//                // -x out of bounds
//        //        std::cout<<"1\n";
//                grid.MoveUpdate(1, scene.mSceneMap);
//            }
//            else if(gl_camera.eye.y - grid.start_pos[1] > 33 * 16){
//                // +y out of bounds
//        //        std::cout<<"2\n";
//                grid.MoveUpdate(2, scene.mSceneMap);
//            }
//            else if(gl_camera.eye.y - grid.start_pos[1] < 32 * 16){
//        //        std::cout<<"3\n";
//                // -y out of bounds
//                grid.MoveUpdate(3, scene.mSceneMap);
//            }
//            else if(gl_camera.eye.z - grid.start_pos[2] > 33 * 16){
//                // +z out of bounds
//        //        std::cout<<"4\n";
//                grid.MoveUpdate(4, scene.mSceneMap);

//            }
//            else if(gl_camera.eye.z - grid.start_pos[2] < 32 * 16){
//                // -z out of bounds
//        //        std::cout<<"5\n";
//                grid.MoveUpdate(5, scene.mSceneMap);
//            }
        }
    }
    prog_new.setTime(timeCount);

    Firecount++;
    if(Firecount>20&&Fireblocks.size()>0)
    {
        Firecount=0;
        FireSpread();
        std::vector<Fire>::iterator k = Fireblocks.begin();
        while(k!=Fireblocks.end())
        {

            if(!(*k).FireBurning())
            {
                glm::vec3 fire_pos=(*k).getpos();
                tuple pos(fire_pos[0],fire_pos[1],fire_pos[2]);
                std::map<tuple, Block*>::iterator iter_temp;
                iter_temp=scene.mSceneMap.find(pos);
                if(iter_temp!=scene.mSceneMap.end())
                {
                    scene.mSceneMap.erase(iter_temp);
                    grid.set(fire_pos[0],fire_pos[1],fire_pos[2],0);
                }

                Fireblocks.erase(k);
            }
            else
                k++;
        }
    }

    double distance_water=WaterDistanceClose();
    double distance_fire=FireDistanceClose();
    if(distance_water<20.0f)
    {
        if(!RiverSound.isPlaying())
            RiverSound.play();
    }
    else
    {
        if(distance_water<180.0f)
        {
            double volume=15/distance_water;
            RiverSound.setVolume(volume);
            if(!RiverSound.isPlaying())
                RiverSound.play();
        }
        else
        {
            if(RiverSound.isPlaying())
                RiverSound.stop();
        }
    }
    if(distance_fire>0)
    {
        if(distance_fire<20.0f)
        {
            if(!FireSound.isPlaying())
                FireSound.play();
        }
        else
        {
            if(distance_fire<180.0f)
            {
                double volume=15/distance_fire;
                FireSound.setVolume(volume);
                if(!FireSound.isPlaying())
                    FireSound.play();
            }
            else
            {
                if(FireSound.isPlaying())
                    FireSound.stop();
            }
        }
    }


    if(gl_camera.takeoff)
    {
        if(gl_camera.takeoff_time>1.0f)
        {
            gl_camera.takeoff=false;
            gl_camera.takeoff_time=0;
        }
        else
            gl_camera.TranslateAlongUp(1.0f/20.0f);

        gl_camera.takeoff_time+=1.0f/60.0f;
    }


    Keyevents();
    if(gl_camera.cameramode==FLYING_MODE)
        return;

    if(boundarytest())
    {
        QMessageBox::information(NULL,"Note","Falling out of the boundary!");
        QApplication::quit();
    }
    float distance=g_velocity*(time_step)+0.5*(gravity_acceleration+external_force_acceleration)*time_step*time_step;
    if(distance>1)
        distance=1;
    gl_camera.TranslateAlongWorldUp(distance);
    update();
    if(bottom_test())
    {
        if(g_velocity<=0)
        {
            if(g_velocity<0)
            {
                if(Step_type==WATER)
                    WaterGround.play();
                else
                    Ground.play();
            }
            g_velocity=0;
            external_force_acceleration=-gravity_acceleration;
            if(!keyboard[16])
                jump_state=false;
        }
    }
    else
    {
        g_velocity+=gravity_acceleration*(time_step);
        external_force_acceleration=0;
    }
}


//void MyGL::test(){
////    int width, height;
////    unsigned char* image = SOIL_load_image("F:/QT_project/Final_Project_Minicraft/miniminecraft/minecraft_textures_all/minecraft_textures_all.png",
////                                           &width, &height, 0, SOIL_LOAD_RGB);
////    printf("SOIL results: '%s'\n", SOIL_last_result());
//}

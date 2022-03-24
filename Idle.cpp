#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"

#include "OpenSimplexNoise.hh"

#include "IdleUnitClasses.h"
#include "globals.h"


using namespace std;


    
    
class Idle : public olc::PixelGameEngine
{
public:
    Idle()
    {
        sAppName = "Idle Strategies";
    }
    
    
    float time;
    const float buttonSize = 25;
    enum GAMESTATE {MENU,PLAY,EXIT};
    enum PLAYSTATE {LOADING,MAP,ARENA};
    GAMESTATE gameState = MENU;
    PLAYSTATE playState = MAP;
    OSN::Noise<2> perlin;
    
    olc::TileTransformedView viewer;
    int PlayLayerDraw = 0;
    int UnitLayerDraw = 0;
    
public:
    bool OnUserCreate() override
    {
        // Called once at the start, so create things here
        time = 0;
        int windowSize = min(ScreenHeight()/arenaSize,ScreenWidth()/arenaSize);
        viewer = olc::TileTransformedView({ScreenWidth(),ScreenHeight()},{windowSize,windowSize});
        viewer.SetWorldOffset({-arenaSize*0.5f,0});
        UnitLayerDraw = CreateLayer();
        PlayLayerDraw = CreateLayer();
        EnableLayer(UnitLayerDraw,true);
        EnableLayer(PlayLayerDraw,true);
        //Clear(olc::BLACK);
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        // called once per frame
        
        time = fElapsedTime;
        
        Clear(olc::BLANK);
        switch(gameState){
            case MENU:
                MainMenu();
                break;
    
            case PLAY:
                PlayLoop();
                break;
            
            case EXIT:
                memoryCleanup();
                return false; 
        }
        
        return true;
    }
    
    int MainMenu(){
        string title = "Real Time Idle Wars";
        float buttonWidth = ScreenWidth()*0.3f;
        float buttonHeight = ScreenHeight()*0.1f;
        float buttonX = (ScreenWidth() - buttonWidth)*0.5f;
        float buttonSpacing = ScreenHeight()*0.2f;
        Button(title,0,buttonSpacing,ScreenWidth(),buttonHeight,olc::RED,olc::RED,olc::BLANK,olc::BLANK);
        
        if(Button("Play",buttonX,buttonSpacing*2,buttonWidth,buttonHeight,olc::RED,olc::BLUE, olc::BLACK, olc::DARK_RED) == 1)
            {gameState = PLAY;}
            
        if(Button("Exit",buttonX,buttonSpacing*3,buttonWidth,buttonHeight,olc::RED,olc::BLUE, olc::BLACK, olc::DARK_RED) == 1)
            {gameState = EXIT;}
            
            
        
        return 1;
    }
        
    int PlayLoop(){
        tickGame();
        drawGame();
        drawUI();
        return 1;
        
    }
    
    
    
    
    olc::vf2d mapNumber;
    float threshold = 0.6f;
    float arenaBoard[arenaSize][arenaSize];
    

    
    float scale = 0.05f;
    Capital* westForces;
    Capital* northForces;    
    Capital* eastForces;
    Capital* southForces;
    
    
    //~ Soldier* testSoldier;
    //~ Soldier* testSoldiertwo;
    float speed = 2.0f;
    bool walking = false;
    
    
    void tickGame(){
        
    }
    
    void drawGame(){
        
        
        
        switch(playState){
            
            case MAP:
                map();
                break;
            case LOADING:
                loading();
                break;
            case ARENA:
                arena();
                break;
        }
    }
    
    void map(){
        float windowSize = min(ScreenHeight()/arenaSize,ScreenWidth()/arenaSize);
        viewer.SetWorldScale({windowSize,windowSize});
        viewer.SetWorldOffset({-arenaSize*0.5f,0});
        Clear(olc::BLACK);
        for(int j = 0;j<5;j++){
            for(int i = 0;i<5;i++){
                if(viewerButton("P",i*arenaSize*0.2f,j*arenaSize*0.2f,arenaSize*0.2f,arenaSize*0.2f,olc::RED,olc::YELLOW,olc::BLUE,olc::DARK_BLUE)){
                    mapNumber = {float(i),float(j)};
                    playState = LOADING;
                }
            }   
        }
    }
    
    void loading(){
        olc::vf2d mover = {1,0};
        Clear(olc::BLACK);
        DrawString(10,10,"LOADING",olc::RED,20);
        // Build terrain Map
        terrainBuild();
        // Locate and Place Capital Bases
        initializeBases();
        
        // Check Valid Pathing exists
        while(!aStarExists(northForces->location,southForces->location,westForces->location.x,0,eastForces->location.x,arenaSize))
            threshold+=0.01f;
        while(!aStarExists(westForces->location,eastForces->location,0,northForces->location.y,arenaSize,southForces->location.y))
            threshold+=0.01f;
        
        
        // Generate inital Vector Fields
        buildVectorField(westForces,westDistVec);
        buildVectorField(northForces,northDistVec);
        buildVectorField(eastForces,eastDistVec);
        buildVectorField(southForces,southDistVec);
        
        //testSoldier = new Soldier(localMinima(25,25,75,75) + mover,0.2f,olc::Pixel(250,0,250),&viewer,WEST);
        //testSoldiertwo = new Soldier(localMinima(25,25,75,75),0.2f,olc::Pixel(250,0,250),&viewer,WEST);
        SetDrawTarget(PlayLayerDraw);
            Clear(olc::VERY_DARK_BLUE);
            terrainDraw();
            SetDrawTarget(nullptr);
        playState = ARENA;
    }
    
    void arena(){
        //terrainBuild(); // For debuggin map math
        //checkPanZoom();
        if(checkPanZoom()){
            SetDrawTarget(PlayLayerDraw);
            Clear(olc::VERY_DARK_BLUE);
            terrainDraw();
            SetDrawTarget(nullptr);
            DrawString(0,0, "Panning",olc::WHITE,1.0f);
        }
        
        SetDrawTarget(UnitLayerDraw);
        Clear(olc::BLANK);
        
     auto tpStart = std::chrono::system_clock::now();  
        westForces->update(time);
        northForces->update(time);
        eastForces->update(time);
        southForces->update(time);




        vector<Unit*> allPersonal;
        vector<Unit*> allStructures;
        

        westForces->collectUnits(allStructures,allPersonal);
        northForces->collectUnits(allStructures,allPersonal);
        eastForces->collectUnits(allStructures,allPersonal);
        southForces->collectUnits(allStructures,allPersonal);
        

        for(int i = 0; i< allPersonal.size(); i++){
            for(int j = i+1; j < allPersonal.size(); j++){
                
                allPersonal[i]->checkCollide(allPersonal[j]);
                if (allPersonal[i]->team != allPersonal[j]->team){
                    allPersonal[i]->checkAttack(allPersonal[j]);
                }
                
            }
            for(int k = 0; k < allStructures.size(); k++){
                allPersonal[i]->checkCollide(allStructures[k]);
                if (allPersonal[i]->team != allStructures[k]->team){
                    allPersonal[i]->checkAttack(allStructures[k]);
                }
            }
        }

        westForces->draw();
        northForces->draw();    
        eastForces->draw();
        southForces->draw();
       std::chrono::duration<float> dur = std::chrono::system_clock::now() - tpStart;
DrawString(5,80, "Units Updates:" + to_string(dur.count()),olc::WHITE,1.0f);
            
        //debugging things to remove later
        DrawString(5,5,to_string(threshold),olc::WHITE,1.0f);
        if(GetKey(olc::I).bHeld)
            threshold -= 0.1f*time;
        if(GetKey(olc::K).bHeld)
            threshold += 0.1f*time;
        if(GetKey(olc::O).bHeld)
            scale -= 0.001f*time;
        if(GetKey(olc::L).bHeld)
            scale += 0.001f*time;
        
        //~ if(GetKey(olc::UP).bHeld)
            //~ testSoldier->location.y -= time*speed;
        //~ if(GetKey(olc::DOWN).bHeld)
            //~ testSoldier->location.y += time*speed;
        //~ if(GetKey(olc::LEFT).bHeld)
            //~ testSoldier->location.x -= time*speed;
        //~ if(GetKey(olc::RIGHT).bHeld)
            //~ testSoldier->location.x += time*speed;
            
            
        //~ if(GetKey(olc::UP).bHeld)
            //~ buildVectorField(northForces,westDistVec);
        //~ if(GetKey(olc::DOWN).bHeld)
            //~ buildVectorField(southForces,westDistVec);
        //~ if(GetKey(olc::LEFT).bHeld)
            //~ buildVectorField(westForces,westDistVec);
        //~ if(GetKey(olc::RIGHT).bHeld)
            //~ buildVectorField(eastForces,westDistVec);
        //~ if(GetKey(olc::Z).bPressed)
            //~ walking = !walking;


        //~ testSoldier->checkCollide(testSoldiertwo);
        //~ testSoldiertwo->checkCollide(testSoldier);
        
        //~ testSoldier->checkCollide(northForces);
        //~ testSoldiertwo->checkCollide(northForces);
        
        //~ testSoldier->checkCollide(southForces);
        //~ testSoldiertwo->checkCollide(southForces);
        
        //~ testSoldier->checkCollide(westForces);
        //~ testSoldiertwo->checkCollide(westForces);
        
        //~ testSoldier->checkCollide(eastForces);
        //~ testSoldiertwo->checkCollide(eastForces);
        
        
        //~ testSoldier->draw();
        //~ testSoldiertwo->draw();
        SetDrawTarget(nullptr);
        
        //~ if (walking){
            //~ DrawString(5,70,testSoldier->update(time),olc::WHITE,1.0f);
            //~ testSoldiertwo->update(time);
        //~ } else {
            //~ DrawString(5,70, to_string(testSoldier->location.x) +" " +to_string(testSoldier->location.y),olc::WHITE,1.0f);
            //~ DrawString(5,80, to_string(northForces->location.x) +" " +to_string(northForces->location.y),olc::WHITE,1.0f);
            
            
        //~ }
            
        //~ DrawString(5,15,to_string(scale),olc::WHITE,1.0f);
        //~ olc::vf2d mouseXY = viewer.ScreenToWorld(GetMousePos());
        //~ if (mouseXY.x > 0 && mouseXY.y > 0 && int(round(mouseXY.x)) < arenaSize && int(round(mouseXY.y)) < arenaSize){
            //~ DrawString(5,25,to_string(westDistVec[int(round(mouseXY.x))][int(round(mouseXY.y))]),olc::WHITE,1.0f);
            //~ DrawString(5,35,to_string(northDistVec[int(round(mouseXY.x))][int(round(mouseXY.y))]),olc::WHITE,1.0f);
            //~ DrawString(5,45,to_string(eastDistVec[int(round(mouseXY.x))][int(round(mouseXY.y))]),olc::WHITE,1.0f);
            //~ DrawString(5,55,to_string(southDistVec[int(round(mouseXY.x))][int(round(mouseXY.y))]),olc::WHITE,1.0f);
        //~ }
        
        //end debug
    }
    
    void terrainBuild(){
        for(int j = 0;j < arenaSize; j++){
            for (int i = 0; i < arenaSize; i++){
                float ii = (i-arenaSize*0.5f)*0.04f; float jj = (j-arenaSize*0.5f)*0.04f;
                float depth1 = (((ii*jj*0.7f)*(ii*jj*0.7f))*0.5f+(cos(float(ii*ii*0.6f+jj*jj*0.6f)))*0.3f)+0.5f;
                float depth2 = (perlin.eval(float((mapNumber.x*arenaSize+i)*.15f),float((mapNumber.y*arenaSize+j)*.15f))+1)*0.5f;
                float depth = (depth1 + depth2)*0.5f;
                arenaBoard[i][j] = depth;
            }
        }
        
    }
    
    void cellFill(int i, int j,float dTL, float dTR, float dBR, float dBL){
        olc::Pixel wallColour = olc::WHITE;
        olc::Pixel pathColour = olc::Pixel(51,45,26);
        olc::Pixel hillColour = olc::GREEN;
        int wallType = 0;
        if (dTL > threshold){
            wallType += 8;
        }
        if (dTR > threshold){
            wallType += 4;
        }
        if (dBR > threshold){
            wallType += 2;
        }
        if (dBL > threshold){
            wallType += 1;
        }
        switch(wallType){
            case 15:// All Four Corners = Solid Land
                viewer.FillRect(i,j,1.0f,1.0f,hillColour);
                break;
            case 14://Top Left + Top Right + Bottom Right = Inverted Bottom Left
                viewer.FillRect(i,j,1.0f,1.0f,hillColour);
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},{i+0.0f,j+1.0f},pathColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 13://Top Left + Top Right + Bottom Left = Inverted Bottom Right
                viewer.FillRect(i,j,1.0f,1.0f,hillColour);
                viewer.FillTriangle({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},{i+1.0f,j+1.0f},pathColour);
                viewer.DrawLine({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 12://Top Left + Top Right = Top Half
                viewer.FillRect(i,j,1.0f,0.5f,hillColour);
                viewer.FillRect(i,j+0.5f,1.0f,0.5f,pathColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+1.0f,j+0.5f},wallColour);
                break;
            case 11://Top Left + Bottom Right + Bottom Left = Invert Top Right 
                viewer.FillRect(i,j,01.0f,1.0f,hillColour);
                viewer.FillTriangle({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},{i+1.0f,j+0.0f},pathColour);
                viewer.DrawLine({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},wallColour);
                break;
            case 10://Top Left + Bottom Right = Special Case: Opposite Corners
                viewer.FillRect(i,j,01.0f,1.0f,hillColour);
                viewer.FillTriangle({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},{i+1.0f,j+0.0f},pathColour);
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},{i+0.0f,j+1.0f},pathColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                viewer.DrawLine({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},wallColour);
                break;
            case 9://Top Left + Bottom Left = left half
                viewer.FillRect(i,j,0.5f,1.0f,hillColour);
                viewer.FillRect(i+0.5f,j,0.5f,1.0f,pathColour);
                viewer.DrawLine({i+0.5f,j+0.0f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 8://Top Left
                viewer.FillRect(i,j,01.0f,1.0f,pathColour);
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},{i+0.0f,j+0.0f},hillColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},wallColour);
                break;
            case 7://Top Right + Bottom Right + Bottom Left = Inverte Top Left
                viewer.FillRect(i,j,01.0f,1.0f,hillColour);
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},{i+0.0f,j+0.0f},pathColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},wallColour);
                break;
            case 6://Top Right + Bottom Right = Right Side
                viewer.FillRect(i,j,0.5f,1.0f,pathColour);
                viewer.FillRect(i+0.5f,j,0.5f,1.0f,hillColour);
                viewer.DrawLine({i+0.5f,j+0.0f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 5://Top Right + Bottom Left = Special Case: Opposite Corners
                viewer.FillRect(i,j,01.0f,1.0f,hillColour);
                
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},{i+0.0f,j+0.0f},pathColour);
                viewer.FillTriangle({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},{i+1.0f,j+1.0f},pathColour );
                
                viewer.DrawLine({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+0.0f},wallColour);
                break;
            case 4://Top Right
                viewer.FillRect(i,j,01.0f,1.0f,pathColour);
                viewer.FillTriangle({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},{i+1.0f,j+0.0f},hillColour);
                viewer.DrawLine({i+0.5f,j+0.0f},{i+1.0f,j+0.5f},wallColour);
                break;
            case 3://Bottom Right + Bottom Left = bottom half
                viewer.FillRect(i,j,1.0f,0.5f,pathColour);
                viewer.FillRect(i,j+0.5f,1.0f,0.5f,hillColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+1.0f,j+0.5f},wallColour);
                break;
            case 2://Bottom Right
                viewer.FillRect(i,j,1.0f,1.0f,pathColour);
                viewer.FillTriangle({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},{i+1.0f,j+1.0f},hillColour );
                viewer.DrawLine({i+1.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 1://Bottom Left
                viewer.FillRect(i,j,1.0f,1.0f,pathColour);
                viewer.FillTriangle({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},{i+0.0f,j+1.0f}, hillColour);
                viewer.DrawLine({i+0.0f,j+0.5f},{i+0.5f,j+1.0f},wallColour);
                break;
            case 0://No Walls = Floor Tile
                viewer.FillRect(i,j,1.0f,1.0f,pathColour);//33 2D 1A
                break;
        }
    }
    
    void terrainDraw(){
        auto tpStart = std::chrono::system_clock::now();
        olc::vf2d tl = viewer.GetTopLeftTile();
        olc::vf2d br = viewer.GetBottomRightTile();


        for(int j = max(0,int(tl.y));j < min(int(br.y),arenaSize-1); j++){
        
            for(int i = max(0,int(tl.x));i < min(int(br.x),arenaSize-1);i++){
 
                // depth Top/Left/Right/Bottom
                float dTL = arenaBoard[i  ][j  ];
                float dTR = arenaBoard[i+1][j  ];
                float dBR = arenaBoard[i+1][j+1];
                float dBL = arenaBoard[i  ][j+1];
                
                cellFill(i,j,dTL,dTR,dBR,dBL);
              
            }
        }

        
        std::chrono::duration<float> dur = std::chrono::system_clock::now() - tpStart;
        DrawString(5,90, "GroundDraw:" + to_string(dur.count()),olc::WHITE,1.0f);
        
  
    }
    
    olc::vf2d localMinima(int xS,int yS, int xE, int yE){
        //returns the lowest point in range inclusive of edges - if multiple equal points found get closest to center of area
        olc::vf2d point = {float(xS),float(yS)}; float val = 1.0f;
        olc::vf2d center = {(xS+xE)*0.5f,(yS+yE)*0.5f};
        for (int j = yS;j < yE; j++){
            for (int i = xS;i < xE; i++){
                if(arenaBoard[i][j] < val){
                    point = {float(i),float(j)};
                    val = arenaBoard[i][j];
                } else if (arenaBoard[i][j] == val){
                    olc::vf2d newpoint = {float(i),float(j)};
                    if (center - point < center - newpoint){
                        point = {float(i),float(j)};
                    }
                }
                
            }
        }
        return point;
    }
    
    bool aStarExists(olc::vf2d start,olc::vf2d end,int westLimit,int northLimit,int eastLimit,int southLimit){
        bool pathFound = false;
        list<olc::vf2d> nodes;
        nodes.push_back(start);
        olc::vf2d thisNode;
        olc::vf2d nextNode;
        bool visited[arenaSize][arenaSize] = {0};
        
        while(nodes.size() > 0){
            thisNode = nodes.front();
            nodes.pop_front();
            
            if (thisNode == end)
                return true;
            
            // viewer.FillRect(thisNode.x + 0.25f, thisNode.y + 0.25f,0.5f,0.5f,olc::MAGENTA); //for testing
            
            if(thisNode.x <= max(westLimit,1) || thisNode.x >= min(eastLimit,arenaSize-1))
                continue;
            if(thisNode.y <= max(northLimit,1) || thisNode.y >= min(southLimit,arenaSize-1))
                continue;
                
            
            
            if((!visited[int(thisNode.x+1)][int(thisNode.y)]) && arenaBoard[int(thisNode.x+1)][int(thisNode.y)] < threshold){
                nextNode = {thisNode.x+1,thisNode.y};
                if(max(max(arenaBoard[int(thisNode.x+1)][int(thisNode.y)],arenaBoard[int(thisNode.x-1)][int(thisNode.y)]),
                       max(arenaBoard[int(thisNode.x)][int(thisNode.y+1)],arenaBoard[int(thisNode.x)][int(thisNode.y-1)])) < threshold){
                    visited[int(nextNode.x)][int(nextNode.y)] = true;
                    nodes.push_back(nextNode);
                }
            }
            if((!visited[int(thisNode.x)][int(thisNode.y+1)]) && arenaBoard[int(thisNode.x)][int(thisNode.y+1)] < threshold){
                nextNode = {thisNode.x,thisNode.y+1};
                if(max(max(arenaBoard[int(thisNode.x+1)][int(thisNode.y)],arenaBoard[int(thisNode.x-1)][int(thisNode.y)]),
                       max(arenaBoard[int(thisNode.x)][int(thisNode.y+1)],arenaBoard[int(thisNode.x)][int(thisNode.y-1)])) < threshold){
                    visited[int(nextNode.x)][int(nextNode.y)] = true;
                    nodes.push_back(nextNode);
                }
            }
            if((!visited[int(thisNode.x-1)][int(thisNode.y)]) && arenaBoard[int(thisNode.x-1)][int(thisNode.y)] < threshold){
                nextNode = {thisNode.x-1,thisNode.y};
                if(max(max(arenaBoard[int(thisNode.x+1)][int(thisNode.y)],arenaBoard[int(thisNode.x-1)][int(thisNode.y)]),
                       max(arenaBoard[int(thisNode.x)][int(thisNode.y+1)],arenaBoard[int(thisNode.x)][int(thisNode.y-1)])) < threshold){
                    visited[int(nextNode.x)][int(nextNode.y)] = true;
                    nodes.push_back(nextNode);
                }
            }
            if((!visited[int(thisNode.x)][int(thisNode.y-1)]) && arenaBoard[int(thisNode.x)][int(thisNode.y-1)] < threshold){
                nextNode = {thisNode.x,thisNode.y-1};
                if(max(max(arenaBoard[int(thisNode.x+1)][int(thisNode.y)],arenaBoard[int(thisNode.x-1)][int(thisNode.y)]),
                       max(arenaBoard[int(thisNode.x)][int(thisNode.y+1)],arenaBoard[int(thisNode.x)][int(thisNode.y-1)])) < threshold){
                    visited[int(nextNode.x)][int(nextNode.y)] = true;
                    nodes.push_back(nextNode);
                }
            }
        }
        return pathFound;
    }
    
    void initializeBases(){
        olc::vf2d mainBaseSize = {1.5f,1.5f};
        olc::vf2d northBaseLoc = localMinima(floor(arenaSize * 0.3f),floor(arenaSize * 0.05f),ceil(arenaSize * 0.7f),floor(arenaSize * 0.2f));
        northForces = new Capital(northBaseLoc,mainBaseSize,NORTH,&viewer,olc::RED);
        olc::vf2d southBaseLoc = localMinima(floor(arenaSize * 0.3f),floor(arenaSize * 0.8f),ceil(arenaSize * 0.7f),floor(arenaSize * 0.95f));
        southForces = new Capital(southBaseLoc,mainBaseSize,SOUTH,&viewer,olc::DARK_MAGENTA);          
        olc::vf2d westBaseLoc = localMinima(floor(arenaSize * 0.05f),floor(arenaSize * 0.3f),ceil(arenaSize * 0.2f),floor(arenaSize * 0.7f));
        westForces = new Capital(westBaseLoc,mainBaseSize,WEST,&viewer,olc::BLUE);
        olc::vf2d eastBaseLoc = localMinima(floor(arenaSize * 0.8f),floor(arenaSize * 0.3f),ceil(arenaSize * 0.95f),floor(arenaSize * 0.7f));
        eastForces = new Capital(eastBaseLoc,mainBaseSize,EAST,&viewer,olc::YELLOW);
    }
    
    //class NodeBinTree{  //More efficient container for vector fields later
        //olc::vf2d node;
        //float dist;
        //int mindepth;
        
        //NodeBinTree* left;
        //NodeBinTree* Right;
        
        
    //};
    
    void buildVectorField(Capital* capital,float vecField[arenaSize][arenaSize]){
        float MaxNodeValue = arenaSize*arenaSize*1.5f;
        for (int j = 0;j<arenaSize;j++){
            for (int i = 0;i<arenaSize;i++){
                vecField[i][j] = float(MaxNodeValue);                
            }
        }
        
        list<olc::vf2d> nodes;
        olc::vf2d thisNode;
        float pendingValue;
        
        olc::vf2d nextNode = capital->location;
        vecField[int(nextNode.x)][int(nextNode.y)] = 0;
        nodes.push_back(nextNode);
        
        for(int b = 0; b < capital->buildings.size();b++){
            nextNode = capital->buildings[b]->location;
            vecField[int(nextNode.x)][int(nextNode.y)] = 1;
            nodes.push_back(nextNode);
        }
        while(nodes.size() > 0){
            thisNode = nodes.front();
            nodes.pop_front();
            if(thisNode.x <= 0 || thisNode.x >= arenaSize)
                continue;
            if(thisNode.y <= 0 || thisNode.y >= arenaSize)
                continue;
                
            pendingValue = vecField[int(thisNode.x)][int(thisNode.y)];
            if(vecField[int(thisNode.x+1)][int(thisNode.y)] > pendingValue + 1 && arenaBoard[int(thisNode.x+1)][int(thisNode.y)] < threshold){
                nextNode = {thisNode.x+1,thisNode.y};
                vecField[int(nextNode.x)][int(nextNode.y)] = pendingValue + 1;
                nodes.push_back(nextNode);
            }
            if(vecField[int(thisNode.x)][int(thisNode.y+1)] > pendingValue + 1 && arenaBoard[int(thisNode.x)][int(thisNode.y+1)] < threshold){
                nextNode = {thisNode.x,thisNode.y+1};
                vecField[int(nextNode.x)][int(nextNode.y)] = pendingValue + 1;
                nodes.push_back(nextNode);
            }
            if(vecField[int(thisNode.x-1)][int(thisNode.y)] > pendingValue + 1 && arenaBoard[int(thisNode.x-1)][int(thisNode.y)] < threshold){
                nextNode = {thisNode.x-1,thisNode.y};
                vecField[int(nextNode.x)][int(nextNode.y)] = pendingValue + 1;
                nodes.push_back(nextNode);
            }
            if(vecField[int(thisNode.x)][int(thisNode.y-1)] > pendingValue + 1 && arenaBoard[int(thisNode.x)][int(thisNode.y-1)] < threshold){
                nextNode = {thisNode.x,thisNode.y-1};
                vecField[int(nextNode.x)][int(nextNode.y)] = pendingValue + 1;
                nodes.push_back(nextNode);
            }
        }
    }
    
    
    
    void drawUI(){
        if(playState == ARENA){
            if(Button("|",ScreenWidth()-buttonSize*2,ScreenHeight()-buttonSize,buttonSize,buttonSize,olc::WHITE,olc::GREY,olc::GREY,olc::BLACK)){
                playState = MAP;
            }
        }
        if(Button("=",ScreenWidth()-buttonSize,ScreenHeight()-buttonSize,buttonSize,buttonSize,olc::WHITE,olc::GREY,olc::GREY,olc::BLACK)){
            gameState = MENU;
        }
        return;
    }
    
    bool checkPanZoom(){
        bool updated = false;
        olc::vf2d curZoom = viewer.GetWorldScale();
        olc::vf2d maxZoom = {10.0f,10.0f};olc::vf2d minZoom = {80.0f,80.0f};
        
        if (GetMouse(2).bPressed){ 
            viewer.StartPan(GetMousePos());
            updated = true;
        }
        if (GetMouse(2).bHeld){
            viewer.UpdatePan(GetMousePos());
            updated = true;
        }
        if (GetMouse(2).bReleased){
            viewer.EndPan(GetMousePos());
            updated = true;
        }
        if (GetMouseWheel() > 0 && curZoom < minZoom){
            viewer.ZoomAtScreenPos(2.0f, GetMousePos());
            updated = true;
        }
        if (GetMouseWheel() < 0 && curZoom > maxZoom){
            viewer.ZoomAtScreenPos(0.5f, GetMousePos());
            updated = true;
        }
        
        if(curZoom < maxZoom)
            viewer.SetWorldScale(maxZoom);
        if(curZoom > minZoom)
            viewer.SetWorldScale(minZoom);
        olc::vf2d movement = {0,0};
        olc::vf2d screenCenter = (viewer.GetWorldBR() + viewer.GetWorldTL()) *0.5f;
        if (screenCenter.x < 0.0f)
            movement.x -= screenCenter.x;
        if (screenCenter.y < 0.0f)
            movement.y -= screenCenter.y;
        if (screenCenter.x > arenaSize)
            movement.x -= (screenCenter.x - arenaSize);
        if (screenCenter.y > arenaSize)
            movement.y -= (screenCenter.y - arenaSize);
            
        viewer.MoveWorldOffset(movement);
        return updated;
    }
    
    void memoryCleanup(){
        //clean up memory
        
        return;
    }
    
    int Button(string text, int x,int y, int w, int h, olc::Pixel textC, olc::Pixel textCMouse, olc::Pixel bgColour, olc::Pixel bgColourMouse)
    {
        bool name = false;
        if (text.length() > 0) //if String empty then skip
            name = true;
        
        int click = 0;
  
        float mX = GetMouseX(); float mY = GetMouseY();
        if (not(mX <= x || mX >= x+w || mY <= y || mY >= y+h)){
            bgColour = bgColourMouse;
            textC = textCMouse;
            
            if(GetMouse(0).bPressed){
                click = 1;
            } else if(GetMouse(1).bPressed){
                click = 2;
            }
        }
        
        FillRect(x,y,w,h,bgColour);
        if (name){
        
            int tScale = floor(min(w / (text.length() * 8.0f),h/8.0f));
            float charSize = 8*tScale;
            float dX = (w - charSize * text.length()) * 0.5f + x;
            float dY = (h - charSize) * 0.5f + y + (charSize * 0.0625f);
            DrawString(dX,dY,text,textC,tScale);
        }
        return click;
    }
    int viewerButton(string text, int x,int y, int w, int h, olc::Pixel textC, olc::Pixel textCMouse, olc::Pixel bgColour, olc::Pixel bgColourMouse)
    {
        if (text.length() == 0) //if String empty then Crash: error 100 (Intentional : To Fix Later with throws)
            exit(100);
        
        int click = 0;
        float tScale = floor(min(w / (text.length() * 8.0f),h/8.0f));
        float charSize = 8*tScale;
        float dX = (w - charSize * text.length()) * 0.5f + x;
        float dY = (h - charSize) * 0.5f + y + (charSize * 0.0625f);
        
        olc::vf2d mouseXY = GetMousePos();
        mouseXY = viewer.ScreenToWorld(GetMousePos());
        float mX = mouseXY.x; float mY = mouseXY.y;
        if (mX > x && mX < x+w && mY > y && mY < y+h){
            bgColour = bgColourMouse;
            textC = textCMouse;
            
            if(GetMouse(0).bPressed){
                click = 1;
            } else if(GetMouse(1).bPressed){
                click = 2;
            }
        }
        
        viewer.FillRect(x,y,w,h,bgColour);
        viewer.DrawString(dX,dY,text,textC,{float(tScale),float(tScale)});
        
        return click;
    }
};


int main()
{
    Idle game;
    if (game.Construct(1920, 1024, 1, 1, false, true))
        game.Start();

    return 0;
}

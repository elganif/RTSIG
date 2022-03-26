#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"
#include "globals.h"

using namespace std;

struct StatBlock
{
    float maxHP = 100;
    float curHP = 100;
    
    float attRange = 1;
    float sightRange = 5;
    float damage = 20.0f;
    
    float attackSpeed = 2.0f;
    float lastAttack = 2.0f;
    
};

class Unit{
    protected:
        olc::TileTransformedView* transview;
    
        enum Shape {SQUARE,CIRCLE};
        olc::vf2d location;
        
        olc::Pixel TeamColour;
        olc::vf2d area;
        float size;
        Shape shape;
        vector<Unit*> soldiers;
        Team team;
        bool alive = true;
        
        StatBlock stats;
        
    public:
        virtual void draw(){};
        virtual string update(float t){return "";};
        virtual void collectUnits(vector<Unit*> &allStructure,vector<Unit*> &allPeronal){};
        virtual olc::vf2d checkCollide(Unit* other){return {0,0};};
        virtual float checkAttack(Unit* other){return 0.0f;};
        virtual Shape getShape(){return shape;}
        virtual olc::vf2d getLocation(){return location;}
        virtual olc::vf2d getArea(){return area;}
        virtual float getSize(){return size;}
        virtual float getTeam(){return team;}
        
};

class Soldier : public Unit{
    private:
        Team target;
        
    public:

        Soldier(olc::vf2d newlocation,float newSize,Team myTeam,olc::Pixel newTeamColour,
            olc::TileTransformedView* newtransview,Team goal)
        
        {
            this->location = newlocation;
            location.x = min(max(location.x,0.0f),float(arenaSize));
            location.y = min(max(location.y,0.0f),float(arenaSize));
            size = newSize;
            transview = newtransview;
            TeamColour = newTeamColour;
            target = goal;
            team = myTeam;
            shape = CIRCLE;
            
        }
        
        void draw() override{
            transview->FillCircle(location,size,TeamColour);
        }
        
        string update(float t) override{
            // move & attack according to AI
            
            
            int X = min(max(int(round(location.x)),1),arenaSize-1);
            int Y = min(max(int(round(location.y)),1),arenaSize-1);
            float here = 0;
            float n = 0;
            float s = 0;
            float w = 0;
            float e = 0;
            switch (target){
                case WEST:
                     here = westDistVec[X][Y];
                     n = westDistVec[X-1][Y];
                     s = westDistVec[X+1][Y];
                     w = westDistVec[X ][Y-1];
                     e = westDistVec[X][Y+1];
                    break;
                case NORTH:
                     here = northDistVec[X][Y];
                     n = northDistVec[X-1][Y];
                     s = northDistVec[X+1][Y];
                     w = northDistVec[X ][Y-1];
                     e = northDistVec[X][Y+1];
                    break;
                case EAST:
                     here = eastDistVec[X][Y];
                     n = eastDistVec[X-1][Y];
                     s = eastDistVec[X+1][Y];
                     w = eastDistVec[X ][Y-1];
                     e = eastDistVec[X][Y+1];
                    break;
                case SOUTH:
                     here = southDistVec[X][Y];
                     n = southDistVec[X-1][Y];
                     s = southDistVec[X+1][Y];
                     w = southDistVec[X ][Y-1];
                     e = southDistVec[X][Y+1];
                    break;
            }
            
            
            olc::vf2d direction = {0,0};
            direction -= {min(max(here - n ,-1.0f),1.0f),0};
            direction += {min(max(here - s ,-1.0f),1.0f),0};
            direction -= {0,min(max(here - w ,-1.0f),1.0f)};
            direction += {0,min(max(here - e ,-1.0f),1.0f)};
            
            if(direction.x == 0 && direction.y == 0) // Stuck on flat vector need backup plan?
                return "stuck";
            direction = direction.norm()*t;
            location += direction;
            location.x = min(max(location.x,0.0f),float(arenaSize));
            location.y = min(max(location.y,0.0f),float(arenaSize));
            
            return "";
        }
        
        olc::vf2d checkCollide(Unit* other) override{
            if(other->getShape() == SQUARE){
                //building collision check
                olc::vf2d center = other->getLocation();
                olc::vf2d topleft = other->getLocation() - (other->getArea() * 0.5f);
                olc::vf2d botright = other->getLocation() + (other->getArea() * 0.5f);
                
                olc::vf2d nearest = {min(max(location.x,topleft.x),botright.x),
                                     min(max(location.y,topleft.y),botright.y)};
                
                olc::vf2d dist = nearest - this->location;
                float colide = size - dist.mag();
                if (isnan(colide)){// colide = 0;
                    colide = 0;
                }
                olc::vf2d normal = dist.norm();
                if (isnan(normal.x) || isnan(normal.y))
                    normal={0,0};
                if (colide > 0)
                    location -= normal * colide;
                
                return {0,0};
            }
            if(other->getShape() == CIRCLE){
                // Unit collision Checking
                float colDist = other->getSize() + size;
                olc::vf2d otherLoc = other->getLocation();
                float xDist = abs(otherLoc.x - location.x);
                float yDist = abs(otherLoc.y - location.y);
                
                if (colDist * colDist < xDist * xDist + yDist * yDist)
                    return {0,0};
                olc::vf2d direction = location - other->getLocation();
                float overlap = colDist - direction.mag();
                location += direction.norm() * overlap * 0.5f;
                
            location.x = min(max(location.x,0.0f),float(arenaSize));
            location.y = min(max(location.y,0.0f),float(arenaSize));
            
                
                return direction.norm() * overlap;
                
                
            }
            return {0,0};
        } 
};

class Building : public Unit{
    public:
        Building(olc::vf2d newlocation,olc::vf2d newArea,Team myTeam,
                 olc::TileTransformedView* newtransview,olc::Pixel newTeamColour){
            location = newlocation;
            location.x = min(max(location.x,0.0f),float(arenaSize));
            location.y = min(max(location.y,0.0f),float(arenaSize));
            
            area = newArea;
            size = max(area.x,area.y);
            transview = newtransview;
            TeamColour = newTeamColour;
            team = myTeam;
            shape = SQUARE;
        }
        void draw() override{
            transview->FillRect(location-(area*0.5f),area,TeamColour);
            for(int u = 0; u < soldiers.size();u++){
                soldiers[u]->draw();
            }
        }
        
        string update(float t) override{
            // spawn and manage units, call update() on units
            return "";
        }
        
        void collectUnits(vector<Unit*> &allStructure,vector<Unit*> &allPeronal) override{
            allStructure.push_back(this);
            for(int i = 0; i < soldiers.size(); i++){
                allPeronal.push_back(soldiers[i]);
            }
            
        };
        //~ olc::vf2d checkCollide(Unit* other) override{
            //~ return {0,0};
        //~ } 
};

class Capital : public Unit{

    protected:
        vector<Unit*> buildings;
    public:
        Capital(olc::vf2d newlocation,olc::vf2d newArea,Team myTeam,
                olc::TileTransformedView* newtransview,olc::Pixel newTeamColour){
            location = newlocation;
            location.x = min(max(location.x,0.0f),float(arenaSize));
            location.y = min(max(location.y,0.0f),float(arenaSize));
            
            area = newArea;
            size = max(area.x,area.y);
            transview = newtransview;
            TeamColour = newTeamColour;
            team = myTeam;
            shape = SQUARE;
        }
        
        void draw() override{
            transview->FillRect(location-(area*0.5f),area,TeamColour);
            for(int u = 0; u < buildings.size();u++){
                buildings[u]->draw();
            }
            for(int u = 0; u < soldiers.size();u++){
                soldiers[u]->draw();
            }
        }
        
        void ConstructBuilding(olc::vf2d loc){
            Building* b = new Building(loc,{1,1},team,transview,TeamColour);
            buildings.push_back(b);
        }
        
        string update(float t) override{ 
            // spawn and manage units or buildings, call update() on units and buildings
            int target = rand()%4 + 1;
            Team targetT;
            if (target == 1)
                targetT = WEST;
            if (target == 2)
                targetT = NORTH;
            if (target == 3)
                targetT = EAST;
            if (target == 4)
                targetT = SOUTH;

            // Spawn mechanics
            if (soldiers.size() < 100){
                Soldier* nextUnit = new Soldier(this->location,0.2f,team,TeamColour,transview,targetT);
                soldiers.push_back(nextUnit);
            }
            // update all units
            for (int i = 0; i < soldiers.size(); i++){
                soldiers[i]->update(t);
            }
            return"";
        }
        
        void collectUnits(vector<Unit*> &allStructure,vector<Unit*> &allPeronal) override{
            allStructure.push_back(this);
            for(int i = 0; i < soldiers.size(); i++){
                allPeronal.push_back(soldiers[i]);
            }
            for(int i = 0; i < buildings.size(); i++){
                buildings[i]->collectUnits(allStructure,allPeronal);
            }
            
        };
         
        std::list<olc::vf2d> buildingLocations(){
            std::list<olc::vf2d> locations;
            for(int b = 0; b < buildings.size();b++){
                locations.push_back(buildings[b]->getLocation());
            }
            return locations;
            
        }
        //~ olc::vf2d checkCollide(Unit* other) override{
            //~ return {0,0};
        //~ } 
};

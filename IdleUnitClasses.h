#define OLC_PGEX_TRANSFORMEDVIEW
#include "olcPGEX_TransformedView.h"
#include "globals.h"

using namespace std;
class Unit{
    public:
        enum Shape {SQUARE,CIRCLE};
        olc::vf2d location;
        olc::TileTransformedView* transview;
        olc::Pixel TeamColour;
        olc::vf2d area;
        float size;
        Shape shape;
        vector<Unit*> soldiers;
        
        virtual void draw(){};
        virtual string update(float t){return "";};
        virtual olc::vf2d checkCollide(Unit* other){return {0,0};};
        
    
};

class Soldier : public Unit{
    public:
    
        Team target;
        float (* vectorTable);
        
        Soldier(olc::vf2d newlocation,float newSize,olc::Pixel TeamColour,
            olc::TileTransformedView* newtransview,Team team)
        
        {
            location = newlocation;
            size = newSize;
            transview = newtransview;
            this->TeamColour = TeamColour;
            target = team;
            shape = CIRCLE;
            
        }
        
        void draw() override{
            transview->FillCircle(location,size,TeamColour);
        }
        string update(float t) override{
            // move & attack according to AI
            //float (* vectorTable)[arenaSize][arenaSize] = &northDistVec;
            
            int X = int(round(location.x));
            int Y = int(round(location.y));
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
            
            if(direction.x == 0 && direction.y == 0)
                return "oops"+ to_string(location.x) +" " +to_string(location.y) +" " + to_string(n)+" " +to_string(s)+" " +to_string(w)+" " +to_string(e);
            direction = direction.norm()*t;
            location += direction;
            return to_string(location.x) +" " +to_string(location.y) +" " + to_string(n)+" " +to_string(s)+" " +to_string(w)+" " +to_string(e);
        }
        
        olc::vf2d checkCollide(Unit* other) override{
            if(other->shape == SQUARE){
                //building collision check
                olc::vf2d center = other->location;
                olc::vf2d topleft = other->location - (other->area * 0.5f);
                olc::vf2d botright = other->location + (other->area * 0.5f);
                
                olc::vf2d nearest = {min(max(this->location.x,topleft.x),botright.x),
                                     min(max(this->location.y,topleft.y),botright.y)};
                
                olc::vf2d dist = nearest - this->location;
                float colide = this->size - dist.mag();
                if (isnan(colide)) colide = 0;
                
                if (colide > 0)
                    this->location -= dist.norm() * colide;
                
                return {0,0};
            }
            if(other->shape == CIRCLE){
                // Unit collision Checking
                float colDist = other->size + this->size;
                float xDist = abs(other->location.x - this->location.x);
                float yDist = abs(other->location.y - this->location.y);
                
                if (colDist * colDist < xDist * xDist + yDist * yDist)
                    return {0,0};
                olc::vf2d direction = this->location - other->location;
                float overlap = colDist - direction.mag();
                this->location += direction.norm() * overlap * 0.5f;
                other->location -= direction.norm() * overlap * 0.5f;
                
                return direction.norm() * overlap;
                
                
            }
            return {0,0};
        } 
};

class Building : public Unit{
    public:
        Building(olc::vf2d newlocation,olc::vf2d newArea,olc::TileTransformedView* newtransview,olc::Pixel TeamColour){
            location = newlocation;
            area = newArea;
            size = max(area.x,area.y);
            transview = newtransview;
            this->TeamColour = TeamColour;
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
        
        olc::vf2d checkCollide(Unit* other) override{
            return {0,0};
        } 
};

class Capital : public Unit{
    public:
        
        vector<Unit*> buildings;
        
        Capital(olc::vf2d newlocation,olc::vf2d newArea,olc::TileTransformedView* newtransview,olc::Pixel TeamColour){
            location = newlocation;
            area = newArea;
            size = max(area.x,area.y);
            transview = newtransview;
            this->TeamColour = TeamColour;
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
            Building* b = new Building(loc,{1,1},transview,TeamColour);
            buildings.push_back(b);
        }
        
        string update(float t) override{ return"";
            // spawn and manage units or buildings, call update() on units and buildings
        }
        
        olc::vf2d checkCollide(Unit* other) override{
            return {0,0};
        } 
};
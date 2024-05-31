#include <iostream>
#include <string.h>
#include <vector>
#include <unordered_map>
#include "stdio.h"

int main(){
    std::unordered_map<string, float> PLayers = {
        {"Vitaliy",0.352},
        {"Ilgiz",0,725},
        {"Nikita Dallas Lock",0,407},
        {"Damir",1,005},
        {"Marsel",0,88},
        {"Marat",0,66},
        {"Usatiy",0,69},
        {"Jacob", 0,68}
    };
    pairPLayers(PLayers);
}

void pairPLayers(const std::unordered_map <string,float&> playerweights){
    std::vector<std::pair<string, float>> players;
    for(const auto& player:playerWeights){
    player.emplace_back(player);
    }

    sort(player.begin(), players.end(), 
            [](const pair<string, float>& a, const pair<string, float>& b) {
            return a.second<b.second;
           });

    std::cout<<"Team 1:"<<endl;
    for(size_t i = 0; i<players.size()/2;++i){
    std::cout<<"Player"<<players[i].first<<"with weight"<<players[i].second<<endl;
        std::cout<<"Player"<<players[players.size()-1-i].first
            <<"with weight"<<players[players.size()-1-i].second<<endl;
    }

}

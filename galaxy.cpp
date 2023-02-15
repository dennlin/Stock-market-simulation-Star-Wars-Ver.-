//AD48FB4835AF347EB0CA8009E24C3B13F8519882

#include <vector>
#include <sstream>
#include <iostream>
#include <string>
#include <fstream>
#include <getopt.h>
#include <ctype.h>
#include <queue>
#include <deque>
#include <algorithm>
#include "P2random.h"

using namespace std;

class battalion{

    public:

    uint32_t arrival = 0;
    uint32_t generalID = 0;
    uint32_t force = 0;
    mutable uint32_t quantity = 0;
};

class general{

    public:

    uint32_t genID = 0;
    uint32_t totalSith = 0;
    uint32_t totalJedi = 0;
    uint32_t troopsAlive = 0;
};

struct SortbySith{

    bool operator()(const battalion &battalion1, const battalion &battalion2){

        if(battalion1.force < battalion2.force){

            return true;
        }

        if(battalion1.force > battalion2.force){

            return false;
        }

        if(battalion1.arrival < battalion2.arrival){

            return false;        
        }

        if(battalion1.arrival > battalion2.arrival){

            return true;
        }

        return false;
    }
};

struct SortbyJedi{

    bool operator()(const battalion battalion1, const battalion battalion2){

        if(battalion1.force < battalion2.force){

            return false;
        }

        if(battalion1.force > battalion2.force){

            return true;
        }

        if(battalion1.arrival < battalion2.arrival){

            return false;        
        }

        if(battalion1.arrival > battalion2.arrival){

            return true;
        }

        return false;
    }
};

enum class State{Inital, SeenOne, SeenBoth};

class watcher{

    public:

    int currentJediTS = 0;
    int currentSithTS = 0;
    int betterTS = 0;
    uint32_t currentJediForce = 0;
    uint32_t currentSithForce = 0;
    uint32_t betterForce = 0;
    uint32_t difference = 0;
    bool betterSet = false;

    State state = State::Inital;

    void bestAttack(string &deploymentType, uint32_t &force, int &TS){

        if(state == State::Inital){

            if(deploymentType == "JEDI"){

                currentJediForce = force;
                currentJediTS = TS;

                state = State::SeenOne;
            }
        }

        else if(state == State::SeenOne){

            if(deploymentType == "JEDI"){

                if(currentJediForce > force){

                    currentJediForce = force;
                    currentJediTS = TS;
                }
            }

            else{

                if(currentJediForce <= force){

                    currentSithForce = force;
                    currentSithTS = TS;
                    difference = currentSithForce - currentJediForce;

                    state = State::SeenBoth;
                }
            }
        }

        else if(state == State::SeenBoth){

            if(deploymentType == "JEDI"){

                if(force < currentJediForce && betterSet == false){

                    betterForce = force;
                    betterTS = TS;
                    betterSet = true;
                }

                else if(force < betterForce){

                    betterForce = force;
                    betterTS = TS;
                }
            }

            else{

                if(betterSet == false){

                    betterForce = currentJediForce;
                    betterTS = currentJediTS;
                }

                if(force >= betterForce && (force - betterForce) > difference){

                    currentSithForce = force;
                    currentSithTS = TS;
                    currentJediForce = betterForce;
                    currentJediTS = betterTS;
                    difference = currentSithForce - currentJediForce;
                }
            }
        }
        
    }

    void bestAmbush(string &deploymentType, uint32_t &force, int &TS){

        if(state == State::Inital){

            if(deploymentType == "SITH"){

                currentSithForce = force;
                currentSithTS = TS;
                state = State::SeenOne;
            }
        }

        else if(state == State::SeenOne){

            if(deploymentType == "SITH"){

                if(currentSithForce < force){

                    currentSithForce = force;
                    currentSithTS = TS;
                }
            }

            else{

                if(currentSithForce >= force){

                    currentJediForce = force;
                    currentJediTS = TS;
                    difference = currentSithForce - currentJediForce;
                    state = State::SeenBoth;
                }
            }
        }

        else if(state == State::SeenBoth){

            if(deploymentType == "SITH"){

                if(currentSithForce < force && betterSet == false){

                    betterForce = force;
                    betterTS = TS;
                    betterSet = true;
                }

                else if(force > betterForce){

                    betterForce = force;
                    betterTS = TS;
                }                
            }

            else{

                if(betterSet == false){

                    betterForce = currentSithForce;
                    betterTS = currentSithTS;
                }

                if(betterForce >= force && (betterForce - force) > difference){

                    currentJediForce = force;
                    currentSithForce = betterForce;
                    currentJediTS = TS;
                    currentSithTS = betterTS;
                    difference = currentSithForce - currentJediForce;
                }
            }
        }
    }
};


void verbose(uint32_t SithGeneral, uint32_t JediGeneral, uint32_t planet, uint32_t troopsLost){

    cout << "General " << SithGeneral << "'s battalion attacked General " << JediGeneral << "'s battalion on planet " << planet << ". " << troopsLost << " troops were lost.\n";
    // cout << "General " << general1 << "'s battalion attacked General " << general2 << "'s battalion on planet " << planet << ". " << troopsLost << " troops were lost.\n";
}

void printMedian(const uint32_t &median, uint32_t &planet, int &TS){

    cout << "Median troops lost on planet " << planet << " at time " << TS << " is " << median << ".\n";
}


class planet{

    public:

    uint32_t planetID = 0;
    uint32_t totalBattles = 0;
    static vector<general> &generals;
    priority_queue<battalion, vector<battalion>, SortbySith> SithPQ;
    priority_queue<battalion, vector<battalion>, SortbyJedi> JediPQ;
    priority_queue<uint32_t, vector<uint32_t>, less<uint32_t>> Lower;
    priority_queue<uint32_t, vector<uint32_t>, greater<uint32_t>> Upper; 

    watcher ambush;
    watcher attack;

    void assignSithDeployment(battalion deployment){
        
        SithPQ.push(deployment);
    }

    void assignJediDeployment(battalion deployment){

        JediPQ.push(deployment);
    }

    void battle(bool &vmode, bool &mmode){

        uint32_t lost = 0;
        uint32_t SGID = 0;
        uint32_t JGID = 0;

        while(SithPQ.size() != 0 && JediPQ.size() != 0 && SithPQ.top().force >= JediPQ.top().force){

            SGID = SithPQ.top().generalID;
            JGID = JediPQ.top().generalID;

            if(SithPQ.top().quantity > JediPQ.top().quantity){

                SithPQ.top().quantity -= JediPQ.top().quantity;
                lost = 2*JediPQ.top().quantity;
                JediPQ.pop();
            }

            else if(SithPQ.top().quantity < JediPQ.top().quantity){

                JediPQ.top().quantity -= SithPQ.top().quantity;
                lost = 2*SithPQ.top().quantity;
                SithPQ.pop();
            }

            else{

                lost = SithPQ.top().quantity + JediPQ.top().quantity;
                SithPQ.pop();
                JediPQ.pop();
            }

            if(vmode == true){

                verbose(SGID, JGID, planetID, lost);
            }

            if(mmode == true){

                if(Lower.empty() && Upper.empty()){

                    Lower.push(lost);
                }

                else if(lost > Lower.top()){

                    Upper.push(lost);

                    if(Upper.size()-1 > Lower.size()){

                        Lower.push(Upper.top());
                        Upper.pop();
                    }
                }

                else{

                    Lower.push(lost);

                    if(Lower.size()-1 > Upper.size()){

                        Upper.push(Lower.top());
                        Lower.pop();
                    }
                }
            }
                
            totalBattles++;
        }
    }

    void calcMedian(int &TS){

        if(totalBattles > 0){

            if(Lower.size() > Upper.size()){

                printMedian(Lower.top(), planetID, TS);
            }

            else if(Lower.size() < Upper.size()){

                printMedian(Upper.top(), planetID, TS);
            }

            else{

                printMedian((Upper.top()+Lower.top())/2, planetID, TS);
            }
        }
    }
};


class galaxy{

    public:

    uint32_t currentID = 0;
    string junk = "";
    char letterjunk = '\0';
    string inputMode = "";
    uint32_t num_generals = 0;
    uint32_t num_planets = 0;
    uint32_t seed = 0;
    uint32_t num_deployments = 0;
    uint32_t arrival_rate = 0;
    int timestamp = 0;
    int currentTime = 0;
    int previousTS = 0;
    uint32_t planetNum = 0;
    uint32_t sumBattles = 0;
    bool verboseMode = false;
    bool medianMode = false;
    bool generalEval = false;
    bool watcherMode = false;
    stringstream ss;

    battalion deployment;
    vector<general> generalList;
    vector<planet> planetList;


    void readStartInfo(){


        getline(cin, junk);
        cin >> junk >> inputMode;
        cin >> junk >> num_generals;
        cin >> junk >> num_planets;

        planetList.resize(num_planets);
        generalList.resize(num_generals);

        if(inputMode == "PR"){

            cin >> junk >> seed;
            cin >> junk >> num_deployments;
            cin >> junk >> arrival_rate;
        }
    }

    void assign(){

        deployment.arrival = currentID;
        currentID++;

        if(junk == "SITH"){

            planetList[planetNum].assignSithDeployment(deployment);

            if(generalEval == true){

                generalList[deployment.generalID].totalSith += deployment.quantity;
            }
        }

        else if(junk == "JEDI"){

            planetList[planetNum].assignJediDeployment(deployment);

            if(generalEval == true){

                generalList[deployment.generalID].totalJedi += deployment.quantity;
            }
        }

        if(currentTime != timestamp){

            if(medianMode == true){

                for(uint32_t i = 0; i < num_planets; i++){

                    planetList[i].calcMedian(currentTime);
                }
            
                currentTime = timestamp;
            }
        }
    }

    void battleSim(){

        planetList[planetNum].battle(verboseMode, medianMode);
    }

    void calcBattles(){

        for(uint32_t i = 0; i < planetList.size(); i++){

            sumBattles += planetList[i].totalBattles;    
        }
    }

    void watch(){

        planetList[planetNum].ambush.bestAmbush(junk, deployment.force, timestamp);
        planetList[planetNum].attack.bestAttack(junk, deployment.force, timestamp);
    }

    void calcAlive(){

        for(uint32_t i = 0; i < num_planets; i++){

            while(!planetList[i].SithPQ.empty()){

                generalList[planetList[i].SithPQ.top().generalID].troopsAlive += planetList[i].SithPQ.top().quantity;
                planetList[i].SithPQ.pop();
            }

            while(!planetList[i].JediPQ.empty()){

                generalList[planetList[i].JediPQ.top().generalID].troopsAlive += planetList[i].JediPQ.top().quantity;
                planetList[i].JediPQ.pop();
            }
        }
    }

    void lastMedian(){

        for(uint32_t i = 0; i < num_planets; i++){

            planetList[i].calcMedian(currentTime);
        }
    }

    void updateTS(){

        previousTS = timestamp;
    }

    void errorChecking(){

        if(deployment.generalID >= num_generals){

            cout << "Invalid general ID\n";
            exit(1);
        }

        if(planetNum >= num_planets){

            cout << "Invalid planet ID\n";
            exit(1);
        }

        if(previousTS > timestamp){

            cout << "Invalid decreasing timestamp\n";
            exit(1);
        }

        if(deployment.force == 0){

            cout << "Invalid force sensntivity level\n";
            exit(1);
        }

        if(deployment.quantity == 0){

            cout << "Invalid number of troops\n";
            exit(1);
        }
    }
};

//--------------------Main Driver-----------------------//

int main(int argc, char * argv[]){

    ios_base::sync_with_stdio(false);
    
    int coun = 0;
    int choice = 0;
    galaxy Galaxy;


  option long_options[] = {
    {"verbose", no_argument, nullptr, 'v'},
    {"median", no_argument, nullptr, 'm'},
    {"general-eval", no_argument, nullptr, 'g'},
    {"watcher", no_argument, nullptr, 'w'},
    {"help", no_argument, nullptr, 'h'},
    {nullptr, 0, nullptr, '\0'}
    };

    while ((choice = getopt_long(argc, argv, "vmgw", long_options, nullptr)) != -1) {
        switch (choice) {

          case 'v':

              Galaxy.verboseMode = true;
              coun++;
              break;

          case 'm':

              Galaxy.medianMode = true;
              coun++;
              break;

          case 'g':

              Galaxy.generalEval = true;
              break; 

          case 'w':

              Galaxy.watcherMode = true;
              break; 
          
          case 'h':
              cout << "suu" << endl;
              return 0;
          } 
        
    } 

    Galaxy.readStartInfo();

    cout << "Deploying troops...\n"; 

    for(uint32_t i = 0; i < Galaxy.num_planets; i++){

        Galaxy.planetList[i].planetID = i;
    }

    for(uint32_t i = 0; i < Galaxy.num_generals; i++){

        Galaxy.generalList[i].genID = i;
    }

    if(Galaxy.inputMode == "PR"){

        P2random::PR_init(Galaxy.ss, Galaxy.seed, Galaxy.num_generals, Galaxy.num_planets, Galaxy.num_deployments, Galaxy.arrival_rate);
    }

    istream &inputStream = Galaxy.inputMode == "PR" ? Galaxy.ss : cin;

    while(inputStream >> Galaxy.timestamp >> Galaxy.junk >> Galaxy.letterjunk >> Galaxy.deployment.generalID >> Galaxy.letterjunk >> 
        Galaxy.planetNum >> Galaxy.letterjunk >> Galaxy.deployment.force>> Galaxy.letterjunk >> Galaxy.deployment.quantity){

        Galaxy.errorChecking();
        Galaxy.assign();

        if(Galaxy.watcherMode == true){

            Galaxy.watch();
        }

        Galaxy.battleSim();
        Galaxy.updateTS();
    }

    if(Galaxy.medianMode == true){

        Galaxy.lastMedian();
    }

    Galaxy.calcBattles();
    cout << "---End of Day---\n" << "Battles: " << Galaxy.sumBattles << "\n";
    


    if(Galaxy.generalEval == true){

        cout << "---General Evaluation---\n";

        Galaxy.calcAlive();

        for(uint32_t i = 0; i < Galaxy.num_generals; i++){

            cout << "General " << Galaxy.generalList[i].genID << " deployed " << Galaxy.generalList[i].totalJedi << " Jedi troops and " << Galaxy.generalList[i].totalSith << " Sith troops, and " << Galaxy.generalList[i].troopsAlive << "/" << (Galaxy.generalList[i].totalSith + Galaxy.generalList[i].totalJedi) << " troops survived.\n";
        }
    }

    //Movie Watcher Mode

    if(Galaxy.watcherMode == true){

        cout << "---Movie Watcher---\n";
        
        for(uint32_t i = 0; i < Galaxy.num_planets; i++){

            if(Galaxy.planetList[i].totalBattles == 0 || Galaxy.planetList[i].ambush.state == State::Inital || Galaxy.planetList[i].ambush.state == State::SeenOne){

                cout << "A movie watcher would not see an interesting ambush on planet " << Galaxy.planetList[i].planetID << ".\n";
            }

            else if(Galaxy.planetList[i].totalBattles > 0 && Galaxy.planetList[i].ambush.state == State::SeenBoth){

                cout << "A movie watcher would enjoy an ambush on planet " << Galaxy.planetList[i].planetID << " with Sith at time " << Galaxy.planetList[i].ambush.currentSithTS << " and Jedi at time " << Galaxy.planetList[i].ambush.currentJediTS << " with a force difference of " << Galaxy.planetList[i].ambush.difference << ".\n";
            }

            if(Galaxy.planetList[i].totalBattles == 0 || Galaxy.planetList[i].attack.state == State::Inital || Galaxy.planetList[i].attack.state == State::SeenOne){

                cout << "A movie watcher would not see an interesting attack on planet " << Galaxy.planetList[i].planetID << ".\n";
            }

            else if(Galaxy.planetList[i].totalBattles > 0 && Galaxy.planetList[i].attack.state == State::SeenBoth){

                cout << "A movie watcher would enjoy an attack on planet " << Galaxy.planetList[i].planetID << " with Jedi at time " << Galaxy.planetList[i].attack.currentJediTS << " and Sith at time " << Galaxy.planetList[i].attack.currentSithTS << " with a force difference of " << Galaxy.planetList[i].attack.difference << ".\n";
            }
        }
    }
}

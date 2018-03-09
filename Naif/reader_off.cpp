#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <thread>
#include "../util.hpp"
#include "omp.h"
#include<random>
#include<chrono>
#include <map>

using namespace std;
typedef std::map<vector<int>, int> Map;
mt19937 gen(chrono::system_clock::now().time_since_epoch().count());

template <class T> void listShuffle(list<T> &L){
  vector<T> V( L.begin(), L.end() );
  shuffle( V.begin(), V.end(), gen );
  L.assign( V.begin(), V.end() );
}


std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}


std::vector<std::list<std::vector<int>>> splitlist(int lengthmax, int numlist, const std::list<std::vector<int>> &edges){
//std::vector<std::list<std::pair<vector<int>, int>>>splitlist(int lengthmax, int numlist, const std::list<std::pair<vector<int>, int>> &edges){
  std::vector<std::list<std::vector<int>>> edgelists(numlist);
  //std::vector<std::list<std::pair<vector<int>, int>>> edgelists(numlist);
  for (int i = 0; i < numlist; i++){
    int bgn = i * lengthmax;
    int ed = (i+1) * lengthmax;
    //cout << "  bornes " << bgn<< " " << ed << "\n";
    //cout << i << "/" << numlist-1 <<"\n";
    if (i == numlist -1){
      //cout << "  bornes " << i* lengthmax << " " <<  edges.size()<<"\n";
      ed = edges.size();
    }
    //std::list<std::pair<vector<int>, int>> temp(std::next(edges.begin(), bgn), std::next(edges.begin(),ed));
    std::list<std::vector<int>> temp(std::next(edges.begin(), bgn), std::next(edges.begin(),ed));
    edgelists[i] = temp;
  }
  return(edgelists);
}


std::list<std::vector<int>> read_edges(std::string filepath){

  // Read file
  int n_verts;
  int n_faces;
  std::vector<std::string> strs;
  std::ifstream file;
  file.open(filepath);

  std::string templine;
  getline(file,templine);
  getline(file,templine);
  strs = split(templine, ' ');

  n_verts = std::stoi(strs[0]);
  n_faces =  std::stoi(strs[1]);
  cout << n_faces + n_verts<< "\n";

  std::vector<std::vector<float>> verts(n_verts, std::vector<float>(3));
  std::vector<std::vector<int>> faces(n_faces, std::vector<int>(3));

  for (int i = 0; i < n_verts; i++) {
    getline(file,templine);
    strs = split(templine, ' ');

    verts[i][0] =  std::stof(strs[0]);
    verts[i][1] =  std::stof(strs[1]);
    verts[i][2] =  std::stof(strs[2]);
  }

  for (int i = 0; i < n_faces; i++)  {

    getline(file,templine);
    strs = split(templine, ' ');

    faces[i][0] =  std::stoi(strs[1]);
    faces[i][1] =  std::stoi(strs[2]);
    faces[i][2] =  std::stoi(strs[3]);

    std::sort(faces[i].begin(),faces[i].end());
   }

  file.close();

  std::list<std::vector<int>>edges;

  for (int i = 0; i < n_faces; i++){

    std::vector<int> edge1(4);
    std::vector<int> edge2(4);
    std::vector<int> edge3(4);

    edge1[0] = faces[i][0];
    edge1[1] = faces[i][1];
    edge1[2] = i;
    edge2[0] = faces[i][0];
    edge2[1] = faces[i][2];
    edge2[2] = i;
    edge3[0] = faces[i][1];
    edge3[1] = faces[i][2];
    edge3[2] = i;

    edges.push_back (edge1);
    edges.push_back (edge2);
    edges.push_back (edge3);
  }
  return(edges);

}

int main(int argc, char* argv[]){

  std::string filepath = "../trimstar.off";

  std::list<std::vector<int>> edges = read_edges(filepath);
  //std::list<std::pair<vector<int>, int>> edges = read_edges(filepath);
  int total = edges.size();
  double start_time;      // Starting time
  double run_time;        // Timing
  util::Timer timer;

  timer.reset();
  start_time = static_cast<double>(timer.getTimeMilliseconds());

//Sequentiel naif

  int partial = total - edges.size();
  std::vector<std::vector<int>> newedges;

  while(!edges.empty()){

    partial = total - edges.size();
    std::list<std::vector<int>>::iterator edge = edges.begin();
    std::list<std::vector<int>>::iterator it = std::next(edge,0);
    bool found = false;

    while (std::next(it,1) != edges.end() && !found){
      it++;

      if((*edge)[0]==(*it)[0] && !found && (*edge)[1]==(*it)[1]){

        std::vector<int> newe(2);
        newe[0] = (*edge)[2];
        newe[1] = (*it)[2];

	edges.erase(edge);
	edges.erase(it);

	newedges.push_back(newe);
        found = true;
      }
    }
    //cout << partial << "/" << total << "\n"<< std::flush;
  }

  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);
  return 0;
}

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
#include "util.hpp"
#include "omp.h"
#include<random>
#include<chrono>
#include <map>

using namespace std;
typedef std::map<vector<int>, int> Map;
mt19937 gen(chrono::system_clock::now().time_since_epoch().count());

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

std::vector<std::list<std::pair<vector<int>, int>>>splitlist(int lengthmax, int numlist, const std::list<std::pair<vector<int>, int>> &edges){
  std::vector<std::list<std::pair<vector<int>, int>>> edgelists(numlist);
  for (int i = 0; i < numlist; i++){
    int bgn = i * lengthmax;
    int ed = (i+1) * lengthmax;
    if (i == numlist -1){
      ed = edges.size();
    }
    std::list<std::pair<vector<int>, int>> temp(std::next(edges.begin(), bgn), std::next(edges.begin(),ed));
    edgelists[i] = temp;
  }
  return(edgelists);
}

// Lecture du fichier et transformation en liste de paire clé-valeur

std::list<std::pair<std::vector<int>,int>> read_edges(std::string filepath){

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

  std::list<std::pair<std::vector<int>,int>>edges;
  for (int i = 0; i < n_faces; i++){

    std::vector<int> edge1(2);
    std::vector<int> edge2(2);
    std::vector<int> edge3(2);

    edge1[0] = faces[i][0];
    edge1[1] = faces[i][1];
    edge2[0] = faces[i][0];
    edge2[1] = faces[i][2];
    edge3[0] = faces[i][1];
    edge3[1] = faces[i][2];

    edges.push_back (std::pair<std::vector<int>, int>(edge1, i));
    edges.push_back (std::pair<std::vector<int>, int>(edge2, i));
    edges.push_back (std::pair<std::vector<int>, int>(edge3, i));
  }

  return(edges);

}


int main(int argc, char* argv[]){

  std::string filepath = "Armadillo.off";
  std::list<std::pair<vector<int>, int>> edges = read_edges(filepath);
  int total = edges.size();
  double start_time;
  double run_time;
  util::Timer timer;
  
  timer.reset();
  start_time = static_cast<double>(timer.getTimeMilliseconds());
  Map mymap;
  //parallel

  int nbthread = std::thread::hardware_concurrency();
  
  std::vector<std::vector<int>> newedges;
  std::pair<std::map<vector<int>, int>::iterator,bool> ret;
  int lengthmax = total/nbthread;
  int numlist = total/lengthmax + 1;
  std::vector<std::list<std::pair<vector<int>, int>>> edgelists = splitlist(lengthmax, numlist, edges);
  
  #pragma omp parallel for
  for (int i = 0; i < numlist; i++){
    
    std::list<std::pair<vector<int>, int>>::iterator edge = edgelists[i].begin();
    while(edge !=  edgelists[i].end()){
      
      std::pair<std::map<vector<int>, int>::iterator,bool> ret;
      #pragma omp critical
      ret = mymap.insert(*edge);
      if (ret.second == 0){
        std::vector<int>newe(2);
        newe[0] =  ret.first->second;
        newe[1] = (*edge).second;
        #pragma omp critical
        newedges.push_back(newe);
      }
      edge++;
    }
  }
  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);

  //sequentiel
  /*
  std::vector<std::vector<int>> newedges;
  std::pair<std::map<vector<int>, int>::iterator,bool> ret;
  std::list<std::pair<vector<int>, int>>::iterator edge = edges.begin();
  while(edge != edges.end()){
    std::pair<std::map<vector<int>, int>::iterator,bool> ret;
    ret = mymap.insert(*edge);
    if (ret.second == 0){
      std::vector<int>newe(2);
      newe[0] =  ret.first->second;
      newe[1] = (*edge).second;
      newedges.push_back(newe);
    }
    edge++;
  }
  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);*/
}

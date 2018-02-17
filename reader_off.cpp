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
#include "util.hpp"

using namespace std;

std::vector<std::string> split(const std::string &s, char delim) {
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (std::getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}



int main(void){


  // Read file
  char* filepath;
  filepath = "trimstar.off";
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

  //strip faces

  //std::vector<std::vector<int>> edges(3*n_faces, std::vector<int>(4));

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

  // match edges;

  int total = edges.size();
  double start_time;      // Starting time
  double run_time;        // Timing
  util::Timer timer;
  timer.reset();
  start_time = static_cast<double>(timer.getTimeMilliseconds());
  //Sequentiel naif

  /*int partial = total - edges.size();

  std::vector<std::vector<int>> newedges;

  while(!edges.empty()){
    partial = total - edges.size();
    std::list<std::vector<int>>::iterator edge = edges.begin();
    std::list<std::vector<int>>::iterator it = std::next(edge,0);
    bool found = false;
    while (std::next(it,1) != edges.end() && !found){
      it++;
      if((*edge)[0]==(*it)[0] && !found && (*edge)[1]==(*it)[1]){
        edges.erase(edge);
        edges.erase(it);
        std::vector<int> newe(2);
        newe[0] = (*edge)[2];
        newe[1] = (*it)[2];
        newedges.push_back(newe);
        found = true;
      }
    }
    printf("%d/%d\n", partial, total);
  }
  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);
  */
  //sequentiel non naif
  //split list
  int lengthmax = 10000;
  int numlist = total/lengthmax + 1;

  std::vector<std::list<std::vector<int>>> edgelists(numlist);
  for (int i = 0; i < numlist; i++){
    int bgn = i * lengthmax;
    int ed = (i+1) * lengthmax;
    if (i == numlist -1){
      ed = total;
    }
    std::list<std::vector<int>> temp(std::next(edges.begin(), bgn), std::next(edges.begin(),ed));
    edgelists[i] = temp;
  }


  std::vector<std::vector<int>> newedges;
  #pragma omp for
  for (int i = 0; i < numlist; i++){
    int avance = 0;
    while(!edgelists[i].empty() && avance < edgelists[i].size()){
      if(i == 3){
      }
      std::list<std::vector<int>>::iterator edge = std::next(edgelists[i].begin(), avance);
      std::list<std::vector<int>>::iterator it = edge;

      bool found = false;
      while (std::next(it,1) != edgelists[i].end() && !found){
        it++;
        if((*edge)[0]==(*it)[0] && (*edge)[1]==(*it)[1] && it != edge){
          edgelists[i].erase(edge);
          edgelists[i].erase(it);
          std::vector<int> newe(2);
          newe[0] = (*edge)[2];
          newe[1] = (*it)[2];
          newedges.push_back(newe);
          found = true;

        }
      }
      if (!found){
        avance +=1;
      }
    }
  }
  std::list<std::vector<int>> edgelistmerge;
  for (int i = 0; i < numlist; i++){
    edgelistmerge.splice(edgelistmerge.end(), edgelists[i]);
  }


  while(!edgelistmerge.empty()){
    //partial = total - edgelistmerge.size();
    std::list<std::vector<int>>::iterator edge = edgelistmerge.begin();
    std::list<std::vector<int>>::iterator it = std::next(edge,0);
    bool found = false;
    while (std::next(it,1) != edgelistmerge.end() && !found){
      it++;
      if((*edge)[0]==(*it)[0] && !found && (*edge)[1]==(*it)[1]){
        edgelistmerge.erase(edge);
        edgelistmerge.erase(it);
        std::vector<int> newe(2);
        newe[0] = (*edge)[2];
        newe[1] = (*it)[2];
        newedges.push_back(newe);
        found = true;
      }
    }
  }
  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);
}

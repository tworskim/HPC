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

  int partial = total - edges.size();
  std::vector<std::vector<int>> newedges;

  int nbthread = std::thread::hardware_concurrency();

  int lengthmax = total/nbthread + 1;
  int numlist;

  int lenres = total;
  int step = 0;

  while(lenres > lengthmax){
    std::vector<std::vector<int>> newedges;
    printf("size %d\n", lenres);

    if(step <= 2){
      lengthmax = 50 * (step + 1);
      numlist = lenres/lengthmax + 1;
      }
    else{
      lengthmax = 2000;
      numlist = lenres/lengthmax + 1;
      }
    //lengthmax = lenres/numlist;
    //cout <<"numlist "<<numlist <<" n " << lengthmax<< " Total " << total << " "<< lengthmax * numlist << "\n";

    std::vector<std::list<std::vector<int>>> edgelists = splitlist(lengthmax, numlist, edges);

    #pragma omp parallel for
    for (int i = 0; i < numlist; i++){
      int tid = omp_get_thread_num();
      //printf("Hello World from thread = %d\n", tid);
      int avance = 0;
      //std::list<std::vector<int>>::iterator edge = std::next(edgelists[i].begin(), avance);

      while(!edgelists[i].empty() && avance < edgelists[i].size()){

	std::list<std::vector<int>>::iterator edge = std::next(edgelists[i].begin(), avance);
	std::list<std::vector<int>>::iterator it = edge;
        bool found = false;
	int stepi = 0;
	while (std::next(it,1) != edgelists[i].end() && !found){
          it++;
	  stepi ++;
	  if((*edge)[0]==(*it)[0] && (*edge)[1]==(*it)[1] && it != edge){
	    //cout << "verif" <<stepi + avance << " " << edgelists[i].size()<<"\n";
	    edgelists[i].erase(edge);
	    edgelists[i].erase(it);
            std::vector<int> newe(2);
            newe[0] = (*edge)[2];
            newe[1] = (*it)[2];
            #pragma omp critical
            newedges.push_back(newe);
            found = true;
	  }
	}

	if (!found){

	  avance +=1;
	  //cout << "avance" << avance << " "<<lenres;
	  edge++;
	  //printf("AV %d %d\n", avance, edgelists[i].size());
	}
      }
      //printf("GoodBye World from thread = %d\n", tid);
    }
    edges.clear();

    for (int i = 0; i < numlist; i++){
      edges.splice(edges.end(), edgelists[i]);
    }

    lenres = edges.size();

    if(step > 2){
      listShuffle(edges);
    }
    step ++;
  }

  while(!edges.empty()){
    std::vector<std::vector<int>> newedges;
    std::list<std::vector<int>>::iterator edge = edges.begin();
    std::list<std::vector<int>>::iterator it = edge;
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
  }
  run_time  = static_cast<double>(timer.getTimeMilliseconds()) - start_time;
  printf("%f", run_time);
  return 0;
}

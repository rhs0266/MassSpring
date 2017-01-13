#pragma once

#include "library.h"
#include "particle.h"

int click(V2 mouse, vector<Particle> *p){
	double max=100000000;
	int id = -1;
	int n = (int)(*p).size();
	for (int i=0;i<n;i++){
		double dist = (mouse - (*p)[i].p).norm();
		if (dist<max) max=dist, id=i;
	}
	if (max > 20) id = -1;
	return id;
}

void fixation(V2 mouse, vector<Particle> *p){
	int id = click(mouse, p);
	int n = (int)(*p).size();

	if (id == -1) return;

	(*p)[id].fixed = 1 - (*p)[id].fixed;
}

void follow(V2 mouse, vector<Particle> *p){
	int id = click(mouse, p);
	int n = (int)(*p).size();

	if (id == -1) return;
	if ((*p)[id].fixed == 0) return;

	(*p)[id].p = mouse;
}
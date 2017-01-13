#pragma once

#include "library.h"
const double dT = 0.01; // 0.01 second per one frame
const double g = 9.8; // gravity
const double k_s = 0.1; // spring constant
const double k_d = 0.005; // damping constant
const int N = 5; // N*N = number of particles
const double DEG2RAD = 3.141592/180;
const double coeff = 0.3; // coefficient for collision
const double air = 0.01; // air resist

struct Particle{
	V2 p; // position
	V2 F; // Force
	V2 v; // velocity
	double mass;
	int id;
	int fixed; // if fixed, F = v = 0
	vector<Particle*> connect;
	vector<double> restLength;
	Particle(){};
	Particle(V2 _p, V2 _v, double _mass, int _id, int _fixed = 0):p(_p),v(_v),mass(_mass),id(_id),fixed(_fixed){};
	void move(){ // change position to next frame
		if (fixed) return;
		F = V2(0,0);
		F = V2(0,-g) * mass;
		for (int i=0;i<connect.size();i++){
			Particle* q = connect[i];
			double rest = restLength[i];
			V2 diff = p - q->p; // that to this
			double len = diff.norm();
			if (len < 1e-6) continue;

			V2 vec = diff / len;
			V2 f = -(k_s * (len - rest) + k_d * (v - q->v).dot(vec)) * vec;

			F = F + f;
		}
		// printf("%lf ",F.norm());
		F = F + (-air) *  v; // drag force
		// printf("%lf\n",F.norm());

		V2 a = F / mass; // acceleration

		// --------------------- //
		V2 next_p = p + v * dT;
		V2 next_v = v + a * dT;
		// --------------------- //

		// if (next_p(1) < 0){
		// 	// TODO : handle collision
		// 	next_p(1) = 0;
		// 	next_v(1) = -next_v(1) * coeff;
		// }
		p = next_p;
		v = next_v;
	}
};

void push_edge(Particle *x, Particle *y, double rest){
	x->connect.push_back(y);
	x->restLength.push_back(rest);

	y->connect.push_back(x);
	y->restLength.push_back(rest);
}

vector<Particle> particles;
int dir1[4][2]={{0,1},{1,0},{0,-1},{-1,0}};
int dir2[4][2]={{1,1},{1,-1},{-1,-1},{-1,1}};
int dir3[4][2]={{0,2},{2,0},{0,-2},{-2,0}};
void Initialize(){
	particles.resize(N*N);
	for (int t=0;t<N*N;t++){
		int i = t/N, j = t%N;
		particles[t]=Particle(V2(-50 * (N/2),100) + V2(50,0)*i + V2(0,50)*j, V2(0,0), 2.0 / (N*N), t, 0);
		for (int k=0;k<4;k++){
			int i2 = i + dir1[k][0], j2 = j + dir1[k][1];
			if (i2<0 || j2<0 || i2>=N || j2>=N) continue;
			int t2 = i2 * N + j2;
			push_edge(&particles[t], &particles[t2], 50);
		}
		for (int k=0;k<4;k++){
			int i2 = i + dir2[k][0], j2 = j + dir2[k][1];
			if (i2<0 || j2<0 || i2>=N || j2>=N) continue;
			int t2 = i2 * N + j2;
			push_edge(&particles[t], &particles[t2], 50 * sqrt(2.0));
		}
		// for (int k=0;k<4;k++){
		// 	int i2 = i + dir3[k][0], j2 = j + dir3[k][1];
		// 	if (i2<0 || j2<0 || i2>=N || j2>=N) continue;
		// 	int t2 = i2 * N + j2;
		// 	push_edge(&particles[t], &particles[t2], 50 * 2);
		// }
	}
	particles[N-1].fixed = 1;
	particles[N*N-1].fixed = 1;
}


void drawCircle(V2 p, double radius){
	glBegin(GL_LINE_LOOP);
	int M=360;
	for (int i=0;i<M;i++){
		double deg = (i * 360 / M) * DEG2RAD;
		V2P((p + V2(cos(deg)*radius, sin(deg)*radius)));
	}
	glEnd();
}

void drawParticles(){
	for (int i=0;i<N*N;i++){
		if (particles[i].fixed==1) glColor3f(255,255,0);
		drawCircle(particles[i].p, 5.0);
		glColor3f(255,255,255);
		for (int j=0;j<particles[i].connect.size();j++){
			Particle q = *particles[i].connect[j];
			glBegin(GL_LINE_STRIP);
			V2P(particles[i].p);
			V2P(q.p);
			glEnd();
		}
	}
}

void nextFrame(){
	for (int i=0;i<N*N;i++){
		particles[i].move();
	}
}

vector<Particle> *getParticles(){
	return &particles;
}
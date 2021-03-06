#ifndef PLANE_SIM_H
#define PLANE_SIM_H

#include "Voter.h"
#include "VotingSystem.h"

#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>

class XYSource;
class DoubleRandom;
class GaussianRandom;
#if HAVE_PROTOBUF
class MessageLiteWriter;
class Result2;
#endif

class pos {
public:
	double x, y;
};

class ResultAccumulation;

class PlaneSim {
public:
	class candidatearg;

	pos* candidates;
	VoterArray they;
	
	double minx, maxx, miny, maxy;
	int px, py;
	double voterSigma;
	int electionsPerPixel;

	bool manhattanDistance;
	bool linearFalloff;

	VotingSystem** systems;
	int systemsLength;
	// there must be the same number of result accumulations as systems
	ResultAccumulation** accum;

	// indicates that certain pointers aren't owned and should not be freed
	bool isSlave;

	candidatearg* candroot;
	int candcount;

	//uint8_t* pix;

	int seats;
	bool doCombinatoricExplode;
	VoterArray exploded;
	int* combos;

	DoubleRandom* rootRandom;
	GaussianRandom* gRandom;

	PlaneSim() : candidates( NULL ), minx( -2.0 ), maxx( 2.0 ), miny( -2.0 ), maxy( 2.0 ),
		px( 500 ), py( 500 ), voterSigma( 0.5 ), electionsPerPixel( 10 ),
		manhattanDistance( false ), linearFalloff( false ), accum( NULL ), isSlave( false ),
		candroot( NULL ), candcount( 0 ), //pix( NULL ),
		seats(1), doCombinatoricExplode(false), combos(NULL),
		rootRandom(NULL), gRandom(NULL)
	{}
	~PlaneSim() {
		if (combos) {delete [] combos;}
	}
	
	void build( int numv );
	// build this to be a slave to it.
	void coBuild( const PlaneSim& it );
	
	// Does not take ownership of array so that it may be shared across threads.
	void setVotingSystems(VotingSystem** systems, int numSystems);
	
	void randomizeVoters( double centerx, double centery, double sigma );
	
	void runXYSource(XYSource* source);
	void runPixel(int x, int y, double dx, double dy, int* winners);

	size_t configStr( char* dest, size_t len );

	inline double xIndexToCoord( int x ) {
		double dx;
		dx = (maxx - minx)/(px - 1);
		return minx + dx*x;
	}
	inline double yIndexToCoord( int y ) {
		double dy;
		dy = (maxy - miny)/(py - 1);
		return miny + dy*y;
	}
	
	inline int xCoordToIndex( double x ) {
		double dx;
		dx = (maxx - minx)/(px - 1);
		return (int)floor( (x - minx) / dx );
	}
	inline int yCoordToIndex( double y ) {
		double dy;
		dy = (maxy - miny)/(py - 1);
		return (int)floor( (y - miny) / dy );
	}

	void addCandidateArg( const char* arg );
	void addCandidateArg( double x, double y );
	
#if HAVE_PROTOBUF
	Result2** runRandomXY(Result2**);
#endif
	void calculateHappiness(int* winners, double* happinessP, double* stddevP, double* giniP);
	
	public:
	class candidatearg {
	public:
		double x,y;
		candidatearg* next;
		candidatearg( const char* arg, candidatearg* next );
		candidatearg( double xIn, double yIn, candidatearg* nextIn )
			: x(xIn), y(yIn), next(nextIn) {}
	};
};

class PlaneSimThread {
public:
	PlaneSim* sim;
	pthread_t thread;
	XYSource* source;
#if HAVE_PROTOBUF
	int* countToDo;
	pthread_mutex_t* countLock;
	MessageLiteWriter* writer;
	pthread_mutex_t* writerLock;
#endif
};

// for pthread_create, arg is PlaneSimThread*
extern "C" void* runPlaneSimThread(void* arg);
#if HAVE_PROTOBUF
extern "C" void* runRandomTestThread(void* arg);
#endif

#endif /* PLANE_SIM_H */

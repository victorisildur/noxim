#ifndef __NOXIMROUTING_RPATH_H__
#define __NOXIMROUTING_RPATH_H__

#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"
#include "../Router.h"

using namespace std;

class Routing_RPATH : RoutingAlgorithm {
public:
    vector<int> route(Router * router, const RouteData & routeData);

    static Routing_RPATH * getInstance();

private:
    Routing_RPATH(){};
    ~Routing_RPATH(){};

    static Routing_RPATH * routing_RPATH;
    static RoutingAlgorithmsRegister routingAlgorithmsRegister;

    int coord2Hlabel(const Coord coord);
    Coord hlabel2Coord(const int label);

    int coord2Vlabel(const Coord coord);
    Coord vlabel2Coord(const int label);

    int calcDir(const Coord next, const Coord curr);
};

#endif

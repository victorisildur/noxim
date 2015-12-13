#ifndef __NOXIMROUTING_LOADAWARE_H__
#define __NOXIMROUTING_LOADAWARE_H__

#include "RoutingAlgorithm.h"
#include "RoutingAlgorithms.h"
#include "../Router.h"

using namespace std;

class Routing_LOADAWARE : RoutingAlgorithm {
public:
    vector<int> route(Router * router, const RouteData & routeData);

    static Routing_LOADAWARE * getInstance();

private:
    Routing_LOADAWARE(){};
    ~Routing_LOADAWARE(){};

    static Routing_LOADAWARE * routing_LOADAWARE;
    static RoutingAlgorithmsRegister routingAlgorithmsRegister;

    int coord2Hlabel(const Coord coord);
    Coord hlabel2Coord(const int label);

    int coord2Vlabel(const Coord coord);
    Coord vlabel2Coord(const int label);

    int calcDir(const Coord next, const Coord curr);
};

#endif

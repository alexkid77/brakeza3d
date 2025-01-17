//
// Created by darkhead on 5/3/21.
//

#ifndef BRAKEDA3D_PATHFINDER_H
#define BRAKEDA3D_PATHFINDER_H

#include <iostream>
#include <utility>
#include <math.h>       /* sqrt */
#include <stack>          // std::stack
#include <set>
#include <vector>
#include "../Objects/Vertex3D.h"

#define FLT_MAX 10
typedef std::pair<int, int> PairData;


class PathFinder {
public:
    // Creating a shortcut for int, int pair type

    void demo();

    void consoleDebug();

    void consoleDebugPath(std::stack<PairData> path);

    bool AStarSearch(PairData src, PairData dest, std::stack<PairData> &path);

    void setValue(int posX, int posY, bool value);

    PathFinder(int sizeX, int sizeY);

    void saveGridToPNG(const std::string& filename);

    void loadGridFromPNG(const std::string& filename);

    static std::stack<PairData> readPathFromPNG(const std::string& filename);

private:
    // Creating a shortcut for pair<int, pair<int, int>> type
    typedef std::pair<double, std::pair<int, int> > pPair;

    // A structure to hold the neccesary parameters
    struct cell {
        // Row and Column index of its parent
        // Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
        int parent_i, parent_j;
        // f = g + h
        double f, g, h;
    };

    const int sizeX;
    const int sizeY;
    int **grid;

    bool isValid(int row, int col) const;

    static bool isUnBlocked(int **grid, int row, int col);

    static bool isDestination(int row, int col, PairData dest);

    static double calculateHValue(int row, int col, PairData dest);

    static std::stack<PairData> tracePath(cell **cellDetails, PairData dest);

};


#endif //BRAKEDA3D_PATHFINDER_H

#ifndef TEAM_H
#define TEAM_H
#include <vector>
#include "worm.h"

class Team
{
public:
    std::vector<Worm*> vecMembers;
    int nCurrentMember = 0;		// Index into vector for current worms turn
    int nTeamSize = 0;			// Total number of worms in team

    bool IsTeamAlive();
    Worm* GetNextMember();
};

#endif // TEAM_H

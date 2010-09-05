// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
// DESCRIPTION:
//	Enemy thinking, AI.
//	Action Pointer Functions
//	that are associated with states/frames. 
//
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "m_random.h"
#include "i_system.h"

#include "doomdef.h"
#include "p_local.h"

#include "s_sound.h"

#include "g_game.h"

// State.
#include "doomstat.h"
#include "r_state.h"

// Data.
#include "sounds.h"


// Forward Declarations:
void A_RandomWalk(mobj_t *);


typedef enum
{
    DI_EAST,
    DI_NORTHEAST,
    DI_NORTH,
    DI_NORTHWEST,
    DI_WEST,
    DI_SOUTHWEST,
    DI_SOUTH,
    DI_SOUTHEAST,
    DI_NODIR,
    NUMDIRS
    
} dirtype_t;


//
// P_NewChaseDir related LUT.
//
dirtype_t opposite[] =
{
  DI_WEST, DI_SOUTHWEST, DI_SOUTH, DI_SOUTHEAST,
  DI_EAST, DI_NORTHEAST, DI_NORTH, DI_NORTHWEST, DI_NODIR
};

dirtype_t diags[] =
{
    DI_NORTHWEST, DI_NORTHEAST, DI_SOUTHWEST, DI_SOUTHEAST
};





void A_Fall (mobj_t *actor);


//
// ENEMY THINKING
// Enemies are allways spawned
// with targetplayer = -1, threshold = 0
// Most monsters are spawned unaware of all players,
// but some can be made preaware
//


//
// Called by P_NoiseAlert.
// Recursively traverse adjacent sectors,
// sound blocking lines cut off traversal.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified
//

mobj_t*		soundtarget;

void
P_RecursiveSound
( sector_t*	sec,
  int		soundblocks )
{
    int		i;
    line_t*	check;
    sector_t*	other;
	
    // wake up all monsters in this sector
    if (sec->validcount == validcount
	&& sec->soundtraversed <= soundblocks+1)
    {
	return;		// already flooded
    }
    
    sec->validcount = validcount;
    sec->soundtraversed = soundblocks+1;
    sec->soundtarget = soundtarget;
	
    for (i=0 ;i<sec->linecount ; i++)
    {
	check = sec->lines[i];
	if (! (check->flags & ML_TWOSIDED) )
	    continue;
	
	P_LineOpening (check);

	if (openrange <= 0)
	    continue;	// closed door
	
	if ( sides[ check->sidenum[0] ].sector == sec)
	    other = sides[ check->sidenum[1] ] .sector;
	else
	    other = sides[ check->sidenum[0] ].sector;
	
	if (check->flags & ML_SOUNDBLOCK)
	{
	    if (!soundblocks)
		P_RecursiveSound (other, 1);
	}
	else
	    P_RecursiveSound (other, soundblocks);
    }
}



//
// P_NoiseAlert
// If a monster yells at a player,
// it will alert other monsters to the player.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified
//
void
P_NoiseAlert
( mobj_t*	target,
  mobj_t*	emmiter )
{
    soundtarget = target;
    validcount++;
    P_RecursiveSound (emmiter->subsector->sector, 0);
}

//
// P_WakeUpThing
//
// villsa [STRIFE] New function
// Wakes up an mobj.nearby when somebody has been punched.
//
static void P_WakeUpThing(mobj_t* puncher, mobj_t* bystander)
{
    if(!(bystander->flags & MF_INCOMBAT))
    {
        bystander->target = puncher;
        if(bystander->info->seesound)
            S_StartSound(bystander, bystander->info->seesound);
        P_SetMobjState(bystander, bystander->info->seestate);
    }
}

//
// P_DoPunchAlert
//
// villsa [STRIFE] New function (by Quasar ;)
// Wake up buddies nearby when the player thinks he's gotten too clever
// with the punch dagger. Walks sector links.
//
void P_DoPunchAlert(mobj_t *puncher, mobj_t *punchee)
{   
   mobj_t *rover;
   
   // don't bother with this crap if we're already on alert
   if(punchee->subsector->sector->soundtarget)
      return;
      
   // gotta still be alive to call for help
   if(punchee->health <= 0)
      return;
      
   // has to be something you can wake up and kill too
   if(!(punchee->flags & MF_COUNTKILL) || punchee->flags & MF_INCOMBAT)
      return;
   
   // make the punchee hurt - haleyjd 09/05/10: Fixed to use painstate.
   punchee->target = puncher;
   P_SetMobjState(punchee, punchee->info->painstate); 
   
   // wake up everybody nearby
   
   // scan forward on sector list
   for(rover = punchee->snext; rover; rover = rover->snext)
   {
      // we only wake up certain thing types (Acolytes and Templars?)
      if(rover->health > 0 && rover->type >= MT_GUARD1 && rover->type <= MT_PGUARD &&
         (P_CheckSight(rover, puncher) || P_CheckSight(rover, punchee)))
      {
         P_WakeUpThing(puncher, rover);
         rover->flags |= MF_INCOMBAT;
      }
   }

   // scan backward on sector list
   for(rover = punchee->sprev; rover; rover = rover->sprev)
   {
      // we only wake up certain thing types (Acolytes and Templars?)
      if(rover->health > 0 && rover->type >= MT_GUARD1 && rover->type <= MT_PGUARD &&
         (P_CheckSight(rover, puncher) || P_CheckSight(rover, punchee)))
      {
         P_WakeUpThing(puncher, rover);
         rover->flags |= MF_INCOMBAT;
      }
   }
}




//
// P_CheckMeleeRange
//
// [STRIFE] Minor change to meleerange.
//
boolean P_CheckMeleeRange(mobj_t* actor)
{
    mobj_t*	pl;
    fixed_t	dist;

    if(!actor->target)
        return false;

    pl = actor->target;
    if(actor->z + 3 * actor->height / 2 < pl->z) // villsa [STRIFE]
        return false;

    dist = P_AproxDistance(pl->x - actor->x, pl->y - actor->y);

    // villsa [STRIFE] change to 36
    if(dist >= MELEERANGE - 36*FRACUNIT + pl->info->radius)
        return false;

    if(!P_CheckSight (actor, actor->target))
        return false;

    return true;
}

//
// P_CheckMissileRange
//
// [STRIFE]
// Changes to eliminate DOOM-specific code and to allow for
// varying attack ranges for Strife monsters, as well as a general tweak
// to considered distance for all monsters.
//
boolean P_CheckMissileRange(mobj_t* actor)
{
    fixed_t dist;

    if(!P_CheckSight(actor, actor->target))
        return false;

    if(actor->flags & MF_JUSTHIT)
    {
        // the target just hit the enemy,
        // so fight back!
        actor->flags &= ~MF_JUSTHIT;
        return true;
    }

    if(actor->reactiontime)
        return false;	// do not attack yet

    // OPTIMIZE: get this from a global checksight
    dist = P_AproxDistance(actor->x-actor->target->x,
                           actor->y-actor->target->y) - 64*FRACUNIT;
    
    if (!actor->info->meleestate)
        dist -= 128*FRACUNIT;	// no melee attack, so fire more

    dist >>= 16;

    // villsa [STRIFE] checks for acolytes
    //  haleyjd 09/05/10: Repaired to match disassembly: Was including 
    //  SHADOWGUARD in the wrong case, was missing MT_SENTINEL entirely.
    //  Structure of ASM also indicates this was probably a switch 
    //  statement turned into a cascading if/else by the compiler.
    switch(actor->type)
    {
    case MT_GUARD1:
    case MT_GUARD2:
    case MT_GUARD3:
    case MT_GUARD4:
    case MT_GUARD5:
    case MT_GUARD6:
        // oddly, not all Acolytes are included here...
        dist >>= 4;
        break;
    case MT_SHADOWGUARD:
    case MT_CRUSADER:
    case MT_SENTINEL:
        dist >>= 1;
        break;
    default:
        break;
    }
    
    // villsa [STRIFE] changed to 150
    if (dist > 150)
        dist = 150;

    if (P_Random () < dist)
        return false;

    return true;
}

//
// P_CheckRobotRange
//
// villsa [STRIFE] New function
//
boolean P_CheckRobotRange(mobj_t *actor)
{
    fixed_t dist;

    if(!P_CheckSight(actor, actor->target))
        return false;

    if(actor->reactiontime)
        return false;    // do not attack yet

    dist = (P_AproxDistance(actor->x-actor->target->x,
                            actor->y-actor->target->y) - 64*FRACUNIT) >> FRACBITS;

    return (dist < 200);
}


//
// P_Move
// Move in the current direction,
// returns false if the move is blocked.
//
// [STRIFE]
// villsa/haleyjd 09/05/10: Modified for terrain types and 3D object 
// clipping. Below constants are verified to be unmodified:
//
fixed_t	xspeed[8] = {FRACUNIT,47000,0,-47000,-FRACUNIT,-47000,0,47000};
fixed_t yspeed[8] = {0,47000,FRACUNIT,47000,0,-47000,-FRACUNIT,-47000};

#define MAXSPECIALCROSS	8

extern	line_t*	spechit[MAXSPECIALCROSS];
extern	int	numspechit;

boolean P_Move (mobj_t*	actor)
{
    fixed_t	tryx;
    fixed_t	tryy;

    line_t*	ld;

    // warning: 'catch', 'throw', and 'try'
    // are all C++ reserved words
    boolean	try_ok;
    boolean	good;

    if (actor->movedir == DI_NODIR)
        return false;

    if ((unsigned)actor->movedir >= 8)
        I_Error ("Weird actor->movedir!");

    tryx = actor->x + actor->info->speed*xspeed[actor->movedir];
    tryy = actor->y + actor->info->speed*yspeed[actor->movedir];

    try_ok = P_TryMove (actor, tryx, tryy);

    if (!try_ok)
    {
        // open any specials
        if (actor->flags & MF_FLOAT && floatok)
        {
            // must adjust height
            if (actor->z < tmfloorz)
                actor->z += FLOATSPEED; // [STRIFE] Note FLOATSPEED == 5*FRACUNIT
            else
                actor->z -= FLOATSPEED;

            actor->flags |= MF_INFLOAT;
            return true;
        }

        if (!numspechit)
            return false;

        actor->movedir = DI_NODIR;
        good = false;
        while (numspechit--)
        {
            ld = spechit[numspechit];
            // if the special is not a door
            // that can be opened,
            // return false
            if (P_UseSpecialLine (actor, ld,0))
                good = true;
        }
        return good;
    }
    else
    {
        actor->flags &= ~(MF_INFLOAT|MF_FEETCLIPPED);   // villsa [STRIFE]

        // villsa [STRIFE]
        if(P_GetTerrainType(actor) != FLOOR_SOLID)
            actor->flags |= MF_FEETCLIPPED;
    }


    // villsa [STRIFE] Removed pulling non-floating actors down to the ground.
    //  (haleyjd 09/05/10: Verified)
    /*if (! (actor->flags & MF_FLOAT) )	
          actor->z = actor->floorz;*/

    return true; 
}


//
// TryWalk
// Attempts to move actor on
// in its current (ob->moveangle) direction.
// If blocked by either a wall or an actor
// returns FALSE
// If move is either clear or blocked only by a door,
// returns TRUE and sets...
// If a door is in the way,
// an OpenDoor call is made to start it opening.
//
// haleyjd 09/05/10: [STRIFE] Verified unmodified.
//
boolean P_TryWalk (mobj_t* actor)
{
    if (!P_Move (actor))
    {
        return false;
    }

    actor->movecount = P_Random()&15;
    return true;
}



//
// P_NewChaseDir
//

void P_NewChaseDir(mobj_t* actor)
{
    fixed_t	deltax;
    fixed_t	deltay;
    
    dirtype_t	d[3];
    
    int		tdir;
    dirtype_t	olddir;
    
    dirtype_t	turnaround;

    // villsa [STRIFE] don't bomb out and instead set spawnstate
    if(!actor->target)
    {
        //I_Error("P_NewChaseDir: called with no target");
        P_SetMobjState(actor, actor->info->spawnstate);
        return;
    }

    olddir = actor->movedir;
    turnaround=opposite[olddir];

    deltax = actor->target->x - actor->x;
    deltay = actor->target->y - actor->y;

    if (deltax>10*FRACUNIT)
        d[1]= DI_EAST;
    else if (deltax<-10*FRACUNIT)
        d[1]= DI_WEST;
    else
        d[1]=DI_NODIR;

    if (deltay<-10*FRACUNIT)
        d[2]= DI_SOUTH;
    else if (deltay>10*FRACUNIT)
        d[2]= DI_NORTH;
    else
        d[2]=DI_NODIR;

    // try direct route
    if (d[1] != DI_NODIR
        && d[2] != DI_NODIR)
    {
        actor->movedir = diags[((deltay<0)<<1)+(deltax>0)];
        if (actor->movedir != (int) turnaround && P_TryWalk(actor))
            return;
    }

    // try other directions
    if (P_Random() > 200
        ||  abs(deltay)>abs(deltax))
    {
        tdir=d[1];
        d[1]=d[2];
        d[2]=tdir;
    }

    if (d[1]==turnaround)
        d[1]=DI_NODIR;
    if (d[2]==turnaround)
        d[2]=DI_NODIR;

    if (d[1]!=DI_NODIR)
    {
        actor->movedir = d[1];
        if (P_TryWalk(actor))
        {
            // either moved forward or attacked
            return;
        }
    }

    if (d[2]!=DI_NODIR)
    {
        actor->movedir =d[2];

        if (P_TryWalk(actor))
            return;
    }

    // there is no direct path to the player,
    // so pick another direction.
    if (olddir!=DI_NODIR)
    {
        actor->movedir =olddir;

        if (P_TryWalk(actor))
            return;
    }

    // randomly determine direction of search
    if (P_Random()&1) 	
    {
        for ( tdir=DI_EAST;
              tdir<=DI_SOUTHEAST;
              tdir++ )
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir =tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }
    else
    {
        for ( tdir=DI_SOUTHEAST;
              tdir != (DI_EAST-1);
              tdir-- )
        {
            if (tdir != (int) turnaround)
            {
                actor->movedir = tdir;

                if ( P_TryWalk(actor) )
                    return;
            }
        }
    }

    if (turnaround !=  DI_NODIR)
    {
        actor->movedir =turnaround;
        if ( P_TryWalk(actor) )
            return;
    }

    actor->movedir = DI_NODIR;	// can not move
}

//
// P_NewRandomDir
//
// villsa [STRIFE] new function
//
// haleyjd: Almost identical to the tail-end of P_NewChaseDir, this function 
// finds a purely random direction for an object to walk. Called from 
// A_RandomWalk.
//
// Shockingly similar to the RandomWalk pointer in Eternity :)
//
void P_NewRandomDir(mobj_t* actor)
{
    int dir = 0;

    // randomly determine direction of search
    if(P_Random() & 1)
    {
        for(dir = 0; dir < DI_NODIR; dir++)
        {
            if(dir != opposite[actor->movedir])
            {
                actor->movedir = dir;
                if(P_Random() & 1)
                {
                    if(P_TryWalk(actor))
                        break;
                }
            }
        }
    }
    else
    {
        dir = DI_SOUTHEAST;
        while(1)
        {
            // haleyjd 09/05/10: P_TryWalk -> P_Move, missing random code.
            if(dir != opposite[actor->movedir])
            {
                actor->movedir = dir;

                if(P_Move(actor))
                {
                    actor->movecount = P_Random() & 15;
                    return;
                }
            }

            if(--dir == -1)
            {
                if(opposite[actor->movedir] == DI_NODIR)
                {
                    actor->movedir = DI_NODIR;
                    return;
                }

                actor->movedir = opposite[actor->movedir];
                if(P_Move(actor))
                {
                    actor->movecount = P_Random() & 15;
                    return;
                }
                else
                {
                    actor->movedir = DI_NODIR;
                    return;
                }
            } // end if(--dir == -1)
        } // end while(1)
    } // end else
}

// haleyjd 09/05/10: Needed below.
extern void P_BulletSlope (mobj_t *mo);

#define LOCAL_MELEERANGE 64*FRACUNIT

//
// P_LookForPlayers
//
// If allaround is false, only look 180 degrees in front.
// Returns true if a player is targeted.
//
// [STRIFE]
// haleyjd 09/05/10: Modifications to support friendly units.
//
boolean
P_LookForPlayers
( mobj_t*	actor,
  boolean	allaround )
{
    int         c;
    int         stop;
    player_t*   player;
    sector_t*   sector;
    angle_t     an;
    fixed_t     dist;
    mobj_t  *   master = players[actor->allegiance].mo;

    // haleyjd 09/05/10: handle Allies
    if(actor->flags & MF_ALLY)
    {
        // Deathmatch: support team behavior for Rebels.
        if(netgame)
        {
            // Rebels adopt the allied player's target if it is not of the same
            // allegiance. Other allies do it unconditionally.
            if(master && master->target && 
               (master->target->type != MT_REBEL1 ||
                master->target->allegiance != actor->allegiance))
            {
                actor->target = master->target;
            }
            else
            {
                P_BulletSlope(actor);

                // Clear target if nothing is visible, or if the target is a
                // friendly Rebel or the allied player.
                if(!linetarget ||
                    actor->target->target == MT_REBEL1 &&
                    actor->target->allegiance == actor->allegiance ||
                    actor->target == master)
                {
                    actor->target = NULL;
                    return false;
                }
            }
        }
        else
        {
            // Single-player: Adopt any non-allied player target.
            if(master && master->target && !(master->target->flags & MF_ALLY))
            {
                actor->target = master->target;
                return true;
            }

            P_BulletSlope(actor);

            // Clear target if nothing is visible, or if the target is an ally.
            if(!linetarget || actor->target->flags & MF_ALLY)
            {
                actor->target = NULL;
                return false;
            }
        }

        return true;
    }

    sector = actor->subsector->sector;

    c = 0;
    stop = (actor->lastlook-1)&3;

    for ( ; ; actor->lastlook = (actor->lastlook+1)&3 )
    {
        if (!playeringame[actor->lastlook])
            continue;

        if (c++ == 2
            || actor->lastlook == stop)
        {
            // done looking
            return false;	
        }

        player = &players[actor->lastlook];

        if (player->health <= 0)
            continue;           // dead

        if (!P_CheckSight (actor, player->mo))
            continue;           // out of sight

        if (!allaround)
        {
            an = R_PointToAngle2(actor->x,
                                 actor->y, 
                                 player->mo->x,
                                 player->mo->y) - actor->angle;

            if (an > ANG90 && an < ANG270)
            {
                dist = P_AproxDistance (player->mo->x - actor->x,
                    player->mo->y - actor->y);
                // if real close, react anyway
                if (dist > LOCAL_MELEERANGE) // haleyjd: ......
                    continue;       // behind back
            }
        }

        actor->target = player->mo;
        return true;
    }

    return false;
}

// haleyjd 09/05/10: [STRIFE] Removed A_KeenDie

//
// ACTION ROUTINES
//

//
// A_Look
// Stay in state until a player is sighted.
//
// [STRIFE]
// haleyjd 09/05/10: Adjusted for allies, Inquisitors, etc.
//
void A_Look (mobj_t* actor)
{
    mobj_t*         targ;

    actor->threshold = 0;       // any shot will wake up
    targ = actor->subsector->sector->soundtarget;

    if (targ
        && (targ->flags & MF_SHOOTABLE) )
    {
        // [STRIFE] Allies wander when they call this.
        if(actor->flags & MF_ALLY)
            A_RandomWalk(actor);
        else
        {
            actor->target = targ;

            if ( actor->flags & MF_AMBUSH )
            {
                if (P_CheckSight (actor, actor->target))
                    goto seeyou;
            }
            else
                goto seeyou;
        }
    }

    // haleyjd 09/05/10: This is bizarre, as Rogue keeps using the GIVEQUEST flag
    // as a parameter to control allaround look behavior. Did they just run out of
    // flags, or what? 
    // STRIFE-TODO: Needs serious verification.
    if (!P_LookForPlayers (actor, actor->flags & MF_GIVEQUEST) )
        return;

    // go into chase state
seeyou:
    if (actor->info->seesound)
    {
        int         sound   = actor->info->seesound;
        mobj_t *    emitter = actor;

        // [STRIFE] Removed DOOM random sounds.

        // [STRIFE] Only Inquisitors roar loudly here.
        if (actor->type == MT_INQUISITOR)
            emitter = NULL;

        S_StartSound (emitter, sound);
    }

    // [STRIFE] Set threshold (kinda odd as it's still set to 0 above...)
    actor->threshold = 20;

    P_SetMobjState (actor, actor->info->seestate);
}

//
// A_RandomWalk
//
// [STRIFE] New function.
// haleyjd 09/05/10: Action routine used to meander about.
//
void A_RandomWalk(mobj_t* actor)
{
    // Standing actors do not wander.
    if(actor->flags & MF_STAND)
        return;

    if(actor->reactiontime)
        actor->reactiontime--; // count down reaction time
    else
    {
        // turn to a new angle
        if(actor->movedir < DI_NODIR)
        {
            int delta;

            actor->angle &= (7 << 29);
            delta = actor->angle - (actor->movedir << 29);

            if(delta < 0)
                actor->angle += ANG90/2;
            else if(delta > 0)
                actor->angle -= ANG90/2;
        }

        // try moving
        if(--actor->movecount < 0 || !P_Move(actor))
        {
            P_NewRandomDir(actor);
            actor->movecount += 5;
        }
    }
}

//
// A_FriendLook
//
// [STRIFE] New function
// haleyjd 09/05/10: Action function used mostly by mundane characters such as
// peasants.
//
void A_FriendLook(mobj_t* actor)
{
    mobj_t *soundtarget = actor->subsector->sector->soundtarget;

    actor->threshold = 0;

    if(soundtarget && soundtarget->flags & MF_SHOOTABLE)
    {
        // Handle allies, except on maps 3 and 34 (Front Base/Movement Base)
        if((actor->flags & MF_ALLY) == (soundtarget->flags & MF_ALLY) &&
            gamemap != 3 && gamemap != 34)
        {
            // STRIFE-TODO: Needs serious verification.
            if(P_LookForPlayers(actor, actor->flags & MF_GIVEQUEST))
            {
                P_SetMobjState(actor, actor->info->seestate);
                actor->flags |= MF_INCOMBAT;
                return;
            }
        }
        else
        {
            actor->target = soundtarget;

            if(!(actor->flags & MF_AMBUSH) || P_CheckSight(actor, actor->target))
            {
                actor->threshold = 10;
                P_SetMobjState(actor, actor->info->seestate);
                return;
            }
        }
    }

    // do some idle animation
    if(P_Random() < 30)
        P_SetMobjState(actor, actor->info->spawnstate + 1 + (P_Random() & 1));

    // wander around a bit
    if(!(actor->flags & MF_STAND) && P_Random() < 40)
        P_SetMobjState(actor, actor->info->spawnstate + 3);
}

//
// A_Listen
//
// [STRIFE] New function
// haleyjd 09/05/10: Action routine used to strictly listen for a target.
//
void A_Listen(mobj_t* actor)
{
    mobj_t *soundtarget;

    actor->threshold = 0;

    soundtarget = actor->subsector->sector->soundtarget;

    if(soundtarget && soundtarget->flags & MF_SHOOTABLE)
    {
        if(actor->flags & MF_ALLY != soundtarget->flags & MF_ALLY)
        {
            actor->target = soundtarget;

            if(!(actor->flags & MF_AMBUSH) || P_CheckSight(actor, actor->target))
            {
                if(actor->info->seesound)
                    S_StartSound(actor, actor->info->seesound);

                actor->threshold = 10;

                P_SetMobjState(actor, actor->info->seestate);
            }
        }
    }
}


//
// A_Chase
// Actor has a melee attack,
// so it tries to close as fast as possible
//
// haleyjd 09/05/10: [STRIFE] Various minor changes
//
void A_Chase (mobj_t*	actor)
{
    int         delta;

    if (actor->reactiontime)
        actor->reactiontime--;

    // modify target threshold
    if  (actor->threshold)
    {
        if (!actor->target
            || actor->target->health <= 0)
        {
            actor->threshold = 0;
        }
        else
            actor->threshold--;
    }
    
    // turn towards movement direction if not there yet
    if (actor->movedir < 8)
    {
        actor->angle &= (7<<29);
        delta = actor->angle - (actor->movedir << 29);

        if (delta > 0)
            actor->angle -= ANG90/2;
        else if (delta < 0)
            actor->angle += ANG90/2;
    }

    if (!actor->target
        || !(actor->target->flags&MF_SHOOTABLE))
    {
        // look for a new target
        if (P_LookForPlayers(actor,true))
            return; 	// got a new target

        P_SetMobjState (actor, actor->info->spawnstate);
        return;
    }
    
    // do not attack twice in a row
    if (actor->flags & MF_JUSTATTACKED)
    {
        actor->flags &= ~MF_JUSTATTACKED;
        // [STRIFE] Checks only against fastparm, not gameskill == 5
        if (!fastparm)
            P_NewChaseDir (actor);
        return;
    }
    
    // check for melee attack
    if (actor->info->meleestate
        && P_CheckMeleeRange (actor))
    {
        if (actor->info->attacksound)
            S_StartSound (actor, actor->info->attacksound);

        P_SetMobjState (actor, actor->info->meleestate);
        return;
    }
    
    // check for missile attack
    if (actor->info->missilestate)
    {
        // [STRIFE] Checks only fastparm.
        if (!fastparm && actor->movecount)
        {
            goto nomissile;
        }

        if (!P_CheckMissileRange (actor))
            goto nomissile;

        P_SetMobjState (actor, actor->info->missilestate);

        // [STRIFE] Add INCOMBAT flag to disable dialog
        actor->flags |= (MF_INCOMBAT|MF_JUSTATTACKED);
        return;
    }

    // ?
nomissile:
    // possibly choose another target
    if (netgame
        && !actor->threshold
        && !P_CheckSight (actor, actor->target) )
    {
        if (P_LookForPlayers(actor,true))
            return;	// got a new target
    }
    
    // chase towards player
    if (--actor->movecount<0
        || !P_Move (actor))
    {
        P_NewChaseDir (actor);
    }

    // [STRIFE] Changes to active sound behavior:
    // * Significantly more frequent
    // * Acolytes have randomized wandering sounds

    // make active sound
    if (actor->info->activesound && P_Random () < 38)
    {
        if(actor->info->activesound >= sfx_agrac1 &&
           actor->info->activesound <= sfx_agrac4)
        {
            S_StartSound(actor, sfx_agrac1 + P_Random() % 4);
        }
        else
            S_StartSound (actor, actor->info->activesound);
    }
}


//
// A_FaceTarget
//
// [STRIFE]
// haleyjd 09/05/10: Modified handling for various visibility
// modifying flags.
//
void A_FaceTarget (mobj_t* actor)
{	
    if (!actor->target)
        return;

    actor->flags &= ~MF_AMBUSH;

    actor->angle = R_PointToAngle2 (actor->x,
                                    actor->y,
                                    actor->target->x,
                                    actor->target->y);

    if(actor->target->flags & MF_SHADOW)
    {
        // [STRIFE] increased SHADOW inaccuracy by a power of 2
        int t = P_Random();
        actor->angle += (t - P_Random()) << 22;
    }
    else if(actor->target->flags & MF_MVIS)
    {
        // [STRIFE] MVIS gives even worse aiming!
        int t = P_Random();
        actor->angle += (t - P_Random()) << 23;
    }
}


//
// A_PosAttack
//
void A_PosAttack (mobj_t* actor)
{
    int		angle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    angle = actor->angle;
    slope = P_AimLineAttack (actor, angle, MISSILERANGE);

    S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
    angle += (P_Random()-P_Random())<<20;
    damage = ((P_Random()%5)+1)*3;
    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_SPosAttack (mobj_t* actor)
{
    int		i;
    int		angle;
    int		bangle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;

    S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

    for (i=0 ; i<3 ; i++)
    {
	angle = bangle + ((P_Random()-P_Random())<<20);
	damage = ((P_Random()%5)+1)*3;
	P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
    }
}

void A_CPosAttack (mobj_t* actor)
{
    int		angle;
    int		bangle;
    int		damage;
    int		slope;
	
    if (!actor->target)
	return;

    S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
    A_FaceTarget (actor);
    bangle = actor->angle;
    slope = P_AimLineAttack (actor, bangle, MISSILERANGE);

    angle = bangle + ((P_Random()-P_Random())<<20);
    damage = ((P_Random()%5)+1)*3;
    P_LineAttack (actor, angle, MISSILERANGE, slope, damage);
}

void A_CPosRefire (mobj_t* actor)
{	
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if (P_Random () < 40)
	return;

    if (!actor->target
	|| actor->target->health <= 0
	|| !P_CheckSight (actor, actor->target) )
    {
	P_SetMobjState (actor, actor->info->seestate);
    }
}


void A_SpidRefire (mobj_t* actor)
{	
    // keep firing unless target got out of sight
    A_FaceTarget (actor);

    if (P_Random () < 10)
	return;

    if (!actor->target
	|| actor->target->health <= 0
	|| !P_CheckSight (actor, actor->target) )
    {
	P_SetMobjState (actor, actor->info->seestate);
    }
}

void A_BspiAttack (mobj_t *actor)
{	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);

    // launch a missile
    // villsa [STRIFE] unused
    //P_SpawnMissile (actor, actor->target, MT_ARACHPLAZ);
}


//
// A_TroopAttack
//
void A_TroopAttack (mobj_t* actor)
{
    int		damage;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
	S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
	damage = (P_Random()%8+1)*3;
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }

    
    // launch a missile
    // villsa [STRIFE] unused
    //P_SpawnMissile (actor, actor->target, MT_TROOPSHOT);
}


void A_SargAttack (mobj_t* actor)
{
    int		damage;

    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
	damage = ((P_Random()%10)+1)*4;
	P_DamageMobj (actor->target, actor, actor, damage);
    }
}

void A_HeadAttack (mobj_t* actor)
{
    int		damage;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    if (P_CheckMeleeRange (actor))
    {
	damage = (P_Random()%6+1)*10;
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }
    
    // launch a missile
    // villsa [STRIFE] unused
    //P_SpawnMissile (actor, actor->target, MT_HEADSHOT);
}

void A_CyberAttack (mobj_t* actor)
{	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    // villsa [STRIFE] unused
    //P_SpawnMissile (actor, actor->target, MT_ROCKET);
}


void A_BruisAttack (mobj_t* actor)
{
    int		damage;
	
    if (!actor->target)
	return;
		
    if (P_CheckMeleeRange (actor))
    {
	S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
	damage = (P_Random()%8+1)*10;
	P_DamageMobj (actor->target, actor, actor, damage);
	return;
    }
    
    // launch a missile
    // villsa [STRIFE] unused
    //P_SpawnMissile (actor, actor->target, MT_BRUISERSHOT);
}


//
// A_SkelMissile
//
void A_SkelMissile (mobj_t* actor)
{	
    // villsa [STRIFE] unused
   /* mobj_t*	mo;
	
    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
    actor->z += 16*FRACUNIT;	// so missile spawns higher
    mo = P_SpawnMissile (actor, actor->target, MT_TRACER);
    actor->z -= 16*FRACUNIT;	// back to normal

    mo->x += mo->momx;
    mo->y += mo->momy;
    mo->tracer = actor->target;*/
}

int	TRACEANGLE = 0xc000000;

void A_Tracer (mobj_t* actor)
{
    // villsa [STRIFE] TODO - update with strife version
/*    angle_t	exact;
    fixed_t	dist;
    fixed_t	slope;
    mobj_t*	dest;
    mobj_t*	th;
		
    if (gametic & 3)
	return;
    
    // spawn a puff of smoke behind the rocket		
    P_SpawnPuff (actor->x, actor->y, actor->z);
	
    th = P_SpawnMobj (actor->x-actor->momx,
		      actor->y-actor->momy,
		      actor->z, MT_SMOKE);
    
    th->momz = FRACUNIT;
    th->tics -= P_Random()&3;
    if (th->tics < 1)
	th->tics = 1;
    
    // adjust direction
    dest = actor->tracer;
	
    if (!dest || dest->health <= 0)
	return;
    
    // change angle	
    exact = R_PointToAngle2 (actor->x,
			     actor->y,
			     dest->x,
			     dest->y);

    if (exact != actor->angle)
    {
	if (exact - actor->angle > 0x80000000)
	{
	    actor->angle -= TRACEANGLE;
	    if (exact - actor->angle < 0x80000000)
		actor->angle = exact;
	}
	else
	{
	    actor->angle += TRACEANGLE;
	    if (exact - actor->angle > 0x80000000)
		actor->angle = exact;
	}
    }
	
    exact = actor->angle>>ANGLETOFINESHIFT;
    actor->momx = FixedMul (actor->info->speed, finecosine[exact]);
    actor->momy = FixedMul (actor->info->speed, finesine[exact]);
    
    // change slope
    dist = P_AproxDistance (dest->x - actor->x,
			    dest->y - actor->y);
    
    dist = dist / actor->info->speed;

    if (dist < 1)
	dist = 1;
    slope = (dest->z+40*FRACUNIT - actor->z) / dist;

    if (slope < actor->momz)
	actor->momz -= FRACUNIT/8;
    else
	actor->momz += FRACUNIT/8;*/
}


void A_SkelWhoosh (mobj_t*	actor)
{
    if (!actor->target)
	return;
    A_FaceTarget (actor);
    S_StartSound (actor,sfx_swish); // villsa [STRIFE] TODO - fix sounds
}

void A_SkelFist (mobj_t*	actor)
{
    int		damage;

    if (!actor->target)
	return;
		
    A_FaceTarget (actor);
	
    if (P_CheckMeleeRange (actor))
    {
	damage = ((P_Random()%10)+1)*6;
	S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
	P_DamageMobj (actor->target, actor, actor, damage);
    }
}



//
// PIT_VileCheck
// Detect a corpse that could be raised.
//
mobj_t*		corpsehit;
mobj_t*		vileobj;
fixed_t		viletryx;
fixed_t		viletryy;

boolean PIT_VileCheck (mobj_t*	thing)
{
    // villsa [STRIFE] unused
/*    int		maxdist;
    boolean	check;
	
    if (!(thing->flags & MF_CORPSE) )
	return true;	// not a monster
    
    if (thing->tics != -1)
	return true;	// not lying still yet
    
    // villsa [STRIFE] unused
    //if (thing->info->raisestate == S_NULL)
	//return true;	// monster doesn't have a raise state
    
    maxdist = thing->info->radius + mobjinfo[MT_VILE].radius;
	
    if ( abs(thing->x - viletryx) > maxdist
	 || abs(thing->y - viletryy) > maxdist )
	return true;		// not actually touching
		
    corpsehit = thing;
    corpsehit->momx = corpsehit->momy = 0;
    corpsehit->height <<= 2;
    check = P_CheckPosition (corpsehit, corpsehit->x, corpsehit->y);
    corpsehit->height >>= 2;

    if (!check)
	return true;		// doesn't fit here*/
		
    return false;		// got one, so stop checking
}



//
// A_VileChase
// Check for ressurecting a body
//
// villsa [STRIFE] TODO depcricate this later
void A_VileChase (mobj_t* actor)
{
/*    int			xl;
    int			xh;
    int			yl;
    int			yh;
    
    int			bx;
    int			by;

    mobjinfo_t*		info;
    mobj_t*		temp;
	
    if (actor->movedir != DI_NODIR)
    {
	// check for corpses to raise
	viletryx =
	    actor->x + actor->info->speed*xspeed[actor->movedir];
	viletryy =
	    actor->y + actor->info->speed*yspeed[actor->movedir];

	xl = (viletryx - bmaporgx - MAXRADIUS*2)>>MAPBLOCKSHIFT;
	xh = (viletryx - bmaporgx + MAXRADIUS*2)>>MAPBLOCKSHIFT;
	yl = (viletryy - bmaporgy - MAXRADIUS*2)>>MAPBLOCKSHIFT;
	yh = (viletryy - bmaporgy + MAXRADIUS*2)>>MAPBLOCKSHIFT;
	
	vileobj = actor;
	for (bx=xl ; bx<=xh ; bx++)
	{
	    for (by=yl ; by<=yh ; by++)
	    {
		// Call PIT_VileCheck to check
		// whether object is a corpse
		// that canbe raised.
		if (!P_BlockThingsIterator(bx,by,PIT_VileCheck))
		{
		    // got one!
		    temp = actor->target;
		    actor->target = corpsehit;
		    A_FaceTarget (actor);
		    actor->target = temp;
					
		    P_SetMobjState (actor, S_VILE_HEAL1);
		    S_StartSound (corpsehit, sfx_slop);
		    info = corpsehit->info;
		    
		    P_SetMobjState (corpsehit,info->raisestate);
		    corpsehit->height <<= 2;
		    corpsehit->flags = info->flags;
		    corpsehit->health = info->spawnhealth;
		    corpsehit->target = NULL;

		    return;
		}
	    }
	}
    }

    // Return to normal attack.
    A_Chase (actor);*/
}


//
// A_VileStart
//
void A_VileStart (mobj_t* actor)
{
    S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
}


//
// A_Fire
// Keep fire in front of player unless out of sight
//
void A_Fire (mobj_t* actor);

void A_StartFire (mobj_t* actor)
{
    S_StartSound(actor,sfx_swish);  // villsa [STRIFE] TODO - fix sounds
    A_Fire(actor);
}

void A_FireCrackle (mobj_t* actor)
{
    S_StartSound(actor,sfx_swish);  // villsa [STRIFE] TODO - fix sounds
    A_Fire(actor);
}

void A_Fire (mobj_t* actor)
{
    mobj_t*	dest;
    mobj_t*     target;
    unsigned	an;
		
    dest = actor->tracer;
    if (!dest)
	return;

    target = P_SubstNullMobj(actor->target);
		
    // don't move it if the vile lost sight
    if (!P_CheckSight (target, dest) )
	return;

    an = dest->angle >> ANGLETOFINESHIFT;

    P_UnsetThingPosition (actor);
    actor->x = dest->x + FixedMul (24*FRACUNIT, finecosine[an]);
    actor->y = dest->y + FixedMul (24*FRACUNIT, finesine[an]);
    actor->z = dest->z;
    P_SetThingPosition (actor);
}



//
// A_VileTarget
// Spawn the hellfire
//
void A_VileTarget (mobj_t*	actor)
{
    // villsa [STRIFE] unused
 /*   mobj_t*	fog;
	
    if (!actor->target)
	return;

    A_FaceTarget (actor);

    fog = P_SpawnMobj (actor->target->x,
		       actor->target->x,
		       actor->target->z, MT_FIRE);
    
    actor->tracer = fog;
    fog->target = actor;
    fog->tracer = actor->target;
    A_Fire (fog);*/
}




//
// A_VileAttack
//
void A_VileAttack (mobj_t* actor)
{	
    mobj_t*	fire;
    int		an;
	
    if (!actor->target)
	return;
    
    A_FaceTarget (actor);

    if (!P_CheckSight (actor, actor->target) )
	return;

    S_StartSound (actor, sfx_barexp);
    P_DamageMobj (actor->target, actor, actor, 20);
    actor->target->momz = 1000*FRACUNIT/actor->target->info->mass;
	
    an = actor->angle >> ANGLETOFINESHIFT;

    fire = actor->tracer;

    if (!fire)
	return;
		
    // move the fire between the vile and the player
    fire->x = actor->target->x - FixedMul (24*FRACUNIT, finecosine[an]);
    fire->y = actor->target->y - FixedMul (24*FRACUNIT, finesine[an]);	
    P_RadiusAttack (fire, actor, 70 );
}




//
// Mancubus attack,
// firing three missiles (bruisers)
// in three different directions?
// Doesn't look like it. 
//
#define	FATSPREAD	(ANG90/8)

void A_FatRaise (mobj_t *actor)
{
    A_FaceTarget (actor);
    S_StartSound (actor, sfx_swish);    // villsa [STRIFE] TODO - fix sounds
}


void A_FatAttack1 (mobj_t* actor)
{
    // villsa [STRIFE] unused
/*    mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);

    // Change direction  to ...
    actor->angle += FATSPREAD;
    target = P_SubstNullMobj(actor->target);
    P_SpawnMissile (actor, target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);*/
}

void A_FatAttack2 (mobj_t* actor)
{
    // villsa [STRIFE] unused
/*    mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);
    // Now here choose opposite deviation.
    actor->angle -= FATSPREAD;
    target = P_SubstNullMobj(actor->target);
    P_SpawnMissile (actor, target, MT_FATSHOT);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD*2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);*/
}

void A_FatAttack3 (mobj_t*	actor)
{
    // villsa [STRIFE] unused
 /*   mobj_t*	mo;
    mobj_t*     target;
    int		an;

    A_FaceTarget (actor);

    target = P_SubstNullMobj(actor->target);
    
    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle -= FATSPREAD/2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);

    mo = P_SpawnMissile (actor, target, MT_FATSHOT);
    mo->angle += FATSPREAD/2;
    an = mo->angle >> ANGLETOFINESHIFT;
    mo->momx = FixedMul (mo->info->speed, finecosine[an]);
    mo->momy = FixedMul (mo->info->speed, finesine[an]);*/
}


//
// SkullAttack
// Fly at the player like a missile.
//
#define	SKULLSPEED		(20*FRACUNIT)

void A_SkullAttack (mobj_t* actor)
{
/*    mobj_t*		dest;
    angle_t		an;
    int			dist;

    if (!actor->target)
	return;
		
    dest = actor->target;	
    actor->flags |= MF_SKULLFLY;

    S_StartSound (actor, actor->info->attacksound);
    A_FaceTarget (actor);
    an = actor->angle >> ANGLETOFINESHIFT;
    actor->momx = FixedMul (SKULLSPEED, finecosine[an]);
    actor->momy = FixedMul (SKULLSPEED, finesine[an]);
    dist = P_AproxDistance (dest->x - actor->x, dest->y - actor->y);
    dist = dist / SKULLSPEED;
    
    if (dist < 1)
	dist = 1;
    actor->momz = (dest->z+(dest->height>>1) - actor->z) / dist;*/
}


//
// A_PainShootSkull
// Spawn a lost soul and launch it at the target
//
void
A_PainShootSkull
( mobj_t*	actor,
  angle_t	angle )
{
    // villsa [STRIFE] unused
 /*   fixed_t	x;
    fixed_t	y;
    fixed_t	z;
    
    mobj_t*	newmobj;
    angle_t	an;
    int		prestep;
    int		count;
    thinker_t*	currentthinker;

    // count total number of skull currently on the level
    count = 0;

    currentthinker = thinkercap.next;
    while (currentthinker != &thinkercap)
    {
	if (   (currentthinker->function.acp1 == (actionf_p1)P_MobjThinker)
	    && ((mobj_t *)currentthinker)->type == MT_SKULL)
	    count++;
	currentthinker = currentthinker->next;
    }

    // if there are allready 20 skulls on the level,
    // don't spit another one
    if (count > 20)
	return;


    // okay, there's playe for another one
    an = angle >> ANGLETOFINESHIFT;
    
    prestep =
	4*FRACUNIT
	+ 3*(actor->info->radius + mobjinfo[MT_SKULL].radius)/2;
    
    x = actor->x + FixedMul (prestep, finecosine[an]);
    y = actor->y + FixedMul (prestep, finesine[an]);
    z = actor->z + 8*FRACUNIT;
		
    newmobj = P_SpawnMobj (x , y, z, MT_SKULL);

    // Check for movements.
    if (!P_TryMove (newmobj, newmobj->x, newmobj->y))
    {
	// kill it immediately
	P_DamageMobj (newmobj,actor,actor,10000);	
	return;
    }
		
    newmobj->target = actor->target;
    A_SkullAttack (newmobj);*/
}


//
// A_PainAttack
// Spawn a lost soul and launch it at the target
// 
void A_PainAttack (mobj_t* actor)
{
    if (!actor->target)
	return;

    A_FaceTarget (actor);
    A_PainShootSkull (actor, actor->angle);
}


void A_PainDie (mobj_t* actor)
{
    A_Fall (actor);
    A_PainShootSkull (actor, actor->angle+ANG90);
    A_PainShootSkull (actor, actor->angle+ANG180);
    A_PainShootSkull (actor, actor->angle+ANG270);
}






void A_Scream (mobj_t* actor)
{
    int		sound;
	
    switch (actor->info->deathsound)
    {
      case 0:
	return;
		
      case sfx_agrac1:  // villsa [STRIFE] TODO - fix sounds
      case sfx_agrac2:  // villsa [STRIFE] TODO - fix sounds
      case sfx_agrac3:  // villsa [STRIFE] TODO - fix sounds
	sound = sfx_agrac1 + P_Random ()%3; // villsa [STRIFE] TODO - fix sounds
	break;
	
      default:
	sound = actor->info->deathsound;
	break;
    }

    // Check for bosses.
    // villsa [STRIFE] TODO - replace with strife bosses
    /*if (actor->type==MT_SPIDER
	|| actor->type == MT_CYBORG)
    {
	// full volume
	S_StartSound (NULL, sound);
    }
    else*/
	S_StartSound (actor, sound);
}


void A_XScream (mobj_t* actor)
{
    S_StartSound (actor, sfx_slop);	
}

void A_Pain (mobj_t* actor)
{
    if (actor->info->painsound)
	S_StartSound (actor, actor->info->painsound);	
}



void A_Fall (mobj_t *actor)
{
    // actor is on ground, it can be walked over
    actor->flags &= ~MF_SOLID;

    // So change this if corpse objects
    // are meant to be obstacles.
}


//
// A_Explode
//
void A_Explode (mobj_t* thingy)
{
    P_RadiusAttack(thingy, thingy->target, 128);
}

// Check whether the death of the specified monster type is allowed
// to trigger the end of episode special action.
//
// This behavior changed in v1.9, the most notable effect of which
// was to break uac_dead.wad

static boolean CheckBossEnd(mobjtype_t motype)
{
    // villsa [STRIFE] TODO - update to strife version
    return 0;
 /*   if (gameversion < exe_ultimate)
    {
        if (gamemap != 8)
        {
            return false;
        }

        // Baron death on later episodes is nothing special.

        if (motype == MT_BRUISER && gameepisode != 1)
        {
            return false;
        }

        return true;
    }
    else
    {
        // New logic that appeared in Ultimate Doom.
        // Looks like the logic was overhauled while adding in the
        // episode 4 support.  Now bosses only trigger on their
        // specific episode.

	switch(gameepisode)
	{
            case 1:
                return gamemap == 8 && motype == MT_BRUISER;

            case 2:
                return gamemap == 8 && motype == MT_CYBORG;

            case 3:
                return gamemap == 8 && motype == MT_SPIDER;

	    case 4:
                return (gamemap == 6 && motype == MT_CYBORG)
                    || (gamemap == 8 && motype == MT_SPIDER);

            default:
                return gamemap == 8;
	}
    }*/
}

//
// A_BossDeath
// Possibly trigger special effects
// if on first boss level
//
void A_BossDeath (mobj_t* mo)
{
    thinker_t*	th;
    mobj_t*	mo2;
    line_t	junk;
    int		i;
		
    // villsa [STRIFE] TODO - update to strife version
 /*   if ( gamemode == commercial)
    {
	if (gamemap != 7)
	    return;
		
	if ((mo->type != MT_FATSO)
	    && (mo->type != MT_BABY))
	    return;
    }
    else
    {
        if (!CheckBossEnd(mo->type))
        {
            return;
        }
    }

    // make sure there is a player alive for victory
    for (i=0 ; i<MAXPLAYERS ; i++)
	if (playeringame[i] && players[i].health > 0)
	    break;
    
    if (i==MAXPLAYERS)
	return;	// no one left alive, so do not end game
    
    // scan the remaining thinkers to see
    // if all bosses are dead
    for (th = thinkercap.next ; th != &thinkercap ; th=th->next)
    {
	if (th->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;
	
	mo2 = (mobj_t *)th;
	if (mo2 != mo
	    && mo2->type == mo->type
	    && mo2->health > 0)
	{
	    // other boss not dead
	    return;
	}
    }
	
    // victory!
    if ( gamemode == commercial)
    {
	if (gamemap == 7)
	{
	    if (mo->type == MT_FATSO)
	    {
		junk.tag = 666;
		EV_DoFloor(&junk,lowerFloorToLowest);
		return;
	    }
	    
	    if (mo->type == MT_BABY)
	    {
		junk.tag = 667;
		EV_DoFloor(&junk,raiseToTexture);
		return;
	    }
	}
    }
    else
    {
	switch(gameepisode)
	{
	  case 1:
	    junk.tag = 666;
	    EV_DoFloor (&junk, lowerFloorToLowest);
	    return;
	    break;
	    
	  case 4:
	    switch(gamemap)
	    {
	      case 6:
		junk.tag = 666;
		EV_DoDoor (&junk, blazeOpen);
		return;
		break;
		
	      case 8:
		junk.tag = 666;
		EV_DoFloor (&junk, lowerFloorToLowest);
		return;
		break;
	    }
	}
    }
	
    G_ExitLevel (0);*/
}


void A_Hoof (mobj_t* mo)
{
    S_StartSound (mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
    A_Chase (mo);
}

void A_Metal (mobj_t* mo)
{
    S_StartSound (mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
    A_Chase (mo);
}

void A_BabyMetal (mobj_t* mo)
{
    S_StartSound (mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
    A_Chase (mo);
}

void
A_OpenShotgun2
( player_t*	player,
  pspdef_t*	psp )
{
    S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
}

void
A_LoadShotgun2
( player_t*	player,
  pspdef_t*	psp )
{
    S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
}

void
A_ReFire
( player_t*	player,
  pspdef_t*	psp );

void
A_CloseShotgun2
( player_t*	player,
  pspdef_t*	psp )
{
    S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
    A_ReFire(player,psp);
}



mobj_t*		braintargets[32];
int		numbraintargets;
int		braintargeton = 0;

void A_BrainAwake (mobj_t* mo)
{
    // villsa [STRIFE] unused
 /*   thinker_t*	thinker;
    mobj_t*	m;
	
    // find all the target spots
    numbraintargets = 0;
    braintargeton = 0;
	
    thinker = thinkercap.next;
    for (thinker = thinkercap.next ;
	 thinker != &thinkercap ;
	 thinker = thinker->next)
    {
	if (thinker->function.acp1 != (actionf_p1)P_MobjThinker)
	    continue;	// not a mobj

	m = (mobj_t *)thinker;

	if (m->type == MT_BOSSTARGET )
	{
	    braintargets[numbraintargets] = m;
	    numbraintargets++;
	}
    }
	
    S_StartSound (NULL,sfx_swish);  // villsa [STRIFE] TODO - fix sounds*/
}


void A_BrainPain (mobj_t*	mo)
{
    S_StartSound (NULL,sfx_swish);  // villsa [STRIFE] TODO - fix sounds
}


// villsa [STRIFE] TODO - depcricate this later
void A_BrainScream (mobj_t*	mo)
{
/*    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    for (x=mo->x - 196*FRACUNIT ; x< mo->x + 320*FRACUNIT ; x+= FRACUNIT*8)
    {
	y = mo->y - 320*FRACUNIT;
	z = 128 + P_Random()*2*FRACUNIT;
	th = P_SpawnMobj (x,y,z, MT_ROCKET);
	th->momz = P_Random()*512;

	P_SetMobjState (th, S_BRAINEXPLODE1);

	th->tics -= P_Random()&7;
	if (th->tics < 1)
	    th->tics = 1;
    }
	
    S_StartSound (NULL,sfx_swish);  // villsa [STRIFE] TODO - fix sounds*/
}


// villsa [STRIFE] TODO - depcricate this later
void A_BrainExplode (mobj_t* mo)
{
/*    int		x;
    int		y;
    int		z;
    mobj_t*	th;
	
    x = mo->x + (P_Random () - P_Random ())*2048;
    y = mo->y;
    z = 128 + P_Random()*2*FRACUNIT;
    th = P_SpawnMobj (x,y,z, MT_ROCKET);
    th->momz = P_Random()*512;

    P_SetMobjState (th, S_BRAINEXPLODE1);

    th->tics -= P_Random()&7;
    if (th->tics < 1)
	th->tics = 1;*/
}


void A_BrainDie (mobj_t*	mo)
{
    G_ExitLevel (0);
}

void A_BrainSpit (mobj_t*	mo)
{
    // villsa [STRIFE] unused
 /*   mobj_t*	targ;
    mobj_t*	newmobj;
    
    static int	easy = 0;
	
    easy ^= 1;
    if (gameskill <= sk_easy && (!easy))
	return;
		
    // shoot a cube at current target
    targ = braintargets[braintargeton];
    braintargeton = (braintargeton+1)%numbraintargets;

    // spawn brain missile
    newmobj = P_SpawnMissile (mo, targ, MT_SPAWNSHOT);
    newmobj->target = targ;
    newmobj->reactiontime =
	((targ->y - mo->y)/newmobj->momy) / newmobj->state->tics;

    S_StartSound(NULL, sfx_swish);  // villsa [STRIFE] TODO - fix sounds*/
}



void A_SpawnFly (mobj_t* mo);

// travelling cube sound
void A_SpawnSound (mobj_t* mo)	
{
    S_StartSound (mo,sfx_swish);    // villsa [STRIFE] TODO - fix sounds
    A_SpawnFly(mo);
}

void A_SpawnFly (mobj_t* mo)
{
    // villsa [STRIFE] unused
/*    mobj_t*	newmobj;
    mobj_t*	fog;
    mobj_t*	targ;
    int		r;
    mobjtype_t	type;
	
    if (--mo->reactiontime)
	return;	// still flying
	
    targ = P_SubstNullMobj(mo->target);

    // First spawn teleport fog.
    fog = P_SpawnMobj (targ->x, targ->y, targ->z, MT_SPAWNFIRE);
    S_StartSound (fog, sfx_telept);

    // Randomly select monster to spawn.
    r = P_Random ();

    // Probability distribution (kind of :),
    // decreasing likelihood.
    if ( r<50 )
	type = MT_TROOP;
    else if (r<90)
	type = MT_SERGEANT;
    else if (r<120)
	type = MT_SHADOWS;
    else if (r<130)
	type = MT_PAIN;
    else if (r<160)
	type = MT_HEAD;
    else if (r<162)
	type = MT_VILE;
    else if (r<172)
	type = MT_UNDEAD;
    else if (r<192)
	type = MT_BABY;
    else if (r<222)
	type = MT_FATSO;
    else if (r<246)
	type = MT_KNIGHT;
    else
	type = MT_BRUISER;		

    newmobj	= P_SpawnMobj (targ->x, targ->y, targ->z, type);
    if (P_LookForPlayers (newmobj, true) )
	P_SetMobjState (newmobj, newmobj->info->seestate);
	
    // telefrag anything in this spot
    P_TeleportMove (newmobj, newmobj->x, newmobj->y);

    // remove self (i.e., cube).
    P_RemoveMobj (mo);*/
}



void A_PlayerScream (mobj_t* mo)
{
    // Default death sound.
    int		sound = sfx_pldeth;
	
    if ( (gamemode == commercial)
	&& 	(mo->health < -50))
    {
	// IF THE PLAYER DIES
	// LESS THAN -50% WITHOUT GIBBING
	sound = sfx_swish;  // villsa [STRIFE] TODO - fix sounds
    }
    
    S_StartSound (mo, sound);
}


void A_PeasantPunch(mobj_t* actor)
{

}

void A_ReavShoot(mobj_t* actor)
{

}

void A_BulletAttack(mobj_t* actor)
{

}

void A_CheckTargetVisible(mobj_t* actor)
{

}

void A_SentinelAttack(mobj_t* actor)
{

}

void A_StalkerThink(mobj_t* actor)
{

}

void A_StalkerSetLook(mobj_t* actor)
{

}

void A_StalkerDrop(mobj_t* actor)
{

}

void A_StalkerScratch(mobj_t* actor)
{

}

void A_FloatWeave(mobj_t* actor)
{

}

void A_ReavAttack(mobj_t* actor)
{

}

void A_TemplarMauler(mobj_t* actor)
{

}

void A_CrusaderAttack(mobj_t* actor)
{

}

void A_CrusaderLeft(mobj_t* actor)
{

}

void A_CrusaderRight(mobj_t* actor)
{

}

void A_CheckTargetVisible2(mobj_t* actor)
{

}

void A_InqFlyCheck(mobj_t* actor)
{

}

void A_InqGrenade(mobj_t* actor)
{

}

void A_InqTakeOff(mobj_t* actor)
{

}

void A_InqFly(mobj_t* actor)
{

}

void A_FireSigilWeapon(mobj_t* actor)
{

}

void A_ProgrammerAttack(mobj_t* actor)
{

}

void A_Sigil_A_Action(mobj_t* actor)
{

}

void A_SpectreEAttack(mobj_t* actor)
{

}

void A_SpectreCAttack(mobj_t* actor)
{

}

void A_AlertSpectreC(mobj_t* actor)
{

}

void A_Sigil_E_Action(mobj_t* actor)
{

}

void A_SigilTrail(mobj_t* actor)
{

}

void A_SpectreDAttack(mobj_t* actor)
{

}

void A_FireSigilEOffshoot(mobj_t* actor)
{

}

void A_ShadowOff(mobj_t* actor)
{

}

void A_ModifyVisibility(mobj_t* actor)
{

}

void A_ShadowOn(mobj_t* actor)
{

}

void A_SetTLOptions(mobj_t* actor)
{

}

void A_BossMeleeAtk(mobj_t* actor)
{

}

void A_BishopAttack(mobj_t* actor)
{

}

void A_FireHookShot(mobj_t* actor)
{

}

void A_FireChainShot(mobj_t* actor)
{

}

void A_MissileSmoke(mobj_t* actor)
{

}

void A_SpawnSparkPuff(mobj_t* actor)
{

}

void A_ProgrammerMelee(mobj_t* actor)
{

}

void A_PeasantCrash(mobj_t* actor)
{

}

void A_HideZombie(mobj_t* actor)
{

}

void A_MerchantPain(mobj_t* actor)
{

}

void A_ProgrammerDie(mobj_t* actor)
{

}

void A_InqTossArm(mobj_t* actor)
{

}

void A_SpawnSpectreB(mobj_t* actor)
{

}

void A_SpawnSpectreD(mobj_t* actor)
{

}

void A_SpawnSpectreE(mobj_t* actor)
{

}

void A_SpawnEntity(mobj_t* actor)
{

}

void A_EntityDeath(mobj_t* actor)
{

}

void A_SpawnZombie(mobj_t* actor)
{

}

void A_ZombieInSpecialSector(mobj_t* actor)
{

}

void A_CrystalExplode(mobj_t* actor)
{

}

void A_DeathMsg(mobj_t* actor)
{

}

void A_ExtraLightOff(mobj_t* actor)
{

}

void A_DeathExplode4(mobj_t* actor)
{

}

void A_DeathExplode5(mobj_t* actor)
{

}

void A_DeathExplode1(mobj_t* actor)
{

}

void A_DeathExplode2(mobj_t* actor)
{

}

void A_DeathExplode3(mobj_t* actor)
{

}

void A_RaiseAlarm(mobj_t* actor)
{

}

//
// A_MissileTick
// villsa [STRIFE] - new codepointer
//

void A_MissileTick(mobj_t* actor)
{
    int r = actor->reactiontime--;

    if(r - 1 <= 0)
    {
        P_ExplodeMissile(actor);
        actor->flags &= ~MF_MISSILE;
    }
}

void A_SpawnGrenadeFire(mobj_t* actor)
{

}

void A_NodeChunk(mobj_t* actor)
{

}

void A_HeadChunk(mobj_t* actor)
{

}

void A_BurnSpread(mobj_t* actor)
{

}

void A_AcolyteSpecial(mobj_t* actor)
{

}

void A_InqChase(mobj_t* actor)
{

}

void A_StalkerChase(mobj_t* actor)
{

}

void A_TeleportBeacon(mobj_t* actor)
{

}

void A_BodyParts(mobj_t* actor)
{

}

void A_ClaxonBlare(mobj_t* actor)
{

}

void A_ActiveSound(mobj_t* actor)
{

}

void A_ClearSoundTarget(mobj_t* actor)
{

}

void A_DropBurnFlesh(mobj_t* actor)
{

}

void A_FlameDeath(mobj_t* actor)
{

}

void A_ClearForceField(mobj_t* actor)
{

}
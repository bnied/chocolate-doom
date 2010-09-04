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
//	Weapon sprite animation, weapon objects.
//	Action functions for weapons.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"
#include "d_event.h"

#include "deh_misc.h"

#include "m_random.h"
#include "p_local.h"
#include "s_sound.h"

// State.
#include "doomstat.h"

// Data.
#include "sounds.h"

#include "p_pspr.h"

#define LOWERSPEED		FRACUNIT*6
#define RAISESPEED		FRACUNIT*6

#define WEAPONBOTTOM	128*FRACUNIT
#define WEAPONTOP		32*FRACUNIT



//
// P_SetPsprite
//
void
P_SetPsprite
( player_t*	player,
  int		position,
  statenum_t	stnum ) 
{
    pspdef_t*	psp;
    state_t*	state;
	
    psp = &player->psprites[position];
	
    do
    {
	if (!stnum)
	{
	    // object removed itself
	    psp->state = NULL;
	    break;	
	}
	
	state = &states[stnum];
	psp->state = state;
	psp->tics = state->tics;	// could be 0

        // villsa [STRIFE] unused
	/*if (state->misc1)
	{
	    // coordinate set
	    psp->sx = state->misc1 << FRACBITS;
	    psp->sy = state->misc2 << FRACBITS;
	}*/
	
	// Call action routine.
	// Modified handling.
	if (state->action.acp2)
	{
	    state->action.acp2(player, psp);
	    if (!psp->state)
		break;
	}
	
	stnum = psp->state->nextstate;
	
    } while (!psp->tics);
    // an initial state of 0 could cycle through
}



//
// P_CalcSwing
//	
fixed_t		swingx;
fixed_t		swingy;

void P_CalcSwing (player_t*	player)
{
    fixed_t	swing;
    int		angle;
	
    // OPTIMIZE: tablify this.
    // A LUT would allow for different modes,
    //  and add flexibility.

    swing = player->bob;

    angle = (FINEANGLES/70*leveltime)&FINEMASK;
    swingx = FixedMul ( swing, finesine[angle]);

    angle = (FINEANGLES/70*leveltime+FINEANGLES/2)&FINEMASK;
    swingy = -FixedMul ( swingx, finesine[angle]);
}



//
// P_BringUpWeapon
// Starts bringing the pending weapon up
// from the bottom of the screen.
// Uses player
//
void P_BringUpWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (player->pendingweapon == wp_nochange)
	player->pendingweapon = player->readyweapon;
		
    // villsa [STRIFE] unused
    /*if (player->pendingweapon == wp_chainsaw)
	S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds*/
		
    newstate = weaponinfo[player->pendingweapon].upstate;

    player->pendingweapon = wp_nochange;
    player->psprites[ps_weapon].sy = WEAPONBOTTOM;

    P_SetPsprite (player, ps_weapon, newstate);
}

//
// P_CheckAmmo
// Returns true if there is enough ammo to shoot.
// If not, selects the next weapon to use.
//
boolean P_CheckAmmo (player_t* player)
{
    ammotype_t		ammo;
    int			count;

    // villsa [STRIFE] TODO - temp until this function is cleaned up
    return true;

    ammo = weaponinfo[player->readyweapon].ammo;

    // Minimal amount for one shot varies.
    // villsa [STRIFE] unused
    /*if (player->readyweapon == wp_bfg)
	count = deh_bfg_cells_per_shot;
    else if (player->readyweapon == wp_supershotgun)
	count = 2;	// Double barrel.
    else*/
	count = 1;	// Regular.

    // Some do not need ammunition anyway.
    // Return if current ammunition sufficient.
    if (ammo == am_noammo || player->ammo[ammo] >= count)
	return true;
		
    // Out of ammo, pick a weapon to change to.
    // Preferences are set here.
    // villsa [STRIFE] TODO - BEWARE, NO WEAPON PREFERENCE, MUST FIX!
    /*do
    {
	if (player->weaponowned[wp_plasma]
	    && player->ammo[am_cell]
	    && (gamemode != shareware) )
	{
	    player->pendingweapon = wp_plasma;
	}
	else if (player->weaponowned[wp_supershotgun] 
		 && player->ammo[am_shell]>2
		 && (gamemode == commercial) )
	{
	    player->pendingweapon = wp_supershotgun;
	}
	else if (player->weaponowned[wp_chaingun]
		 && player->ammo[am_clip])
	{
	    player->pendingweapon = wp_chaingun;
	}
	else if (player->weaponowned[wp_shotgun]
		 && player->ammo[am_shell])
	{
	    player->pendingweapon = wp_shotgun;
	}
	else if (player->ammo[am_clip])
	{
	    player->pendingweapon = wp_pistol;
	}
	else if (player->weaponowned[wp_chainsaw])
	{
	    player->pendingweapon = wp_chainsaw;
	}
	else if (player->weaponowned[wp_missile]
		 && player->ammo[am_misl])
	{
	    player->pendingweapon = wp_missile;
	}
	else if (player->weaponowned[wp_bfg]
		 && player->ammo[am_cell]>40
		 && (gamemode != shareware) )
	{
	    player->pendingweapon = wp_bfg;
	}
	else
	{
	    // If everything fails.
	    player->pendingweapon = wp_fist;
	}
	
    } while (player->pendingweapon == wp_nochange);*/

    // Now set appropriate weapon overlay.
    P_SetPsprite (player,
		  ps_weapon,
		  weaponinfo[player->readyweapon].downstate);

    return false;	
}


//
// P_FireWeapon.
//
void P_FireWeapon (player_t* player)
{
    statenum_t	newstate;
	
    if (!P_CheckAmmo (player))
	return;
	
    // villsa [STRIFE] TODO - verify
    P_SetMobjState (player->mo, S_PLAY_05);
    newstate = weaponinfo[player->readyweapon].atkstate;
    P_SetPsprite (player, ps_weapon, newstate);
    P_NoiseAlert (player->mo, player->mo);
}



//
// P_DropWeapon
// Player died, so put the weapon away.
//
void P_DropWeapon (player_t* player)
{
    P_SetPsprite (player,
		  ps_weapon,
		  weaponinfo[player->readyweapon].downstate);
}



//
// A_WeaponReady
// The player can fire the weapon
// or change to another weapon at this time.
// Follows after getting weapon up,
// or after previous attack/fire sequence.
//
void
A_WeaponReady
( player_t*	player,
  pspdef_t*	psp )
{	
    statenum_t	newstate;
    int		angle;
    
    // get out of attack state
    // villsa [STRIFE] TODO - verify
    if (player->mo->state == &states[S_PLAY_05]
	|| player->mo->state == &states[S_PLAY_06] )
    {
	P_SetMobjState (player->mo, S_PLAY_00);
    }
    
        // villsa [STRIFE] unused
    /*if (player->readyweapon == wp_chainsaw
	&& psp->state == &states[S_SAW])
    {
	S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
    }*/
    
    // check for change
    //  if player is dead, put the weapon away
    if (player->pendingweapon != wp_nochange || !player->health)
    {
	// change weapon
	//  (pending weapon should allready be validated)
	newstate = weaponinfo[player->readyweapon].downstate;
	P_SetPsprite (player, ps_weapon, newstate);
	return;	
    }
    
    // check for fire
    //  the missile launcher and bfg do not auto fire
    if (player->cmd.buttons & BT_ATTACK)
    {
        
	if ( !player->attackdown
	     /*|| (player->readyweapon != wp_missile    // villsa [STRIFE] unused?
		 && player->readyweapon != wp_bfg)*/ )
	{
	    player->attackdown = true;
	    P_FireWeapon (player);		
	    return;
	}
    }
    else
	player->attackdown = false;
    
    // bob the weapon based on movement speed
    angle = (128*leveltime)&FINEMASK;
    psp->sx = FRACUNIT + FixedMul (player->bob, finecosine[angle]);
    angle &= FINEANGLES/2-1;
    psp->sy = WEAPONTOP + FixedMul (player->bob, finesine[angle]);
}



//
// A_ReFire
// The player can re-fire the weapon
// without lowering it entirely.
//
void A_ReFire
( player_t*	player,
  pspdef_t*	psp )
{
    
    // check for fire
    //  (if a weaponchange is pending, let it go through instead)
    if ( (player->cmd.buttons & BT_ATTACK) 
	 && player->pendingweapon == wp_nochange
	 && player->health)
    {
	player->refire++;
	P_FireWeapon (player);
    }
    else
    {
	player->refire = 0;
	P_CheckAmmo (player);
    }
}


void
A_CheckReload
( player_t*	player,
  pspdef_t*	psp )
{
    P_CheckAmmo (player);
#if 0
    if (player->ammo[am_shell]<2)
	P_SetPsprite (player, ps_weapon, S_DSNR1);
#endif
}



//
// A_Lower
// Lowers current weapon,
//  and changes weapon at bottom.
//
void
A_Lower
( player_t*	player,
  pspdef_t*	psp )
{	
    psp->sy += LOWERSPEED;

    // Is already down.
    if (psp->sy < WEAPONBOTTOM )
	return;

    // Player is dead.
    if (player->playerstate == PST_DEAD)
    {
	psp->sy = WEAPONBOTTOM;

	// don't bring weapon back up
	return;		
    }
    
    // The old weapon has been lowered off the screen,
    // so change the weapon and start raising it
    if (!player->health)
    {
	// Player is dead, so keep the weapon off screen.
	P_SetPsprite (player,  ps_weapon, S_NULL);
	return;	
    }
	
    player->readyweapon = player->pendingweapon; 

    P_BringUpWeapon (player);
}


//
// A_Raise
//
void
A_Raise
( player_t*	player,
  pspdef_t*	psp )
{
    statenum_t	newstate;
	
    psp->sy -= RAISESPEED;

    if (psp->sy > WEAPONTOP )
	return;
    
    psp->sy = WEAPONTOP;
    
    // The weapon has been raised all the way,
    //  so change to the ready state.
    newstate = weaponinfo[player->readyweapon].readystate;

    P_SetPsprite (player, ps_weapon, newstate);
}



//
// A_GunFlash
//
void
A_GunFlash
( player_t*	player,
  pspdef_t*	psp ) 
{
    // villsa [STRIFE] TODO - verify
    P_SetMobjState (player->mo, S_PLAY_06);
    P_SetPsprite (player,ps_flash,weaponinfo[player->readyweapon].flashstate);
}



//
// WEAPON ATTACKS
//


//
// A_Punch
//
void
A_Punch
( player_t*	player,
  pspdef_t*	psp ) 
{
    angle_t	angle;
    int		damage;
    int		slope;
	
    damage = (P_Random ()%10+1)<<1;

    if (player->powers[pw_strength])	
	damage *= 10;

    angle = player->mo->angle;
    angle += (P_Random()-P_Random())<<18;
    slope = P_AimLineAttack (player->mo, angle, MELEERANGE);
    P_LineAttack (player->mo, angle, MELEERANGE, slope, damage);

    // turn to face target
    if (linetarget)
    {
	S_StartSound (player->mo, sfx_swish);   // villsa [STRIFE] TODO - fix sounds
	player->mo->angle = R_PointToAngle2 (player->mo->x,
					     player->mo->y,
					     linetarget->x,
					     linetarget->y);
    }
}


// Doom does not check the bounds of the ammo array.  As a result,
// it is possible to use an ammo type > 4 that overflows into the
// maxammo array and affects that instead.  Through dehacked, for
// example, it is possible to make a weapon that decreases the max
// number of ammo for another weapon.  Emulate this.

static void DecreaseAmmo(player_t *player, int ammonum, int amount)
{
    if (ammonum < NUMAMMO)
    {
        player->ammo[ammonum] -= amount;
    }
    else
    {
        player->maxammo[ammonum - NUMAMMO] -= amount;
    }
}


//
// A_FireFlameThrower
// villsa [STRIFE] completly new compared to the original
//

void A_FireFlameThrower(player_t* player, pspdef_t* psp) 
{
    mobj_t* mo;

    P_SetMobjState(player->mo, S_PLAY_06);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    player->mo->angle += (P_Random() - P_Random()) << 18;

    mo = P_SpawnPlayerMissile(player->mo, MT_SFIREBALL);
    mo->momz += (5*FRACUNIT);
}

//
// A_FireMissile
// villsa [STRIFE] completly new compared to the original
//

void A_FireMissile(player_t* player, pspdef_t* psp) 
{
    angle_t an;
    mobj_t* mo;

    an = player->mo->angle;
    player->mo->angle += (P_Random() - P_Random())<<(19 - (player->accuracy * 4 / 100));
    P_SetMobjState(player->mo, S_PLAY_06);
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_MINIMISSLE);
    player->mo->angle = an;
}

//
// A_FireMauler2
// villsa [STRIFE] - new codepointer
//

void A_FireMauler2(player_t* player, pspdef_t* pspr)
{
    P_SetMobjState(player->mo, S_PLAY_06);
    P_DamageMobj(player->mo, player->mo, NULL, 20);
    player->ammo[weaponinfo[player->readyweapon].ammo] -= 30;
    P_SpawnPlayerMissile(player->mo, MT_TORPEDO);
    P_Thrust(player, player->mo->angle + ANG180, 512000);
}

//
// A_FireGrenade
// villsa [STRIFE] - new codepointer
//

void A_FireGrenade(player_t* player, pspdef_t* pspr)
{
    mobjtype_t type;
    mobj_t* mo;
    state_t* st1;
    state_t* st2;
    angle_t an;
    fixed_t radius;

    // decide on what type of grenade to spawn
    if(player->readyweapon == wp_hegrenade)
        type = MT_HEGRENADE;
    else
    {
        if(player->readyweapon == wp_wpgrenade)
            type = MT_PGRENADE;
    }

    player->ammo[weaponinfo[player->readyweapon].ammo]--;

    // set flash frame
    st1 = &states[(pspr->state - states) + weaponinfo[player->readyweapon].flashstate];
    st2 = &states[weaponinfo[player->readyweapon].atkstate];
    P_SetPsprite(player, ps_flash, st1 - st2);

    player->mo->z += (32*FRACUNIT); // ugh
    mo = P_SpawnMortar(player->mo, type);
    player->mo->z -= (32*FRACUNIT); // ugh

    // change momz based on player's pitch
    mo->momz = FixedMul((player->pitch<<FRACBITS) / 160, mo->info->speed) + (8*FRACUNIT);
    S_StartSound(mo, mo->info->seesound);

    radius = mobjinfo[type].radius + player->mo->info->radius;
    an = (player->mo->angle >> ANGLETOFINESHIFT);

    mo->x += FixedMul(finecosine[an], radius + (4*FRACUNIT));
    mo->y += FixedMul(finesine[an], radius + (4*FRACUNIT));

    // shoot grenade from left or right side?
    if(&states[weaponinfo[player->readyweapon].atkstate] == pspr->state)
        an = (player->mo->angle - ANG90) >> ANGLETOFINESHIFT;
    else
        an = (player->mo->angle + ANG90) >> ANGLETOFINESHIFT;

    mo->x += FixedMul((15*FRACUNIT), finecosine[an]);
    mo->y += FixedMul((15*FRACUNIT), finesine[an]);

    // set bounce flag
    mo->flags |= MF_BOUNCE;
}

//
// A_FireElectricBolt
// villsa [STRIFE] - new codepointer
//

void A_FireElectricBolt(player_t* player, pspdef_t* pspr)
{
    angle_t an = player->mo->angle;

    player->mo->angle += (P_Random() - P_Random()) << (18 - (player->accuracy * 4 / 100));
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_ELECARROW);
    player->mo->angle = an;
    S_StartSound(player->mo, sfx_xbow);
}

//
// A_FirePoisonBolt
// villsa [STRIFE] - new codepointer
//

void A_FirePoisonBolt(player_t* player, pspdef_t* pspr)
{
    angle_t an = player->mo->angle;

    player->mo->angle += (P_Random() - P_Random())<<(18 - (player->accuracy * 4 / 100));
    player->ammo[weaponinfo[player->readyweapon].ammo]--;
    P_SpawnPlayerMissile(player->mo, MT_POISARROW);
    player->mo->angle = an;
    S_StartSound(player->mo, sfx_xbow);
}

//
// P_BulletSlope
// Sets a slope so a near miss is at aproximately
// the height of the intended target
//
fixed_t		bulletslope;


void P_BulletSlope (mobj_t*	mo)
{
    angle_t	an;
    
    // see which target is to be aimed at
    an = mo->angle;
    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);

    if (!linetarget)
    {
	an += 1<<26;
	bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	if (!linetarget)
	{
	    an -= 2<<26;
	    bulletslope = P_AimLineAttack (mo, an, 16*64*FRACUNIT);
	}
    }
}


//
// P_GunShot
//
void
P_GunShot
( mobj_t*	mo,
  boolean	accurate )
{
    angle_t	angle;
    int		damage;
	
    damage = 4*(P_Random ()%3+4);   // villsa [STRIFE] different damage formula
    angle = mo->angle;

    // villsa [STRIFE] apply player accuracy
    if (!accurate)
	angle += (P_Random()-P_Random())<<(20 - (mo->player->accuracy * 4 / 100));

    P_LineAttack (mo, angle, MISSILERANGE, bulletslope, damage);
}

//
// A_FireRifle
// villsa [STRIFE] - new codepointer
//

void A_FireRifle(player_t* player, pspdef_t* pspr)
{
    S_StartSound(player->mo, sfx_rifle);

    if(player->ammo[weaponinfo[player->readyweapon].ammo])
    {
        P_SetMobjState(player->mo, S_PLAY_06);
        player->ammo[weaponinfo[player->readyweapon].ammo]--;
        P_BulletSlope(player->mo);
        P_GunShot(player->mo, !player->refire);
    }
}

//
// A_FireMauler1
// villsa [STRIFE] - new codepointer
//

void A_FireMauler1(player_t* player, pspdef_t* pspr)
{
    int i;
    angle_t angle;
    int damage;

    if(player->ammo[weaponinfo[player->readyweapon].ammo] > 20)
    {
        player->ammo[weaponinfo[player->readyweapon].ammo] -= 20;
        P_BulletSlope(player->mo);
        S_StartSound(player->mo, sfx_pgrdat);

        for(i = 0; i < 20; i++)
        {
            damage = 5*(P_Random ()%3+1);
            angle = player->mo->angle;
            angle += (P_Random()-P_Random())<<19;
            P_LineAttack(player->mo, angle, MISSILERANGE,
                bulletslope + ((P_Random()-P_Random())<<5), damage);
        }
    }
}

//
// A_SigilSound
// villsa [STRIFE] - new codepointer
//

void A_SigilSound(player_t* player, pspdef_t* pspr)
{
    S_StartSound(player->mo, sfx_siglup);
    player->extralight = 2;

}


//
// ?
//
void A_Light0 (player_t *player, pspdef_t *psp)
{
    player->extralight = 0;
}

void A_Light1 (player_t *player, pspdef_t *psp)
{
    player->extralight = 1;
}

void A_Light2 (player_t *player, pspdef_t *psp)
{
    player->extralight = 2;
}


//
// P_SetupPsprites
// Called at start of level for each player.
//
void P_SetupPsprites (player_t* player) 
{
    int	i;
	
    // remove all psprites
    for (i=0 ; i<NUMPSPRITES ; i++)
	player->psprites[i].state = NULL;
		
    // spawn the gun
    player->pendingweapon = player->readyweapon;
    P_BringUpWeapon (player);
}




//
// P_MovePsprites
// Called every tic by player thinking routine.
//
void P_MovePsprites (player_t* player) 
{
    int		i;
    pspdef_t*	psp;
    state_t*	state;
	
    psp = &player->psprites[0];
    for (i=0 ; i<NUMPSPRITES ; i++, psp++)
    {
	// a null state means not active
	if ( (state = psp->state) )	
	{
	    // drop tic count and possibly change state

	    // a -1 tic count never changes
	    if (psp->tics != -1)	
	    {
		psp->tics--;
		if (!psp->tics)
		    P_SetPsprite (player, i, psp->state->nextstate);
	    }				
	}
    }
    
    player->psprites[ps_flash].sx = player->psprites[ps_weapon].sx;
    player->psprites[ps_flash].sy = player->psprites[ps_weapon].sy;
}



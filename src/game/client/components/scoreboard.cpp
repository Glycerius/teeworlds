#include <string.h>
#include <engine/e_client_interface.h>
#include <engine/client/graphics.h>
#include <game/generated/g_protocol.hpp>
#include <game/generated/gc_data.hpp>
#include <game/client/gameclient.hpp>
#include <game/client/animstate.hpp>
#include <game/client/render.hpp>
#include <game/client/components/motd.hpp>
#include "scoreboard.hpp"


SCOREBOARD::SCOREBOARD()
{
	on_reset();
}

void SCOREBOARD::con_key_scoreboard(void *result, void *user_data)
{
	((SCOREBOARD *)user_data)->active = console_arg_int(result, 0) != 0;
}

void SCOREBOARD::on_reset()
{
	active = false;
}

void SCOREBOARD::on_console_init()
{
	MACRO_REGISTER_COMMAND("+scoreboard", "", CFGFLAG_CLIENT, con_key_scoreboard, this, "Show scoreboard");
}

void SCOREBOARD::render_goals(float x, float y, float w)
{
	float h = 50.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	Graphics()->QuadsEnd();

	// render goals
	//y = ystart+h-54;
	float tw = 0.0f;
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->score_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d", localize("Score limit"), gameclient.snap.gameobj->score_limit);
		gfx_text(0, x+20.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->time_limit)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s: %d min", localize("Time limit"), gameclient.snap.gameobj->time_limit);
		gfx_text(0, x+220.0f, y, 22.0f, buf, -1);
		tw += gfx_text_width(0, 22.0f, buf, -1);
	}
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->round_num && gameclient.snap.gameobj->round_current)
	{
		char buf[64];
		str_format(buf, sizeof(buf), "%s %d/%d", localize("Round"), gameclient.snap.gameobj->round_current, gameclient.snap.gameobj->round_num);
		gfx_text(0, x+450.0f, y, 22.0f, buf, -1);
		
	/*[48c3fd4c][game/scoreboard]: timelimit x:219.428558
	[48c3fd4c][game/scoreboard]: round x:453.142822*/
	}
}

void SCOREBOARD::render_spectators(float x, float y, float w)
{
	char buffer[1024*4];
	int count = 0;
	float h = 120.0f;
	
	str_format(buffer, sizeof(buffer), "%s: ", localize("Spectators"));

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->draw_round_rect(x-10.f, y-10.f, w, h, 10.0f);
	Graphics()->QuadsEnd();
	
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == -1)
			{
				if(count)
					strcat(buffer, ", ");
				strcat(buffer, gameclient.clients[info->cid].name);
				count++;
			}
		}
	}
	
	gfx_text(0, x+10, y, 32, buffer, (int)w-20);
}

void SCOREBOARD::render_scoreboard(float x, float y, float w, int team, const char *title)
{
	//float ystart = y;
	float h = 750.0f;

	Graphics()->BlendNormal();
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.5f);
	RenderTools()->draw_round_rect(x-10.f, y-10.f, w, h, 17.0f);
	Graphics()->QuadsEnd();

	// render title
	if(!title)
	{
		if(gameclient.snap.gameobj->game_over)
			title = localize("Game over");
		else
			title = localize("Score board");
	}

	float tw = gfx_text_width(0, 48, title, -1);

	if(team == -1)
	{
		gfx_text(0, x+w/2-tw/2, y, 48, title, -1);
	}
	else
	{
		gfx_text(0, x+10, y, 48, title, -1);

		if(gameclient.snap.gameobj)
		{
			char buf[128];
			int score = team ? gameclient.snap.gameobj->teamscore_blue : gameclient.snap.gameobj->teamscore_red;
			str_format(buf, sizeof(buf), "%d", score);
			tw = gfx_text_width(0, 48, buf, -1);
			gfx_text(0, x+w-tw-30, y, 48, buf, -1);
		}
	}

	y += 54.0f;

	// find players
	const NETOBJ_PLAYER_INFO *players[MAX_CLIENTS] = {0};
	int num_players = 0;
	for(int i = 0; i < snap_num_items(SNAP_CURRENT); i++)
	{
		SNAP_ITEM item;
		const void *data = snap_get_item(SNAP_CURRENT, i, &item);

		if(item.type == NETOBJTYPE_PLAYER_INFO)
		{
			const NETOBJ_PLAYER_INFO *info = (const NETOBJ_PLAYER_INFO *)data;
			if(info->team == team)
			{
				players[num_players] = info;
				num_players++;
			}
		}
	}

	// sort players
	for(int k = 0; k < num_players; k++) // ffs, bubblesort
	{
		for(int i = 0; i < num_players-k-1; i++)
		{
			if(players[i]->score < players[i+1]->score)
			{
				const NETOBJ_PLAYER_INFO *tmp = players[i];
				players[i] = players[i+1];
				players[i+1] = tmp;
			}
		}
	}

	// render headlines
	gfx_text(0, x+10, y, 24.0f, localize("Score"), -1);
	gfx_text(0, x+125, y, 24.0f, localize("Name"), -1);
	gfx_text(0, x+w-70, y, 24.0f, localize("Ping"), -1);
	y += 29.0f;

	float font_size = 35.0f;
	float line_height = 50.0f;
	float tee_sizemod = 1.0f;
	float tee_offset = 0.0f;
	
	if(num_players > 13)
	{
		font_size = 30.0f;
		line_height = 40.0f;
		tee_sizemod = 0.8f;
		tee_offset = -5.0f;
	}
	
	// render player scores
	for(int i = 0; i < num_players; i++)
	{
		const NETOBJ_PLAYER_INFO *info = players[i];

		// make sure that we render the correct team

		char buf[128];
		if(info->local)
		{
			// background so it's easy to find the local player
			Graphics()->TextureSet(-1);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1,1,1,0.25f);
			RenderTools()->draw_round_rect(x, y, w-20, line_height*0.95f, 17.0f);
			Graphics()->QuadsEnd();
		}

		str_format(buf, sizeof(buf), "%4d", info->score);
		gfx_text(0, x+60-gfx_text_width(0, font_size,buf,-1), y, font_size, buf, -1);
		
		gfx_text(0, x+128, y, font_size, gameclient.clients[info->cid].name, -1);

		str_format(buf, sizeof(buf), "%4d", info->latency);
		float tw = gfx_text_width(0, font_size, buf, -1);
		gfx_text(0, x+w-tw-35, y, font_size, buf, -1);

		// render avatar
		if((gameclient.snap.flags[0] && gameclient.snap.flags[0]->carried_by == info->cid) ||
			(gameclient.snap.flags[1] && gameclient.snap.flags[1]->carried_by == info->cid))
		{
			Graphics()->BlendNormal();
			Graphics()->TextureSet(data->images[IMAGE_GAME].id);
			Graphics()->QuadsBegin();

			if(info->team == 0) RenderTools()->select_sprite(SPRITE_FLAG_BLUE, SPRITE_FLAG_FLIP_X);
			else RenderTools()->select_sprite(SPRITE_FLAG_RED, SPRITE_FLAG_FLIP_X);
			
			float size = 64.0f;
			Graphics()->QuadsDrawTL(x+55, y-15, size/2, size);
			Graphics()->QuadsEnd();
		}
		
		TEE_RENDER_INFO teeinfo = gameclient.clients[info->cid].render_info;
		teeinfo.size *= tee_sizemod;
		RenderTools()->RenderTee(ANIMSTATE::get_idle(), &teeinfo, EMOTE_NORMAL, vec2(1,0), vec2(x+90, y+28+tee_offset));

		
		y += line_height;
	}
}

void SCOREBOARD::on_render()
{
	bool do_scoreboard = false;

	// if we activly wanna look on the scoreboard	
	if(active)
		do_scoreboard = true;
		
	if(gameclient.snap.local_info && gameclient.snap.local_info->team != -1)
	{
		// we are not a spectator, check if we are ead
		if(!gameclient.snap.local_character || gameclient.snap.local_character->health < 0)
			do_scoreboard = true;
	}

	// if we the game is over
	if(gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		do_scoreboard = true;
		
	if(!do_scoreboard)
		return;
		
	// if the score board is active, then we should clear the motd message aswell
	if(active)
		gameclient.motd->clear();
	

	float width = 400*3.0f*Graphics()->ScreenAspect();
	float height = 400*3.0f;
	
	Graphics()->MapScreen(0, 0, width, height);

	float w = 650.0f;

	if(gameclient.snap.gameobj && !(gameclient.snap.gameobj->flags&GAMEFLAG_TEAMS))
	{
		render_scoreboard(width/2-w/2, 150.0f, w, 0, 0);
		//render_scoreboard(gameobj, 0, 0, -1, 0);
	}
	else
	{
			
		if(gameclient.snap.gameobj && gameclient.snap.gameobj->game_over)
		{
			const char *text = localize("Draw!");
			if(gameclient.snap.gameobj->teamscore_red > gameclient.snap.gameobj->teamscore_blue)
				text = localize("Red team wins!");
			else if(gameclient.snap.gameobj->teamscore_blue > gameclient.snap.gameobj->teamscore_red)
				text = localize("Blue team wins!");
				
			float w = gfx_text_width(0, 92.0f, text, -1);
			gfx_text(0, width/2-w/2, 45, 92.0f, text, -1);
		}
		
		render_scoreboard(width/2-w-20, 150.0f, w, 0, localize("Red team"));
		render_scoreboard(width/2 + 20, 150.0f, w, 1, localize("Blue team"));
	}

	render_goals(width/2-w/2, 150+750+25, w);
	render_spectators(width/2-w/2, 150+750+25+50+25, w);
}

#include Library_Mission_Script

func InitializePlayer(proplist player)
{
	_inherited(player, ...);
	
	// Set zoom and move player to the middle of the scenario.
	player->SetZoomByViewRange(LandscapeWidth(), nil, PLRZOOM_Direct);

	var avatar = CreateObject(Character_Player, 120, 190, player);

	player->GetCrew()->RemoveObject();
	avatar->MakeCrewMember(player);

	player->GetCrew()->SetPosition(120, 190);
	player->GetCrew()->MakeInvincible();
	player->GetCrew()->CreateContents(Item_Axe);
	player->GetCrew()->CreateContents(Item_Hammer);
	return true;
}

func Initialize()
{
	var bot = CreateObject(Character_Human, 150, 190);
	AI_Type_Civilian->AddAI(bot);
}

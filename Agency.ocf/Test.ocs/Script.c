
protected func InitializePlayer(int player)
{
	// Set zoom and move player to the middle of the scenario.
	SetPlayerZoomByViewRange(player, LandscapeWidth(), nil, PLRZOOM_Direct);
	GetCrew(player)->SetPosition(120, 190);
	GetCrew(player)->MakeInvincible();

	return true;
}


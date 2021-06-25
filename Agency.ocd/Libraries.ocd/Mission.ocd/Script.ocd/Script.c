
func InitializePlayer(proplist player, int x, int y, object base, int team, id extra_data)
{
	player->SetControlEnabled(CON_PlayerMenu, false);
	return _inherited(player, x, y, base, team, extra_data, ...);
}

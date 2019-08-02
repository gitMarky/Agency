
func InitializePlayer(int player, int x, int y, object base, int team, id extra_data)
{
	SetPlayerControlEnabled(player, CON_PlayerMenu, false);
	return _inherited(player, x, y, base, team, extra_data, ...);
}

#appendto Library_ClonkUseControl

func GetUseCallString(string action)
{
	// Control... or Contained...
	var prefix = "Control";
	if (this.control.using_type == C4D_Structure)
	{
		prefix = "Contained";
	}
	// ..Use.. or ..UseAlt...
	var type = "";
	if (this.control.alt)
	{
		type = "Alt";
	}
	return Format("~%sUse%s%s", prefix, type, action ?? "");
}

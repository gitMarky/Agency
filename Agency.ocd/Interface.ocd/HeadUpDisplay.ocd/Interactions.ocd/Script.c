/**
	Shows a small preview of what happens on CON_Interact next to the interaction target.
*/

/* --- Properties --- */

local interaction_display;

/* --- Engine Callbacks --- */

func Construction()
{
	interaction_display = {};
	EnableInteractionUpdating(true);
	return _inherited(...);
}

func Destruction()
{
	HideInteractions();
	EnableInteractionUpdating(false);
	_inherited(...);
}


/* --- Timer --- */

local FxIntUpdateInteraction = new Effect
{
	Name = "IntUpdateInteraction",
	
	Timer = func ()
	{
		this.Target->UpdateInteractionObject();
	}
};

public func EnableInteractionUpdating(bool enable)
{
	var fx = GetEffect(FxIntUpdateInteraction.Name, this);
	if (fx && !enable)
	{
		RemoveEffect(nil, nil, fx);
	}
	else if (!fx && enable)
	{
		CreateEffect(FxIntUpdateInteraction, 1, 10);
	}
}

/**
	Searches for interactable objects and updates the display.
*/
public func UpdateInteractionObject()
{
	var cursor = GetCursor(GetOwner());
	if (!cursor || !cursor->GetCrewEnabled())
	{
		HideInteractions();
		return;
	}

	var interactions = cursor->~GetInteractableObjects();
	if (!interactions || !GetLength(interactions))
	{
		HideInteractions();
		return;
	}

	// Display the interactions
	for (var interaction in interactions)
	{
		// Do a dumb display for now
		ShowInteraction(interaction, cursor->~HasInteraction(interaction));
	}
}

// Does hide the interaction
func HideInteractions()
{
	if (!interaction_display)
	{
		return;
	}
	for (var existing in GetProperties(interaction_display))
	{
		if (interaction_display[existing])
		{
			interaction_display[existing]->RemoveObject();
		}
	}
}

func ShowInteraction(proplist interaction, bool show)
{
	if (interaction == nil)
	{
		return;
	}
	var target = interaction.Target;
	if (target)
	{
		target = Format("%d", target->ObjectNumber());
		var existing = interaction_display[target];
		if (existing)
		{
			interaction.Display = existing;
		}

		if (show)
		{
			if (interaction.Display == nil)
			{
				var message = CreateObject(FloatingMessage, 0, 0, GetOwner());
				message.Visibility = VIS_Owner;
				message->SetMessage(Format("[%s] %s: %s", GetPlayerControlAssignment(GetOwner(), interaction.Control, true, false), interaction.Name, interaction.Desc));
				message->SetSpeed(0, 0);

				interaction_display[target] = message;
				interaction.Display = message;
			}
			interaction.Display->SetPosition(interaction.Target->GetX(), interaction.Target->GetY());
		}
		else if (interaction.Display != nil)
		{
			interaction.Display->RemoveObject();
		}
	}
}

/**
Returns a proplist with the following properties to display:
help_text: A text describing the interaction or ""
help_icon: A pictographic icon definition symbolising the interaction or nil
help_icon_graphics: The graphics of the icon definition to use or ""
*/
func GetInteractionHelp(proplist interaction, object clonk)
{
	var actiontype = interaction.actiontype;
	var to_interact = interaction.interaction_object;
	var interaction_index = interaction.interaction_index;
	
	var ret =
	{
		help_text = "",
		help_icon = nil,
		help_icon_graphics = ""
	};

	// Help text: Grabbing / Ungrabbing / Pushing out
	if (actiontype == ACTIONTYPE_VEHICLE)
	{
		if (clonk->Contained() && to_interact->Contained() == clonk->Contained())
		{
			ret.help_text = Format("$TxtPushOut$", to_interact->GetName());
			ret.help_icon = Icon_Exit;
		}
		else if (clonk->GetProcedure() == "PUSH" && clonk->GetActionTarget() == to_interact)
		{
			ret.help_text = Format("$TxtUnGrab$", to_interact->GetName());
			ret.help_icon = Icon_LetGo;
		}
		else
		{
			ret.help_text = Format("$TxtGrab$", to_interact->GetName());
			ret.help_icon = Icon_Grab;
		}
	}

	// Help text: Enter / Exit
	if (actiontype == ACTIONTYPE_STRUCTURE)
	{
		if (clonk->Contained() && clonk->Contained() == to_interact)
		{
			ret.help_text = Format("$TxtExit$", to_interact->GetName());
			ret.help_icon = Icon_Exit;
		}
		else
		{
			ret.help_text = Format("$TxtEnter$", to_interact->GetName());
			ret.help_icon = Icon_Enter;
		}
	}

	// Help text: Script Interaction
	if (actiontype == ACTIONTYPE_SCRIPT)
	{
		var metainfo = to_interact->~GetInteractionMetaInfo(clonk, interaction_index);
		if (metainfo)
		{
			ret.help_text = metainfo.Description;
			ret.help_icon = metainfo.IconID;
			ret.help_icon_graphics = metainfo.IconName;
		}
	}

	// Help text: Extra Interaction (already in proplist)
	if (actiontype == ACTIONTYPE_EXTRA)
	{
		ret.help_text = interaction.extra_data.Description;
		ret.help_icon = interaction.extra_data.IconID;
		ret.help_icon_graphics = interaction.extra_data.IconName;
	}

	return ret;
}

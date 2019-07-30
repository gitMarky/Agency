/**
	Handles interactions with other objects.

	Copied from Objects.ocd, will be changed as I go along.
*/


/*
	used properties:
	this.control.is_interacting: whether interaction is in progress (user is holding [space])
	this.control.interaction_start_time: frame counter at the time of the selection process
	this.control.interaction_hud_controller: hud object that takes the callbacks. Updated when starting interaction.
*/

local InteractionDistance = 12;


func Construction()
{
	this.control.is_interacting = false;
	return _inherited(...);
}


func OnShiftCursor(object new_cursor)
{
	if (this.control.is_interacting)
	{
		AbortInteract();
	}
	return _inherited(new_cursor, ...);
}


func ObjectControl(int plr, int ctrl, int x, int y, int strength, bool repeat, int status)
{
	if (!this)
	{
		return false;
	}

	// Begin interaction.
	if (IsInteractionControl(ctrl) && status == CONS_Down)
	{
		this->CancelUse();
		BeginInteract(ctrl);
		return true;
	}
	
	// Switch object or finish interaction?
	
	var interaction_control = this.control.is_interacting;
	if (interaction_control)
	{

	}

	return inherited(plr, ctrl, x, y, strength, repeat, status, ...);
}


local FxControlInteraction = new Effect
{
	Name = "FxControlInteraction",

	Start = func (int temp, proplist interaction, bool has_multiple)
	{
		if (!temp)
		{
			this.obj = interaction.Target;
			this.interaction = interaction;
			this.interaction_help = this.Target.control.interaction_hud_controller->GetInteractionHelp(interaction, this.Target);
		
			CreateDummy(has_multiple);
		}
	},
	
	Timer = func (int time)
	{
		if (!this.dummy) return -1;
		if (!this.obj) return -1;
	
		if (this.scheduled_selection_particle && time > 10)
		{
			this->CreateSelectorParticle();
			this.scheduled_selection_particle = false;
		}
	},
	
	Stop = func (int reason, temp)
	{
		if (temp) return;
		if (this.dummy) this.dummy->RemoveObject();
		if (!this) return;
	},
	
	OnExecute = func ()
	{
		if (!this.obj || !this.dummy) return;
		var message = this.dummy->CreateObject(FloatingMessage, 0, 0, GetOwner());
		message.Visibility = VIS_Owner;
		message->SetMessage(Format("%s||", this.interaction_help.help_text));
		message->SetYDir(-10);
		message->FadeOut(1, 20);
	},
	
	GetOwner = func ()
	{
		return this.Target->GetOwner();
	},
	
	CreateSelectorParticle = func ()
	{
		// Failsafe.
		if (!this.dummy) return;
	
		// Draw a nice selector particle on item change.
		var selector =
		{
			Size = PV_Step(5, 2, 1, Max(this.width, this.height)),
			Attach = ATTACH_Front,
			Rotation = PV_Step(1, PV_Random(0, 360), 1),
			Alpha = 200
		};
	
		this.dummy->CreateParticle("Selector", 0, 0, 0, 0, 0, Particles_Colored(selector, GetPlayerColor(GetOwner())), 1);
	},

	CreateDummy = func (bool has_multiple)
	{
		this.dummy = this.Target->CreateObject(Dummy, this.obj->GetX() - this.Target->GetX(), this.obj->GetY() - this.Target->GetY(), GetOwner());
		this.dummy.ActMap =
		{
			Attach =
			{
				Name = "Attach",
				Procedure = DFA_ATTACH,
				FacetBase = 1
			}
		};
		this.dummy.Visibility = VIS_Owner;
		// The selector's plane should be just behind the Clonk for stuff that usually is behind the Clonk.
		// Otherwise, it looks rather odd when the catapult shines through the Clonk.
		if (this.obj.Plane < this.Plane)
		{
			this.dummy.Plane = this.Plane - 1;
		}
		else
		{
			this.dummy.Plane = 1000;
		}
		var cycle_interactions_hint = "";
		if (has_multiple)
		{
			cycle_interactions_hint = Format("|<c 999999>[%s/%s] $Cycle$..</c>", GetPlayerControlAssignment(GetOwner(), CON_Left, true, false), GetPlayerControlAssignment(GetOwner(), CON_Right, true, false));
		}
		this.dummy->Message("@<c eeffee>%s</c>%s|", this.interaction_help.help_text, cycle_interactions_hint);
	
		// Center dummy!
		this.dummy->SetVertexXY(0, this.obj->GetVertex(0, VTX_X), this.obj->GetVertex(0, VTX_Y));
		this.dummy->SetAction("Attach", this.obj);
	
		this.width  = this.obj->GetDefWidth();
		this.height = this.obj->GetDefHeight();
	
		// Draw the item's graphics in front of it again to achieve a highlighting effect.
		this.dummy->SetGraphics(nil, nil, 1, GFXOV_MODE_Object, nil, GFX_BLIT_Additive, this.obj);
	
		var custom_selector = nil;
		if (this.obj)
		{
			custom_selector = this.obj->~DrawCustomInteractionSelector(this.dummy, this.Target, this.interaction.interaction_index, this.interaction.extra_data);
		}
	
		if (!custom_selector)
		{
			this.scheduled_selection_particle = (FrameCounter() - this.Target.control.interaction_start_time) < 10;
			if (!this.scheduled_selection_particle)
			{
				this->CreateSelectorParticle();
			}
		}
		else
		{
			// Note that custom selectors are displayed immediately - particle because they might e.g. move the dummy.
			this.scheduled_selection_particle = false;
		}
	},

	Control = func (int ctrl, int x, int y, int strength, bool repeat, int status)
	{
		var interaction_control = this.Target->GetCurrentInteraction().Control;

		// Stop interacting.
		if (ctrl == CON_Down || ctrl == CON_PickUpNext_Stop || ctrl == CON_InteractNext_Stop)
		{
			this.Target->AbortInteract();
			return true;
		}

		// Finish interacting.
		if (ctrl == interaction_control && status == CONS_Up)
		{
			this.Target->EndInteract();
			return true;
		}

		// Switch left/right through objects.
		var dir = nil;
		if (ctrl == CON_Left || ctrl == CON_PickUpNext_Left || ctrl == CON_InteractNext_Left)
		{
			dir = -1;
		}
		else if (ctrl == CON_Right || ctrl == CON_PickUpNext_Right || ctrl == CON_InteractNext_Right)
		{
			dir = 1;
		}

		if (dir != nil)
		{
			var item = this.Target->FindNextInteraction(interaction_control, this.Target->GetCurrentInteraction(), dir, true);
			if (item)
			{
				this.Target->SetNextInteraction(item);
			}
			return true;
		}
		
		// Stop interacting if another button is pressed.
		if (ctrl != interaction_control)
		{
			this.Target->AbortInteract();
			return true;
		}
	},
};


func SetNextInteraction(proplist interaction, bool has_multiple)
{
	// Clear all old markers.
	var control;
	while (control = GetEffect(FxControlInteraction.Name, this))
	{
		RemoveEffect(nil, this, control);
	}
	// And set & mark new one.
	SetCurrentInteraction(interaction);
	if (interaction)
	{
		CreateEffect(FxControlInteraction, 1, 2, interaction, has_multiple);
	}
}


func FindNextInteraction(int ctrl, proplist previous_interaction, int cycle_dir)
{
	var starting_object = this;
	if (previous_interaction && previous_interaction.Target)
	{
		starting_object = previous_interaction.Target;
	}
	var sort = Sort_Func("Library_ClonkInventoryControl_Sort_Priority", starting_object->GetX());
	var interactions = GetInteractionInfos(ctrl, sort);
	var count = GetLength(interactions);
	if (!count)
	{
		return nil;
	}
	// Find object next to the current one.
	// (note that index == -1 accesses the last element)
	var index = -1;
	// Determine index of the previous interaction 
	// GetIndexOf does not use DeepEqual, so work around that here.
	// DeepEqual is not suitable because we count when the condition
	// was evaluated the last time.
	for (var i = 0; i < count; ++i)
	{
		if (IsSameInteraction(previous_interaction, interactions[i]))
		{
			index = i;
			break;
		}
	}

	var next_interaction_index = -1;
	if (index == -1) // Previous item was not found in the list.
	{
		// Find highest priority item.
		var high_prio = nil;
		for (var i = 0; i < count; ++i)
		{
			var interaction = interactions[i];
			if (high_prio == nil || interaction.Priority > high_prio.Priority)
			{
				high_prio = interaction;
				next_interaction_index = i;
			}
		}
	}
	else // Cycle through interactions to the right or left .
	{
		next_interaction_index = index;
		var found = false;
		for (var i = 1; (i < count) && !found; ++i)
		{
			next_interaction_index += cycle_dir;
			if (next_interaction_index < 0)
			{
				next_interaction_index += count;
			}
			next_interaction_index = next_interaction_index % count;

			if (!IsSameInteraction(previous_interaction, interactions[next_interaction_index]))
			{
				found = true;

				// When cycling to the left, make sure to arrive at the first interaction for that object (and not the last).
				// Otherwise it's pretty unintuitive, why you sometimes grab and sometimes enter the catapult as the first interaction.
				if (cycle_dir == -1)
				{
					// Fast forward to first interaction.
					var target_object = interactions[next_interaction_index].Target;
					// It's guaranteed that the interactions are not split over the array borders. So we can just search until the index is 0.
					for (var current_index = next_interaction_index - 1; current_index >= 0; --current_index)
					{
						if (interactions[current_index].Target == target_object)
						{
							index = current_index;
						}
					}
				}
			}
		}
	}

	if (next_interaction_index == nil || next_interaction_index == -1)
	{
		return nil;
	}
	return interactions[next_interaction_index];
}


func BeginInteract(int ctrl)
{
	this.control.interaction_hud_controller = this->GetHUDController();
	this.control.is_interacting = true;
	this.control.interaction_start_time = FrameCounter();

	// Force update the HUD controller, which is responsible for pre-selecting the "best" object.
	this.control.interaction_hud_controller->UpdateInteractionObject();
	// Then, iff the HUD shows an object, pre-select one.
	var interactions = GetInteractionInfos(ctrl, nil);
	var interaction = interactions[0];
	if (interaction)
	{
		SetNextInteraction(interaction, GetLength(interactions) > 1);
		this.control.interaction_hud_controller->EnableInteractionUpdating(false);
	}
	else
	{
		AbortInteract();
	}
}


// Stops interaction selection without executing the current selection.
func AbortInteract()
{
	SetCurrentInteraction(nil);
	EndInteract();
}


func EndInteract()
{
	this.control.is_interacting = false;

	var executed = false;
	if (GetCurrentInteraction())
	{
		ExecuteInteraction(GetCurrentInteraction());
		executed = true;
	}

	var interaction = nil;
	while (interaction = GetEffect(FxControlInteraction.Name, this))
	{
		if (executed)
		{
			interaction->OnExecute();
		}
		RemoveEffect(nil, this, interaction);
	}

	SetCurrentInteraction(nil);
	this.control.interaction_hud_controller->EnableInteractionUpdating(true);
}


/**
	Returns an array containing proplists with information about the interactable actions.
	The proplist properties are:
		interaction_object
		priority: used for sorting the objects in the action bar. Note that the returned objects are not yet sorted
		interaction_index: when an object has multiple defined interactions, this is the index
		extra_data: custom extra_data for an interaction specified by the object
		actiontype: the kind of interaction. One of the ACTIONTYPE_* constants
*/
func GetInteractionInfos(int ctrl, array sort)
{
	var possible_interactions = [];

	// All except structures only if outside
	var can_use_surrounding = !Contained();

	// Add interactables (script interface)
	var interactables = GetInteractables();
	for (var interactable in interactables)
	{
		var uses_container = interactable == Contained();
		if (can_use_surrounding || uses_container)
		{
			for (var interaction in interactable->~GetInteractions(this))
			{
				if (HasInteraction(interaction) && (ctrl == nil || interaction.Control == ctrl))
				{
					PushBack(possible_interactions, interaction);
				}
			}
		}
	}

	return possible_interactions;
}


func GetCurrentInteraction()
{
	return this.control.current_interaction;
}


func SetCurrentInteraction(proplist interaction)
{
	this.control.current_interaction = interaction;
}


func GetInteractables(array sort)
{
	// TODO: Cache these, too
	// Make sure that the Clonk's action target is always shown.
	// You can push a lorry out of your bounding box and would, otherwise, then be unable to release it.
	var main_criterion = Find_Distance(InteractionDistance);
	var action_target = GetActionTarget();
	if (action_target)
	{
		main_criterion = Find_Or(main_criterion, Find_InArray([action_target]));
	}

	return FindObjects(main_criterion,
		               Find_Func("IsInteractable"),
		               Find_NoContainer(),
		               Find_Layer(GetObjectLayer()),
		               Find_Exclude(this),
		               sort);
}


func HasInteraction(proplist interaction)
{
	if (interaction.Target && interaction.Execute)
	{
		var current_frame = FrameCounter();
		var condition = interaction.Condition;
		if (condition == nil)
		{
			interaction.IsAvailable = true; // Setting the property is preferred over returning immediately, so that the internal state is consistent
		}
		else if (current_frame > interaction.LastChecked)
		{
			interaction.LastChecked = current_frame;
			interaction.IsAvailable = interaction.Target->Call(condition, this, interaction);
		}
		return interaction.IsAvailable                                 // Checking an individual object first is cheap
		    && IsValueInArray(GetInteractables(), interaction.Target); // Only if that is even interactable, check that the generic conditions still hold
	}
	return false;
}


func IsSameInteraction(proplist a, proplist b)
{
	// Do not factor in:
	// - Description, name, because purely cosmetical
	// - Condition, IsAvailable, because those both need to be valid anyway
	return (a.Target == b.Target)
	    && (a.Control == b.Control)
	    && (a.Execute == b.Execute);
}


// Executes interaction with an object. /action_info/ is a proplist as returned by GetInteractionInfos
func ExecuteInteraction(proplist interaction)
{
	if (HasInteraction(interaction))
	{
		interaction.Target->Call(interaction.Execute, this, interaction);
	}
}


static const Interaction = new Global 
{
	// These variables define the interaction
	
	Target = nil,     // object, target of the interaction
	Name = nil,       // string, name of the interaction
	Desc = nil,       // string, optional, description
	Condition = nil,  // function, optional, the interaction is available only when this condition is true
	Control = CON_Interact, // int, required, the button that is triggers the interaction
	Execute = nil,    // function, required, call this when executing the interaction
	
	// These variables are internal
	IsAvailable = false, // bool, result of evaluation the condition
	LastChecked = 0,     // int, frame when the condition was checked the last time
	Display = nil,       // object, floating message object
};

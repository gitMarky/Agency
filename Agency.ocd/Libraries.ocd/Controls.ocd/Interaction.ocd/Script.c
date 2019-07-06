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
		return inherited(plr, ctrl, x, y, strength, repeat, status, ...);
	}

	// Begin interaction.
	if (ctrl == CON_Interact && status == CONS_Down)
	{
		this->CancelUse();
		BeginInteract();
		return true;
	}

	// Switch object or finish interaction?
	if (this.control.is_interacting)
	{
		// Stop picking up.
		if (ctrl == CON_InteractNext_Stop)
		{
			AbortInteract();
			return true;
		}

		// Finish picking up (aka "collect").
		if (ctrl == CON_Interact && status == CONS_Up)
		{
			EndInteract();
			return true;
		}

		// Switch left/right through objects.
		var dir = nil;
		if (ctrl == CON_InteractNext_Left)
		{
			dir = -1;
		}
		else if (ctrl == CON_InteractNext_Right)
		{
			dir = 1;
		}
		else if (ctrl == CON_InteractNext_CycleObject)
		{
			dir = 0;
		}

		if (dir != nil)
		{
			var item = FindNextInteraction(this.control.interaction_hud_controller->GetCurrentInteraction(), dir);
			if (item)
			{
				SetNextInteraction(item);
			}
			return true;
		}
	}

	return inherited(plr, ctrl, x, y, strength, repeat, status, ...);
}

private func FxIntHighlightInteractionStart(object target, proplist fx, temp, proplist interaction, int nr_interactions)
{
	if (temp)
	{
		return;
	}
	fx.obj = interaction.Target;
	fx.interaction = interaction;
	fx.interaction_help = target.control.interaction_hud_controller->GetInteractionHelp(interaction, target);

	fx.dummy = CreateObject(Dummy, fx.obj->GetX() - GetX(), fx.obj->GetY() - GetY(), GetOwner());
	fx.dummy.ActMap =
	{
		Attach =
		{
			Name = "Attach",
			Procedure = DFA_ATTACH,
			FacetBase = 1
		}
	};
	fx.dummy.Visibility = VIS_Owner;
	// The selector's plane should be just behind the Clonk for stuff that usually is behind the Clonk.
	// Otherwise, it looks rather odd when the catapult shines through the Clonk.
	if (fx.obj.Plane < this.Plane)
	{
		fx.dummy.Plane = this.Plane - 1;
	}
	else
	{
		fx.dummy.Plane = 1000;
	}
	var multiple_interactions_hint = "";
	if (fx.interaction.has_multiple_interactions)
	{
		multiple_interactions_hint = Format("|<c 999999>[%s] $More$..</c>", GetPlayerControlAssignment(GetOwner(), CON_Up, true, false));
	}
	var cycle_interactions_hint = "";
	if (nr_interactions > 1)
	{
		cycle_interactions_hint = Format("|<c 999999>[%s/%s] $Cycle$..</c>", GetPlayerControlAssignment(GetOwner(), CON_Left, true, false), GetPlayerControlAssignment(GetOwner(), CON_Right, true, false));
	}
	fx.dummy->Message("@<c eeffee>%s</c>%s%s|", fx.interaction_help.help_text, multiple_interactions_hint, cycle_interactions_hint);

	// Center dummy!
	fx.dummy->SetVertexXY(0, fx.obj->GetVertex(0, VTX_X), fx.obj->GetVertex(0, VTX_Y));
	fx.dummy->SetAction("Attach", fx.obj);

	fx.width  = fx.obj->GetDefWidth();
	fx.height = fx.obj->GetDefHeight();

	// Draw the item's graphics in front of it again to achieve a highlighting effect.
	fx.dummy->SetGraphics(nil, nil, 1, GFXOV_MODE_Object, nil, GFX_BLIT_Additive, fx.obj);

	var custom_selector = nil;
	if (fx.obj)
	{
		custom_selector = fx.obj->~DrawCustomInteractionSelector(fx.dummy, target, fx.interaction.interaction_index, fx.interaction.extra_data);
	}

	if (!custom_selector)
	{
		fx.scheduled_selection_particle = (FrameCounter() - this.control.interaction_start_time) < 10;
		if (!fx.scheduled_selection_particle)
		{
			EffectCall(nil, fx, "CreateSelectorParticle");
		}
	}
	else
	{
		// Note that custom selectors are displayed immediately - particle because they might e.g. move the dummy.
		fx.scheduled_selection_particle = false;
	}
}

func FxIntHighlightInteractionCreateSelectorParticle(object target, effect fx)
{
	// Failsafe.
	if (!fx.dummy) return;

	// Draw a nice selector particle on item change.
	var selector =
	{
		Size = PV_Step(5, 2, 1, Max(fx.width, fx.height)),
		Attach = ATTACH_Front,
		Rotation = PV_Step(1, PV_Random(0, 360), 1),
		Alpha = 200
	};

	fx.dummy->CreateParticle("Selector", 0, 0, 0, 0, 0, Particles_Colored(selector, GetPlayerColor(GetOwner())), 1);
}

func FxIntHighlightInteractionTimer(object target, proplist fx, int time)
{
	if (!fx.dummy) return -1;
	if (!fx.obj) return -1;

	if (fx.scheduled_selection_particle && time > 10)
	{
		EffectCall(nil, fx, "CreateSelectorParticle");
		fx.scheduled_selection_particle = false;
	}
}

func FxIntHighlightInteractionStop(object target, proplist fx, int reason, temp)
{
	if (temp) return;
	if (fx.dummy) fx.dummy->RemoveObject();
	if (!this) return;
}

func FxIntHighlightInteractionOnExecute(object target, proplist fx)
{
	if (!fx.obj || !fx.dummy) return;
	var message = fx.dummy->CreateObject(FloatingMessage, 0, 0, GetOwner());
	message.Visibility = VIS_Owner;
	message->SetMessage(Format("%s||", fx.interaction_help.help_text));
	message->SetYDir(-10);
	message->FadeOut(1, 20);
}

func SetNextInteraction(proplist to)
{
	// Clear all old markers.
	var e = nil;
	while (e = GetEffect("IntHighlightInteraction", this))
		RemoveEffect(nil, this, e);
	// And set & mark new one.
	this.control.interaction_hud_controller->SetCurrentInteraction(to);
	var interaction_cnt = GetInteractableObjectsCount();
	if (to)
		AddEffect("IntHighlightInteraction", this, 1, 2, this, nil, to, interaction_cnt);
}

func FindNextInteraction(proplist start_from, int x_dir)
{
	var starting_object = this;
	if (start_from && start_from.Target)
		starting_object = start_from.Target;
	var sort = Sort_Func("Library_ClonkInventoryControl_Sort_Priority", starting_object->GetX());
	var interactions = GetInteractableObjects(sort);
	var len = GetLength(interactions);
	if (!len) return nil;
	// Find object next to the current one.
	// (note that index==-1 accesses the last element)
	var index = -1;
	// GetIndexOf does not use DeepEqual, so work around that here.
	for (var i = 0; i < len; ++i)
	{
		if (!DeepEqual(start_from, interactions[i])) continue;
		index = i;
		break;
	}

	if (index != -1) // Previous item was found in the list.
	{
		var previous_interaction = interactions[index];
		// Cycle interactions of same object (dir == 0).
		// Or cycle through objects to the right (x_dir==1) or left (x_dir==-1).
		var cycle_dir = x_dir;
		var do_cycle_object = x_dir == 0;
		if (do_cycle_object) cycle_dir = 1;

		var found = false;
		for (var i = 1; i < len; ++i)
		{
			index = (index + cycle_dir) % len;
			if (index < 0) index += len;
			var is_same_object = interactions[index].Target == previous_interaction.Target;
			if (do_cycle_object == is_same_object)
			{
				found = true;

				// When cycling to the left, make sure to arrive at the first interaction for that object (and not the last).
				// Otherwise it's pretty unintuitive, why you sometimes grab and sometimes enter the catapult as the first interaction.
				if (x_dir == -1)
				{
					// Fast forward to first interaction.
					var target_object = interactions[index].Target;
					// It's guaranteed that the interactions are not split over the array borders. So we can just search until the index is 0.
					for (var current_index = index - 1; current_index >= 0; --current_index)
					{
						if (interactions[current_index].Target == target_object)
						{
							index = current_index;
						}
						else
						{
							break;
						}
					}
				}
				break;
			}
		}

		if (!found) index = -1;
	}
	else
	{
		// Find highest priority item.
		var high_prio = nil;
		for (var i = 0; i < len; ++i)
		{
			var interaction = interactions[i];
			if (high_prio != nil && interaction.priority <= high_prio.priority) continue;
			high_prio = interaction;
			index = i;
		}
	}

	if (index == -1) return nil;
	var next = interactions[index];
	if (DeepEqual(next, start_from)) return nil;
	return next;
}

func BeginInteract()
{
	this.control.interaction_hud_controller = this->GetHUDController();
	this.control.is_interacting = true;
	this.control.interaction_start_time = FrameCounter();

	// Force update the HUD controller, which is responsible for pre-selecting the "best" object.
	this.control.interaction_hud_controller->UpdateInteractionObject();
	// Then, iff the HUD shows an object, pre-select one.
	var interaction = this.control.interaction_hud_controller->GetCurrentInteraction();
	if (interaction)
		SetNextInteraction(interaction);
	this.control.interaction_hud_controller->EnableInteractionUpdating(false);
}

// Stops interaction selection without executing the current selection.
func AbortInteract()
{
	this.control.interaction_hud_controller->SetCurrentInteraction(nil);
	EndInteract();
}

func EndInteract()
{
	this.control.is_interacting = false;

	var executed = false;
	if (this.control.interaction_hud_controller->GetCurrentInteraction())
	{
		ExecuteInteraction(this.control.interaction_hud_controller->GetCurrentInteraction());
		executed = true;
	}

	var e = nil;
	while (e = GetEffect("IntHighlightInteraction", this))
	{
		if (executed)
			EffectCall(this, e, "OnExecute");
		RemoveEffect(nil, this, e);
	}

	this.control.interaction_hud_controller->SetCurrentInteraction(nil);
	this.control.interaction_hud_controller->EnableInteractionUpdating(true);
}

/**
	Wraps "PushBack", but also sets a flag when two interactions of the same object exist.
*/
func PushBackInteraction(array to, proplist interaction)
{
	PushBack(to, interaction);
	var count = 0;
	for (var other in to)
	{
		if (other.Target && (other.Target == interaction.Target))
		{
			count += 1;
			if (count > 1 || other != interaction)
			{
				other.has_multiple_interactions = true;
			}
		}
	}
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
func GetInteractableObjects(array sort)
{
	var possible_interactions = [];

	// All except structures only if outside
	var can_use_surrounding = !Contained();

	// Make sure that the Clonk's action target is always shown.
	// You can push a lorry out of your bounding box and would, otherwise, then be unable to release it.
	var main_criterion = Find_Distance(12);
	var action_target = GetActionTarget();
	if (action_target)
	{
		main_criterion = Find_Or(main_criterion, Find_InArray([action_target]));
	}

	// Add interactables (script interface)
	var interactables = FindObjects(
		main_criterion,
		Find_Func("IsInteractable"),
		Find_NoContainer(), Find_Layer(GetObjectLayer()),
		sort);
	for (var interactable in interactables)
	{
		var uses_container = interactable == Contained();
		if (can_use_surrounding || uses_container)
		{
			for (var interaction in interactable->~GetInteractions(this))
			{
				PushBackInteraction(possible_interactions, interaction);
			}
		}
	}

	return possible_interactions;
}

// Returns the number of interactable objects, which is different from the total number of available interactions.
func GetInteractableObjectsCount()
{
	var interactions = GetInteractableObjects();
	var interaction_objects = [];
	for (var interaction in interactions)
	{
		PushBack(interaction_objects, interaction.Target);
	}
	RemoveDuplicates(interaction_objects);
	return GetLength(interaction_objects);
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
			interaction.IsAvailable = interaction.Target->Call(condition, this);
		}
		return interaction.IsAvailable;
	}
	return  false;
}

// Executes interaction with an object. /action_info/ is a proplist as returned by GetInteractableObjects
func ExecuteInteraction(proplist interaction)
{
	if (HasInteraction(interaction))
	{
		interaction.Target->Call(interaction.Execute, this);
	}
}

static const Interaction = new Global 
{
	// These variables define the interaction
	
	Target = nil,     // object, target of the interaction
	Name = nil,       // string, name of the interaction
	Desc = nil,       // string, optional, description
	Condition = nil,  // function, optional, the interaction is available only when this condition is true
	Execute = nil,    // function, required, call this when executing the interaction
	
	// These variables are internal
	IsAvailable = false, // bool, result of evaluation the condition
	LastChecked = 0,     // int, frame when the condition was checked the last time
	Display = nil,       // object, floating message object
};

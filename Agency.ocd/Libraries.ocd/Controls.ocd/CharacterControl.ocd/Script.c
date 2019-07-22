/*
	Copied from Objects.ocd, because you cannot simply exchange one component.

	Will be adjusted as necessary.
*/

// make use of other sub-libraries
#include Character_Control_Inventory
#include Character_Control_Interaction
#include Library_ClonkMenuControl
#include Library_ClonkUseControl
#include Library_ClonkGamepadControl


/* ++++++++++++++++++++++++ Clonk Inventory Control ++++++++++++++++++++++++ */

/*
	used properties
	this.control.hotkeypressed: used to determine if an interaction has already been handled by a hotkey (space + 1-9)

	this.control.alt: alternate usage by right mouse button
	this.control.mlastx: last x position of the cursor
	this.control.mlasty: last y position of the cursor
*/




/* ################################################# */

protected func Construction()
{
	if (this.control == nil)
	{
		this.control = {};
	}
	this.control.hotkeypressed = false;

	this.control.alt = false;
	return _inherited(...);
}

protected func OnActionChanged(string oldaction)
{
	var old_act = this["ActMap"][oldaction];
	var act = this["ActMap"][GetAction()];
	var old_proc = 0;
	if (old_act)
	{
		old_proc = old_act["Procedure"];
	}
	var proc = 0;
	if (act)
	{
		proc = act["Procedure"];
	}
	// if the object's procedure has changed from a non Push/Attach
	// to a Push/Attach action or the other way round, the usage needs
	// to be cancelled
	if (proc != old_proc)
	{
		if (proc == DFA_PUSH || proc == DFA_ATTACH
		 || old_proc == DFA_PUSH || old_proc == DFA_ATTACH)
		{
			CancelUse();
		}
	}
	return _inherited(oldaction,...);
}


/* +++++++++++++++++++++++++++ Clonk Control +++++++++++++++++++++++++++ */

/* Main control function */
public func ObjectControl(int plr, int ctrl, int x, int y, int strength, bool repeat, int status)
{
	if (!this)
	{
		return false;
	}

	// Contents menu
	if (ctrl == CON_Contents && status == CONS_Down)
	{
		// Close any menu if open.
		if (GetMenu())
		{
			var is_content = GetMenu()->~IsContentMenu();
			// unclosable menu? bad luck
			if (!this->~TryCancelMenu())
			{
				return true;
			}
			// If contents menu, don't open new one and return.
			if (is_content)
			{
				return true;
			}
		}
		// Open contents menu.
		CancelUse();
		GUI_ObjectInteractionMenu->CreateFor(this, GUI_OIM_NewStyle);
		// the interaction menu calls SetMenu(this) in the clonk
		// so after this call menu = the created menu
		if (GetMenu())
		{
			GetMenu()->~Show();
		}
		return true;
	}

	/* aiming with mouse:
	   The CON_Aim control is transformed into a use command. Con_Use if
	   repeated does not bear the updated x,y coordinates, that's why this
	   other control needs to be issued and transformed. CON_Aim is a
	   control which is issued on mouse move but disabled when not aiming
	   or when HoldingEnabled() of the used item does not return true.
	   For the rest of the control code, it looks like the x,y coordinates
	   came from CON_Use.
	  */
	if (GetUsedObject() && ctrl == CON_Aim)
	{
		if (this.control.alt)
		{
			ctrl = CON_UseAlt;
		}
		else
		{
			ctrl = CON_Use;
		}

		repeat = true;
		status = CONS_Down;
	}
	// controls except a few reset a previously given command
	else if (status != CONS_Moved)
	{
		SetCommand("None");
	}

	/* aiming with analog pad or keys:
	   This works completely different. There are CON_AimAxis* and CON_Aim*,
	   both work about the same. A virtual cursor is created which moves in a
	   circle around the clonk and is controlled via these CON_Aim* functions.
	   CON_Aim* is normally on the same buttons as the movement and has a
	   higher priority, thus is called first. The aim is always done, even if
	   the clonk is not aiming. However this returns only true (=handled) if
	   the clonk is really aiming. So if the clonk is not aiming, the virtual
	   cursor aims into the direction in which the clonk is running and e.g.
	   CON_Left is still called afterwards. So if the clonk finally starts to
	   aim, the virtual cursor already aims into the direction in which he ran
	*/
	if (ctrl == CON_AimAxisUp || ctrl == CON_AimAxisDown || ctrl == CON_AimAxisLeft || ctrl == CON_AimAxisRight)
	{
		var success = VirtualCursor()->Aim(ctrl,this,strength,repeat,status);
		// in any case, CON_Aim* is called but it is only successful if the virtual cursor is aiming
		return success && VirtualCursor()->IsAiming();
	}

	// Simulate a mouse cursor for gamepads.
	if (HasVirtualCursor())
	{
		x = this.control.mlastx;
		y = this.control.mlasty;
	}
	// save last mouse position:
	// if the using has to be canceled, no information about the current x,y
	// is available. Thus, the last x,y position needs to be saved
	else if (ctrl == CON_Use || ctrl == CON_UseAlt)
	{
		this.control.mlastx = x;
		this.control.mlasty = y;
	}

	var proc = GetProcedure();

	// building, vehicle, mount, contents, menu control
	var house = Contained();
	var vehicle = GetActionTarget();
	// the clonk can have an action target even though he lost his action.
	// That's why the clonk may only interact with a vehicle if in an
	// appropiate procedure:
	if (proc != "ATTACH" && proc != "PUSH")
	{
		vehicle = nil;
	}

	// menu
	if (this.control.menu)
	{
		return Control2Menu(ctrl, x,y,strength, repeat, status);
	}

	var contents = this->GetActiveItem();

	// Usage
	var use = (ctrl == CON_Use || ctrl == CON_UseAlt);
	if (use)
	{
		if (house)
		{
			return ControlUse2Script(ctrl, x, y, strength, repeat, status, house);
		}
		// control to grabbed vehicle
		else if (vehicle && proc == DFA_PUSH)
		{
			return ControlUse2Script(ctrl, x, y, strength, repeat, status, vehicle);
		}
		else if (vehicle && proc == DFA_ATTACH)
		{
			/* objects to which clonks are attached (like horses, mechs,...) have
			   a special handling:
			   Use controls are, forwarded to the
			   horse but if the control is considered unhandled (return false) on
			   the start of the usage, the control is forwarded further to the
			   item. If the item then returns true on the call, that item is
			   regarded as the used item for the subsequent ControlUse* calls.
			   BUT the horse always gets the ControlUse*-calls that'd go to the used
			   item, too and before it so it can decide at any time to cancel its
			   usage via CancelUse().
			  */

			if (ControlUse2Script(ctrl, x, y, strength, repeat, status, vehicle))
			{
				return true;
			}
			else if (GetUsedObject() == vehicle)
			{
				// handled if the horse is the used object
				// ("using" is set to the object in StartUseControl - when the
				// object returns true on that callback. Exactly what we want)
				return true;
			}
		}
		// releasing the use-key always cancels shelved commands (in that case no GetUsedObject() exists)
		if (status == CONS_Up)
		{
			StopShelvedCommand();
		}
		// Release commands are always forwarded even if contents is 0, in case we
		// need to cancel use of an object that left inventory
		if (contents || (status == CONS_Up && GetUsedObject()))
		{
			if (ControlUse2Script(ctrl, x, y, strength, repeat, status, contents))
			{
				return true;
			}
		}
	}

	// A click on throw can also just abort usage without having any other effects.
	// todo: figure out if wise.
	var currently_in_use = GetUsedObject() != nil;
	if (ctrl == CON_Throw && currently_in_use && status == CONS_Down)
	{
		CancelUse();
		return true;
	}

	// Throwing and dropping
	// only if not in house, not grabbing a vehicle and an item selected
	// only act on press, not release
	if (ctrl == CON_Throw && !house && (!vehicle || proc == DFA_ATTACH || proc == DFA_PUSH) && status == CONS_Down)
	{
		if (contents)
		{
			// The object does not want to be dropped? Still handle command.
			if (contents->~QueryRejectDeparture(this))
			{
				return true;
			}

			// just drop in certain situations
			var only_drop = proc == DFA_SCALE || proc == DFA_HANGLE || proc == DFA_SWIM;
			// also drop if no throw would be possible anyway
			if (only_drop || Distance(0, 0, x, y) < 10 || (Abs(x) < 10 && y > 10))
			{
				only_drop = true;
			}
			// throw
			CancelUse();

			return ObjectCommand("Drop", contents);
		}
	}

	// Movement controls (defined in PlayerControl.c, partly overloaded here)
	if (ctrl == CON_Left || ctrl == CON_Right || ctrl == CON_Up || ctrl == CON_Down || ctrl == CON_Jump)
	{
		// forward to script...
		if (house)
		{
			return ControlMovement2Script(ctrl, x, y, strength, repeat, status, house);
		}
		else if (vehicle)
		{
			if (ControlMovement2Script(ctrl, x, y, strength, repeat, status, vehicle)) return true;
		}

		return ObjectControlMovement(plr, ctrl, strength, status);
	}

	// Fall through half-solid mask
	if (ctrl == CON_FallThrough)
	{
		if (status == CONS_Down)
		{
			if (this->IsWalking())
			{
				HalfVehicleFadeJumpStart();
			}
		}
		else
		{
			HalfVehicleFadeJumpStop();
		}
		return true;
	}

	// Unhandled control
	return _inherited(plr, ctrl, x, y, strength, repeat, status, ...);
}

// A wrapper to SetCommand to catch special behaviour for some actions.
public func ObjectCommand(string command, object target, int tx, int ty, object target2, /*any*/ data)
{
	// special control for throw and jump
	// but only with controls, not with general commands
	if (command == "Jump")
	{
		return this->~ControlJump();
	}
	// else standard command
	else
	{
		// Make sure to not recollect the item immediately on drops.
		if (command == "Drop" && target)
		{
			// Disable collection for a moment.
			this->OnDropped(target);
		}
		return SetCommand(command, target, tx, ty, target2, data);
	}
	// this function might be obsolete: a normal SetCommand does make a callback to
	// script before it is executed: ControlCommand(szCommand, pTarget, iTx, iTy)
}

/*
	Called by the engine before a command is executed.
	Beware that this is NOT called when SetCommand was called by a script.
	At this point I am not sure whether we need this callback at all.
*/
public func ControlCommand(string command, object target, int tx, int ty)
{
	if (command == "Drop")
	{
		// Disable collection for a moment.
		if (target)
		{	
			this->OnDropped(target);
		}
	}
	return _inherited(command, target, tx, ty, ...);
}

/* ++++++++++++++++++++++++ Movement Controls ++++++++++++++++++++++++ */

// Control use redirected to script
func ControlMovement2Script(int ctrl, int x, int y, int strength, bool repeat, int status, object obj)
{
	// overloads of movement commandos
	if (ctrl == CON_Left || ctrl == CON_Right || ctrl == CON_Down || ctrl == CON_Up || ctrl == CON_Jump)
	{
		var control_string = "Control";
		if (Contained() == obj)
		{
			control_string = "Contained";
		}
		if (status == CONS_Up)
		{
			// if any movement key has been released, ControlStop is called
			if (obj->Call(Format("~%sStop", control_string), this, ctrl))
			{
				return true;
			}
		}
		else
		{
			// what about gamepad-deadzone?
			if (strength != nil && strength < CON_Gamepad_Deadzone)
			{
				return true;
			}
			// Control*
			if (ctrl == CON_Left)  if (obj->Call(Format("~%sLeft",control_string),this))  return true;
			if (ctrl == CON_Right) if (obj->Call(Format("~%sRight",control_string),this)) return true;
			if (ctrl == CON_Up)    if (obj->Call(Format("~%sUp",control_string),this))    return true;
			if (ctrl == CON_Down)  if (obj->Call(Format("~%sDown",control_string),this))  return true;

			// for attached (e.g. horse: also Jump command
			if (GetProcedure() == "ATTACH"
			&&  ctrl == CON_Jump
			&&  obj->Call("ControlJump",this))
			{
				return true;
			}
		}
	}

}

// Effect to free/unfree hands by disabling/enabling scale and hangle procedures
public func FxIntControlFreeHandsStart(object target, proplist fx, int temp)
{
	// Process on non-temp as well in case scale/handle effects need to stack
	// Stop current action
	var proc = GetProcedure();
	if (proc == DFA_SCALE || proc == DFA_HANGLE)
	{
		SetAction("Walk");
	}
	// Make sure ActMap is writable
	if (this.ActMap == this.Prototype.ActMap)
	{
		this.ActMap = new this.ActMap{};
	}
	// Kill scale/hangle effects
	fx.act_scale = this.ActMap.Scale;
	this.ActMap.Scale = nil;
	fx.act_hangle = this.ActMap.Hangle;
	this.ActMap.Hangle = nil;
	return FX_OK;
}

public func FxIntControlFreeHandsStop(object target, proplist fx, int reason, bool temp)
{
	// Restore scale/hangle effects (engine will handle re-grabbing walls if needed)
	if (fx.act_scale)
	{
		this.ActMap.Scale = fx.act_scale;
	}
	if (fx.act_hangle)
	{
		this.ActMap.Hangle = fx.act_hangle;
	}
	return FX_OK;
}

// returns true if the clonk is able to enter a building (procedurewise)
public func CanEnter()
{
	var proc = GetProcedure();
	return proc == DFA_WALK
	    || proc == DFA_PUSH
	    || proc == DFA_SWIM
	    || proc == DFA_SCALE
	    || proc == DFA_HANGLE
	    || proc == DFA_FLOAT
	    || proc == DFA_FLIGHT;
}



/*-- Jumping --*/


/*
 Triggers a regular jump, that means that the speed in y direction
 is automatically decided, depending on the action of the clonk.

 If you want to execute a jump with a certain speed, use ControlJumpExecute().
 */
public func ControlJump()
{
	var ydir = 0;

	if (GetProcedure() == "WALK")
	{
		ydir = this.JumpSpeed;
	}

	if (InLiquid() && !GBackSemiSolid(0, -5))
	{
		ydir = BoundBy(this.JumpSpeed * 3 / 5, 240, 380);
	}

	// Jump speed of the wall kick is halved.
	if (GetProcedure() == "SCALE" || GetAction() == "Climb")
	{
		ydir = this.JumpSpeed / 2;
	}

	return ControlJumpExecute(ydir);
}


/*
 Additional function for actually triggering a jump directly.

 The parameter ydir can be decided directly by the user,
 or you can use the clonk's jump speed by passing this.JumpSpeed

 Returns false if the jump was not successful.
 */
public func ControlJumpExecute(int ydir)
{
	if (ydir && !Stuck())
	{
		SetPosition(GetX(), GetY() - 1);

		// Wall kick if scaling or climbing.
		if (GetProcedure() == "SCALE" || GetAction() == "Climb")
		{
			AddEffect("WallKick", this, 1);
			var xdir;
			if (GetDir() == DIR_Right)
			{
				xdir = -1;
				SetDir(DIR_Left);
			}
			else if (GetDir() == DIR_Left)
			{
				xdir = 1;
				SetDir(DIR_Right);
			}

			SetYDir(-ydir * GetCon(), 100 * 100);
			SetXDir(xdir * 17);
			// Set speed first to have proper animations when jump starts.
			SetAction("Jump");
			return true;
		}
		//Normal jump
		else
		{
			SetYDir(-ydir * GetCon(), 100 * 100);
			// Set speed first to have proper animations when jump starts.
			SetAction("Jump");
			return true;
		}
	}
	return false;
}


// Interaction with clonks is special:
// * The clonk opening the menu should always have higher priority so the clonk is predictably selected on the left side even if standing behind e.g. a crate
// * Other clonks should be behind because interaction with them is rare but having your fellow players stand in front of a building is very common
//   (Allies also tend to run in front just when you opened that menu...)
func GetInteractionPriority(object target)
{
	// Self with high priority
	if (target == this)
	{
		return 100;
	}
	// Dead Clonks are shown (for a death message e.g.) but sorted to the bottom.
	if (!GetAlive())
	{
		return -190;
	}
	var owner = NO_OWNER;
	if (target)
	{
		owner = target->GetOwner();
	}
	// Prefer own clonks for item transfer
	if (owner == GetOwner())
	{
		return -100;
	}
	// If no own clonk, prefer friendly
	if (!Hostile(owner, GetOwner()))
	{	
		return -120;
	}
	// Hostile clonks? Lowest priority.
	return -200;
}

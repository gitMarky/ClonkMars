/**
	Door control

	This object supplies the base functionality for structures with doors.
	Closing and opening the door is handled by an effect.
	
*/

static const ANIM_SLOT_Door = 1;


/* -- Engine callbacks -- */

public func Initialize()
{
	SetEntrance(false);
	return _inherited(...);
}


public func ActivateEntrance(object entering_obj)
{
	// block enemies?
	if (this->~CanBlockEnemies())
	{
		var for_plr = entering_obj->GetOwner();
		if (Hostile(GetOwner(), for_plr))
		{
			entering_obj->~PlaySoundDecline();
			PlayerMessage(for_plr, "$TxtNoEntryEnemy$", GetPlayerName(GetOwner()));
			return false;
		}
	}

	// open door
	if (!GetEffect("FxDoorControl", this))
	{
		CreateEffect(FxDoorControl, 1, 1);
	}

	return true;
}


public func Ejection(object obj)
{
	// Let the door stay open longer on ejection.
	KeepOpen();
	return _inherited(obj, ...);
}


public func Collection2(object obj)
{
	// Let the door stay open longer on collection.
	KeepOpen();
	return _inherited(obj, ...);
}

/* -- Internals -- */

private func OpenEntrance()
{
	SetEntrance(true);
}


private func CloseEntrance()
{
	SetEntrance(false);
}


private func KeepOpen()
{
	var control = GetEffect("FxDoorControl", this);
	
	if (control)
	{
		control->StayOpen();
	}
}


local FxDoorControl = new Effect
{
	Construction = func()
	{
		var default_delay = 20;
	
		this.time_opening = GetActionLength("OpenDoor")  ?? (Target.DoorTimeToOpen ?? default_delay);
		this.time_opened =  GetActionLength("DoorOpen")  ?? (Target.DoorTimeIsOpen ?? default_delay);
		this.time_closing = GetActionLength("CloseDoor") ?? (Target.DoorTimeToClose ?? default_delay);
		this.time_open = this.time_opened;
		
		this.is_open = false;
		this.sound_open = true;
		this.sound_close = true;
		this.anim_open = true;
		this.anim_opened = true;
		this.anim_close = true;
	},

	Timer = func()
	{
		if (this.time_opening > 0)
		{
			this.time_opening -= 1;
			if (this.sound_open)
			{
				this.sound_open = false;
				Target->SoundOpenDoor();	
			}
			if (this.anim_open)
			{
				DoorAnimation("DoorOpen", 0, this.time_opening);
				this.anim_open = false;
			}
		}
		else if (this.time_open > 0)
		{
			if (!this.is_open)
			{
				Target->OpenEntrance();
				this.is_open = true;
			}

			this.time_open -= 1;

			if (this.anim_opened)
			{
				DoorAnimation("DoorOpened", 1, this.time_open);
				this.anim_opened = false;
			};
		}
		else if (this.time_closing > 0)
		{
			this.time_closing -= 1;
			if (this.is_open)
			{
				Target->CloseEntrance();
				this.is_open = false;
			}
			if (this.sound_close)
			{
				this.sound_close = false;
				Target->SoundCloseDoor();
			}
			if (this.anim_close)
			{
				DoorAnimation("DoorClose", 2, this.time_closing);
				this.anim_close = false;
			}
		}
		else
		{
			return FX_Execute_Kill;
		}
	},

	Destruction = func()
	{
		Target->SetEntrance(false);
	},

	GetActionLength = func(string name)
	{
		if (Target.ActMap && Target.ActMap[name]) 
		{
			return (Target.ActMap[name].Delay ?? 1) * (Target.ActMap[name].Length ?? 1);
		}
		else
		{
			return nil;
		}
	},

	StayOpen = func()
	{
		if (this.is_open)
		{
			this.time_open = this.time_opened;
		}
	},
	
	DoorAnimation = func(string animation, int slot, int duration)
	{
		Target->PlayAnimation(animation, ANIM_SLOT_Door + slot, Anim_Linear(0, 0, Target->GetAnimationLength(animation), duration + 1, ANIM_Remove), Anim_Linear(0, 0, 1000, 1, ANIM_Remove));
	},
};


/* -- Internal callbacks that can be overloaded */


private func SoundOpenDoor()
{
	Sound("Structures::DoorOpen?");
}


private func SoundCloseDoor()
{
	Sound("Structures::DoorClose?");
}

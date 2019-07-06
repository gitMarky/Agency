#include Task_Template

public func Execute(proplist controller, object agent)
{
	if (FrameCounter() % 10 == 0)
	{
		// Actions on walking.
		if (agent->IsWalking()) 
		{
			// Change direction of walking from time to time.
			if (!Random(5))
			{
				agent->SetComDir([COMD_Left, COMD_Right][Random(2)]);
				return TASK_EXECUTION_IN_PROGRESS;
			}
	
			// Stop walking from time to time.
			if (!Random(20) && agent->GetComDir() != COMD_Stop)
			{
				agent->SetComDir(COMD_Stop);
				return TASK_EXECUTION_IN_PROGRESS;
			}
		}
	}

	return TASK_EXECUTION_IN_PROGRESS;
}

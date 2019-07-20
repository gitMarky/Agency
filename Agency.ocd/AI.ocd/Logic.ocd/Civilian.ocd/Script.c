#include AI_Controller // Include the basic functionality
#include AI_TaskExecutor // Main execution logic


func OnAddAI(proplist controller)
{
	_inherited(controller);

	controller->SetAgent(AI_Agent_Human); // Sets the agent logic and capabilities

	Task_RandomWalk->AddTo(controller.Target, 1);
}


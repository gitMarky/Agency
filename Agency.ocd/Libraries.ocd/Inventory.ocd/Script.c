/**
	Inventory
*/

#include Library_Inventory // for now, so that the inventory display still works as intended.


func GetHandItem(int i) // this should not have a parameter, but the default implementation forces you to pass 0, or else it returns nil
{
	return _inherited(i ?? 0, ...);
}

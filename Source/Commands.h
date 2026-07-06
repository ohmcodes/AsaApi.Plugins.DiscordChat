

void AddOrRemoveCommands(bool addCmd = true)
{
	if (addCmd)
	{
		AsaApi::GetCommands().AddOnChatMessageCallback("ChatMessageCallback", &ChatMessageCallback);
	}
	else
	{
		AsaApi::GetCommands().RemoveOnChatMessageCallback("ChatMessageCallback");
	}
}
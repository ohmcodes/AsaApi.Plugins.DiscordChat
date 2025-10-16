

void AddOrRemoveCommands(bool addCmd = true)
{
	const FString RepairItems = DiscordGlobalChat::config["Commands"]["RepairItemCMD"].get<std::string>().c_str();
	if (!RepairItems.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(RepairItems, &RepairItemsCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(RepairItems);
		}
	}

	const FString DeletePlayer = DiscordGlobalChat::config["Commands"]["DeletePlayerCMD"].get<std::string>().c_str();
	if (!DeletePlayer.IsEmpty())
	{
		if (addCmd)
		{
			AsaApi::GetCommands().AddChatCommand(DeletePlayer, &DeletePlayerCallback);
		}
		else
		{
			AsaApi::GetCommands().RemoveChatCommand(DeletePlayer);
		}
	}
}
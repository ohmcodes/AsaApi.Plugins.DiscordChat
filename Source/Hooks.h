

DECLARE_HOOK(AShooterPlayerController_ClientChatMessage, void, AShooterPlayerController*, FPrimalChatMessage*);
void Hook_AShooterPlayerController_ClientChatMessage(AShooterPlayerController* _this, FPrimalChatMessage* Chat)
{
    /*if (DiscordGlobalChat::isDebug == true)
        Log::GetLog()->info("Hook_AShooterPlayerController_ClientChatMessage() Called");*/

    /*FPrimalChatMessage{
    FString SenderName;
    FString SenderSteamName;
    FString SenderTribeName;
    unsigned int SenderId;
    FString Message;
    FString Receiver;
    int SenderTeamIndex;
    long double ReceivedTime;
    TEnumAsByte<enum EChatSendMode::Type> SendMode;
    unsigned int RadioFrequency;
    TEnumAsByte<enum EChatType::Type> ChatType;
    UTexture2D* SenderIcon;
    unsigned __int8 senderPlatform;
    FString UserId;
    unsigned __int8 SenderIsAdmin : 1;*/

    /*Log::GetLog()->info("SenderName {} SenderSteamName {} SenderTribeName {} SenderId {} Receiver {} SenderTeamIndex {} ReceivedTime {} RadioFreq {} senderPlatform {} UserId {} SenderIsAdmin {}",
        Chat->SenderName.ToString(),
        Chat->SenderSteamName.ToString(),
        Chat->SenderTribeName.ToString(),
        std::to_string(Chat->SenderId),
        Chat->Receiver.ToString(),
        std::to_string(Chat->SenderTeamIndex),
        std::to_string(Chat->ReceivedTime),
        std::to_string(Chat->RadioFrequency),
        std::to_string(Chat->senderPlatform),
        Chat->UserId.ToString(),
        std::to_string(Chat->SenderIsAdmin));*/

    LogChat(Chat);

    SendMessageToDiscord(Chat->Message.ToString());

    /*Log::GetLog()->info("SenderName {}",
        Chat->SenderName.ToString());

    Log::GetLog()->info("SenderSteamName {}",
        Chat->SenderSteamName.ToString());

    Log::GetLog()->info("SenderTribeName {}",
        Chat->SenderTribeName.ToString());

    Log::GetLog()->info("SenderId {}",
        std::to_string(Chat->SenderId));

    Log::GetLog()->info("Receiver {}",
        Chat->Receiver.ToString());

    Log::GetLog()->info("SenderTeamIndex {}",
        std::to_string(Chat->SenderTeamIndex));

    Log::GetLog()->info("ReceivedTime {}",
        std::to_string(Chat->ReceivedTime));

    Log::GetLog()->info("RadioFreq {}",
        std::to_string(Chat->RadioFrequency));

    Log::GetLog()->info("senderPlatform {}",
        std::to_string(Chat->senderPlatform));

    Log::GetLog()->info("UserId {}",
        Chat->UserId.ToString());

    Log::GetLog()->info("SenderIsAdmin {}",
        std::to_string(Chat->SenderIsAdmin));

    Log::GetLog()->info("Log after");*/

    if (Chat->SendMode != EChatSendMode::GlobalChat && Chat->SendMode != EChatSendMode::GlobalTribeChat)
    {

        
        
        /*if (DiscordGlobalChat::config["General"].value("ShowIcons", false) && (Chat->senderPlatform != 2 && Chat->senderPlatform != 3))
        {
            Chat->SenderIcon = GetPlayerIcons(Chat, _this);
        }
        else
        {
            Chat->SenderIcon = nullptr;
        }*/

        //SendDiscord(Chat);
        //LogChat(Chat);
    }

    // override ally and local chat chat message color
    /*if ((Chat->SendMode == EChatSendMode::AllianceChat || Chat->ChatType == EChatType::AllianceChat) || (Chat->SendMode == EChatSendMode::LocalChat || Chat->ChatType == EChatType::ProximityChat))
    {
        Chat->Message = ParseEmoticons(Chat->SendMode, &Chat->Message);
    }*/

    AShooterPlayerController_ClientChatMessage_original(_this, Chat);
}

void SetHooks(bool addHooks = true)
{
	if (addHooks)
	{
        AsaApi::GetHooks().SetHook("AShooterPlayerController.ClientChatMessage(FPrimalChatMessage)", &Hook_AShooterPlayerController_ClientChatMessage, &AShooterPlayerController_ClientChatMessage_original);
	}
	else
	{
        AsaApi::GetHooks().DisableHook("AShooterPlayerController.ClientChatMessage(FPrimalChatMessage)", &Hook_AShooterPlayerController_ClientChatMessage);
	}

}
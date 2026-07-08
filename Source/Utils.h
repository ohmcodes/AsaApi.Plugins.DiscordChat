
#include <fstream>
#include <regex>


static bool startsWith(const std::string& str, const std::string& prefix)
{
	return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

void FetchMessageFromDiscordCallback(bool success, std::string results)
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	if (success)
	{
		if(results.empty()) return;

		nlohmann::json parsed = nlohmann::json::parse(results);

		if (parsed.is_null())
		{
			Log::GetLog()->warn("resObj is null");
			return;
		}
		nlohmann::json  resObj = parsed[0];

		auto globalName = resObj["author"]["global_name"];

		// if not sent by bot
		if (resObj.contains("bot") && globalName.is_null())
		{
			Log::GetLog()->warn("the sender is bot");
			return;
		}

		std::string msg = resObj["content"].get<std::string>();

		//if (!startsWith(msg, "!"))
		//{
		//	//Log::GetLog()->warn("message not startswith !");
		//	return;
		//}

		if (DiscordGlobalChat::lastMessageID == resObj["id"].get<std::string>()) return;

		std::string sender = fmt::format("Discord: {}", globalName.get<std::string>());

		AsaApi::GetApiUtils().SendChatMessageToAll(FString(sender), msg.c_str());

		DiscordGlobalChat::lastMessageID = resObj["id"].get<std::string>();

		//try
		//{
		//	
		//}
		//catch (const std::exception& error)
		//{
		//	//Log::GetLog()->error("Error parsing JSON results. Error: {}",error.what());
		//}
	}
	else
	{
		Log::GetLog()->warn("Failed to fetch messages. success: {}", success);
	}
}

void FetchMessageFromDiscord()
{
	//Log::GetLog()->warn("Function: {}", __FUNCTION__);
	// TODO: Validate token and channelID

	std::string botToken = DiscordGlobalChat::config["DiscordBot"].value("BotToken","");

	if (botToken.empty()) return;

	std::string channelID = DiscordGlobalChat::config["DiscordBot"].value("ChannelID", "");

	if (channelID.empty()) return;

	std::string apiURL = FString::Format("https://discord.com/api/v10/channels/{}/messages?limit=1", channelID).ToString();


	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: DiscordGlobalChat/1.0",
		"Connection: keep-alive",
		"Accept: */*",
		"Content-Length: 0",
		"Authorization: Bot " + botToken
	};

	bool req = DiscordGlobalChat::req.CreateGetRequest(apiURL, FetchMessageFromDiscordCallback, headers);

	if (!req)
		Log::GetLog()->error("Failed to perform Get request. req = {}", req);

	/*try
	{
		
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to perform Get request. Error: {}", error.what());
	}*/
}

void SendMessageToDiscordCallback(bool success, std::string results, std::unordered_map<std::string, std::string> responseHeaders)
{
	if (!success)
	{
		Log::GetLog()->error("Failed to send Post request. {} {} {}", __FUNCTION__, success, results);
	}
	else
	{
		Log::GetLog()->info("Success. {} {} {}", __FUNCTION__, success, results);
	}
}

void SendMessageToDiscord(FString msg)
{

	// Note: This cant be on Hooks or else every messages will be send to discord

	//Log::GetLog()->warn("Function: {}", __FUNCTION__);

	if (msg.StartsWith("Discord:")) { return; }

	
	std::string webhook = DiscordGlobalChat::config["DiscordBot"].value("Webhook", "");
	std::string botImgUrl = DiscordGlobalChat::config["DiscordBot"].value("BotImageURL", "");

	if (webhook == "" || webhook.empty()) return;

	FString msgFormat = L"{{\"content\":\"{}\",\"username\":\"{}\",\"avatar_url\":\"{}\"}}";

	FString msgOutput = FString::Format(*msgFormat, msg.ToString(), "ArkBot", botImgUrl);

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: DiscordGlobalChat/1.0",
		"Connection: keep-alive",
		"Accept: */*"
	};

	try
	{
		bool req = DiscordGlobalChat::req.CreatePostRequest(webhook, SendMessageToDiscordCallback, msgOutput.ToStringUTF8(), "application/json", headers);

		if(!req)
			Log::GetLog()->error("Failed to send Post request. req = {}", req);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("Failed to send Post request. Error: {}", error.what());
	}
}

bool Points(FString eos_id, int cost, bool check_points = false)
{
	if (cost == -1)
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is -1");
		}
		return false;
	}

	if (cost == 0)
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Cost is 0");
		}

		return true;
	}

	nlohmann::json config = DiscordGlobalChat::config["PointsDBSettings"];

	if (config.value("Enabled", false) == false)
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Points system is disabled");
		}

		return true;
	}

	std::string tablename = config.value("TableName", "ArkShopPlayers");
	std::string unique_id = config.value("UniqueIDField", "EosId");
	std::string points_field = config.value("PointsField", "Points");
	std::string totalspent_field = config.value("TotalSpentField", "TotalSpent");

	if (tablename.empty() || unique_id.empty() || points_field.empty())
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("DB Fields are empty");
		}
		return false;
	}

	std::string escaped_eos_id = DiscordGlobalChat::pointsDB->escapeString(eos_id.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}'", tablename, unique_id, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;

	if (!DiscordGlobalChat::pointsDB->read(query, results))
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Error reading points db");
		}

		return false;
	}

	if (results.size() <= 0)
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("No record found");
		}
		return false;
	}

	int points = std::atoi(results[0].at(points_field).c_str());

	if (check_points)
	{
		if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
		{
			Log::GetLog()->warn("Player got {} points", points);
		}

		if (points >= cost) return true;
	}
	else
	{
		int amount = points - cost;

		std::vector<std::pair<std::string, std::string>> data;

		data.push_back({ points_field, std::to_string(amount) });

		if (totalspent_field != "")
		{
			int total_spent = std::atoi(results[0].at(totalspent_field).c_str());
			std::string total_ts = std::to_string(total_spent + cost);

			data.push_back({totalspent_field, total_ts});
		}

		std::string condition = fmt::format("{}='{}'", unique_id, escaped_eos_id);

		if (DiscordGlobalChat::pointsDB->update(tablename, data, condition))
		{
			if (DiscordGlobalChat::config["Debug"].value("Points", false) == true)
			{
				Log::GetLog()->info("{} Points DB updated", amount);
			}

			return true;
		}
	}

	return false;
}

nlohmann::json GetCommandString(const std::string permission, const std::string command)
{
	if (permission.empty()) return {};
	if (command.empty()) return {};

	nlohmann::json config_obj = DiscordGlobalChat::config["PermissionGroups"];
	nlohmann::json perm_obj = config_obj[permission];
	nlohmann::json command_obj = perm_obj["Commands"];
	nlohmann::json setting_obj = command_obj[command];

	return setting_obj;
}

TArray<FString> GetPlayerPermissions(FString eos_id)
{
	TArray<FString> PlayerPerms = { "Default" };

	std::string escaped_eos_id = DiscordGlobalChat::permissionsDB->escapeString(eos_id.ToString());

	std::string tablename = DiscordGlobalChat::config["PermissionsDBSettings"].value("TableName", "Players");

	std::string condition = DiscordGlobalChat::config["PermissionsDBSettings"].value("UniqueIDField", "EOS_Id");

	std::string query = fmt::format("SELECT * FROM {} WHERE {}='{}';", tablename, condition, escaped_eos_id);

	std::vector<std::map<std::string, std::string>> results;
	if (!DiscordGlobalChat::permissionsDB->read(query, results))
	{
		if (DiscordGlobalChat::config["Debug"].value("Permissions", false) == true)
		{
			Log::GetLog()->warn("Error reading permissions DB");
		}

		return PlayerPerms;
	}

	if (results.size() <= 0) return PlayerPerms;

	std::string permsfield = DiscordGlobalChat::config["PermissionsDBSettings"].value("PermissionGroupField","PermissionGroups");

	FString playerperms = FString(results[0].at(permsfield));

	if (DiscordGlobalChat::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("current player perms {}", playerperms.ToString());
	}

	playerperms.ParseIntoArray(PlayerPerms, L",", true);

	return PlayerPerms;
}

FString GetPriorPermByEOSID(FString eos_id)
{
	TArray<FString> player_groups = GetPlayerPermissions(eos_id);

	const nlohmann::json permGroups = DiscordGlobalChat::config["PermissionGroups"];

	std::string defaultGroup = "Default";
	int minPriority = INT_MAX;
	nlohmann::json result;
	FString selectedPerm = "Default";

	for (const FString& param : player_groups)
	{
		if (permGroups.contains(param.ToString()))
		{
			int priority = static_cast<int>(permGroups[param.ToString()]["Priority"]);
			if (priority < minPriority)
			{
				minPriority = priority;
				result = permGroups[param.ToString()];
				selectedPerm = param;
			}
		}
	}

	if (result.is_null() && permGroups.contains(defaultGroup))
	{
		if(!permGroups[defaultGroup].is_null())
			result = permGroups[defaultGroup];

		result = {};
	}

	if (DiscordGlobalChat::config["Debug"].value("Permissions", false) == true)
	{
		Log::GetLog()->info("Selected Permission {}", selectedPerm.ToString());
	}

	return selectedPerm;
}

bool CheckPlayerGroupIfExists(FString eos_id)
{
	TArray<FString> player_groups = GetPlayerPermissions(eos_id);
	const nlohmann::json permGroups = DiscordGlobalChat::config["General"]["GroupIcons"];

	for (const FString& param : player_groups)
	{
		if (permGroups.contains(param.ToString()))
		{
			return true;
		}
	}
}

bool AddPlayer(FString eosID, int playerID, FString playerName, int platform)
{
	std::vector<std::pair<std::string, std::string>> data = {
		{"EosId", eosID.ToString()},
		{"PlayerId", std::to_string(playerID)},
		{"PlayerName", playerName.ToString()},
		{"Platform", std::to_string(platform)}
	};

	return DiscordGlobalChat::pluginTemplateDB->create(DiscordGlobalChat::config["PluginDBSettings"]["TableName"].get<std::string>(), data);
}

bool ReadPlayer(FString eosID)
{
	std::string escaped_id = DiscordGlobalChat::pluginTemplateDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT * FROM {} WHERE EosId='{}'", DiscordGlobalChat::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	DiscordGlobalChat::pluginTemplateDB->read(query, results);

	return results.size() <= 0 ? false : true;
}

bool UpdatePlayer(FString eosID, FString playerName)
{
	std::string unique_id = "EosId";

	std::string escaped_id = DiscordGlobalChat::pluginTemplateDB->escapeString(eosID.ToString());

	std::vector<std::pair<std::string, std::string>> data = {
		{"PlayerName", playerName.ToString() + "123"}
	};

	std::string condition = fmt::format("{}='{}'", unique_id, escaped_id);

	return DiscordGlobalChat::pluginTemplateDB->update(DiscordGlobalChat::config["PluginDBSettings"]["TableName"].get<std::string>(), data, condition);
}

bool DeletePlayer(FString eosID)
{
	std::string escaped_id = DiscordGlobalChat::pluginTemplateDB->escapeString(eosID.ToString());

	std::string condition = fmt::format("EosId='{}'", escaped_id);

	return DiscordGlobalChat::pluginTemplateDB->deleteRow(DiscordGlobalChat::config["PluginDBSettings"]["TableName"].get<std::string>(), condition);
}

int GetPlayerPlatform(FString eosID)
{
	std::string escaped_id = DiscordGlobalChat::pluginTemplateDB->escapeString(eosID.ToString());

	std::string query = fmt::format("SELECT Platform FROM {} WHERE EosId='{}'", DiscordGlobalChat::config["PluginDBSettings"]["TableName"].get<std::string>(), escaped_id);

	std::vector<std::map<std::string, std::string>> results;
	DiscordGlobalChat::pluginTemplateDB->read(query, results);

	// Check if we got any results
	if (!results.empty() && results[0].find("Platform") != results[0].end())
	{
		// Convert the string value to int
		return std::stoi(results[0]["Platform"]);
	}

	// Return a default value if no results found
	return -1; // or 0, depending on your use case
}



/* Plugin Utils */

std::vector<std::string> splitString(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(str);
	while (std::getline(tokenStream, token, delimiter)) {
		tokens.push_back(token);
	}
	return tokens;
}

FString GetPlayerColors(AShooterPlayerController* player_controller)
{
	if (DiscordGlobalChat::isDebug == true)
		Log::GetLog()->info("GetPlayerColors() Called");
	FString color = "";

	FString senderName = AsaApi::GetApiUtils().GetCharacterName(player_controller);

	nlohmann::json groups = DiscordGlobalChat::config["General"].value("GroupColors", nlohmann::json().object());
	// default look for group colors
	FString eos_id = player_controller->GetEOSId();

	FString player_group = GetPriorPermByEOSID(eos_id);

	for (auto& group_name : groups.items())
	{
		if (player_group.Contains(group_name.key().c_str()))
		{
			std::string val = group_name.value();
			color = val.c_str();
			break;
		}
	}

	// override player chat color if specified
	nlohmann::json jsonData = DiscordGlobalChat::config["General"].value("Colors", nlohmann::json().object());

	for (auto& item : jsonData.items())
	{
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->info("SenderName {} Key {}", senderName.ToString(), item.key());

		if (senderName.Contains(item.key().c_str()))
		{
			std::string val = item.value();

			if (DiscordGlobalChat::isDebug == true)
				Log::GetLog()->info("Color {}", val);

			color = val.c_str();
			break;
		}
	}

	return color;
}

EChatSendMode::Type GetChatSendModeIntType(int SendMode)
{
	/*GlobalChat = 0x0,
	GlobalTribeChat = 0x1,
	LocalChat = 0x2,
	AllianceChat = 0x3,
	MAX = 0x4*/
	EChatSendMode::Type chatEnum;
	switch (SendMode)
	{
	case 0:
		chatEnum = EChatSendMode::GlobalChat;
		break;
	case 1:
		chatEnum = EChatSendMode::GlobalTribeChat;
		break;
	case 2:
		chatEnum = EChatSendMode::LocalChat;
		break;
	case 3:
		chatEnum = EChatSendMode::AllianceChat;
		break;
	default:
		break;
	}
	return chatEnum;
}

EChatType::Type GetChatTypeIntType(int ChatType)
{

	EChatType::Type chatEnum;
	switch (ChatType)
	{
	case 0:
		chatEnum = EChatType::GlobalChat;
		break;
	case 1:
		chatEnum = EChatType::ProximityChat;
		break;
	case 2:
		chatEnum = EChatType::GlobalTribeChat;
		break;
	case 3:
		chatEnum = EChatType::AllianceChat;
		break;
	default:
		break;
	}
	return chatEnum;
}

FString GetChatTypeIntString(int ChatType)
{
	/*GlobalChat = 0x0,
	ProximityChat = 0x1,
	RadioChat = 0x2,
	GlobalTribeChat = 0x3,
	AllianceChat = 0x4,*/
	FString chatEnum;
	switch (ChatType)
	{
	case 0:
		chatEnum = "Global";
		break;
	case 1:
		chatEnum = "TRIBE";
		break;
	case 2:
		chatEnum = "LOCAL";
		break;
	case 3:
		chatEnum = "ALLY";
		break;
	default:
		break;
	}
	return chatEnum;
}

std::string GetIconPath(FString SenderName, AShooterPlayerController* player_controller)
{
	if (DiscordGlobalChat::isDebug == true)
		Log::GetLog()->warn("GetIconPath() Called");

	std::string icon = "";

	FString player_group = GetPriorPermByEOSID(player_controller->GetEOSId());

	// default icons in permission
	nlohmann::json groups = DiscordGlobalChat::config["General"].value("GroupIcons", nlohmann::json().object());
	// default look for group colors
	FString eos_id = AsaApi::GetApiUtils().GetEOSIDFromController(player_controller);

	for (auto& group_name : groups.items())
	{
		if(player_group.Contains(group_name.key().c_str()))
		{
			icon = group_name.value();
			break;
		}
	}

	// override icons
	nlohmann::json jsonData = DiscordGlobalChat::config.value("Icons", nlohmann::json().object());

	for (auto& item : jsonData.items()) {
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->warn("SenderName {} Key {}", SenderName.ToString(), item.key());

		if (SenderName.Contains(item.key().c_str()))
		{
			icon = item.value();

			if (DiscordGlobalChat::isDebug == true)
				Log::GetLog()->warn("Path {}", icon);

			break;
		}
	}

	return icon;
}

UTexture2D* GetPlayerIcons(FPrimalChatMessage* Chat, AShooterPlayerController* player_controller)
{
	UTexture2D* PlayerIcon{};

	std::string generic_icon = "/Game/PrimalEarth/UI/Icons/Crossplay_icons/Crossplay_Generic.Crossplay_Generic";
	std::string steam_icon = "/Game/PrimalEarth/UI/Icons/Crossplay_icons/Crossplay_Steam.Crossplay_Steam";
	std::string xbox_icon = "/Game/PrimalEarth/UI/Icons/Crossplay_icons/Crossplay_Xbox.Crossplay_Xbox";
	std::string playstation_icon = "/Game/PrimalEarth/UI/Icons/Crossplay_icons/Crossplay_PS.Crossplay_PS";
	std::string admin_icon = DiscordGlobalChat::config["General"]["AdminIcon"].value("AdminIcon", "/Game/PrimalEarth/UI/Textures/ServerAdminChat_Icon.ServerAdminChat_Icon");

	

	std::string iconExists = GetIconPath(Chat->SenderName, player_controller);

	if (iconExists != "")
	{
		PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(iconExists).c_str(), nullptr, 0, 0, true));
	}
	else
	{
		

		if (DiscordGlobalChat::config["General"].value("ShowPlatformIcon", false))
		{
			switch (Chat->senderPlatform)
			{
			case 0:
				PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(generic_icon).c_str(), nullptr, 0, 0, true));
				break;
			case 1:
				PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(steam_icon).c_str(), nullptr, 0, 0, true));
				break;
			case 2:
				PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(xbox_icon).c_str(), nullptr, 0, 0, true));
				break;
			case 3:
				PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(playstation_icon).c_str(), nullptr, 0, 0, true));
				break;
			default:
				PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(generic_icon).c_str(), nullptr, 0, 0, true));
				break;
			}
		}
	}
	
	// override anything if admin
	if (DiscordGlobalChat::config["General"].value("AdminEnable", true) && Chat->SenderIsAdmin > 0 && admin_icon != "")
	{
		PlayerIcon = reinterpret_cast<UTexture2D*>(Globals::StaticLoadObject(UTexture2D::StaticClass(), nullptr, AsaApi::Tools::Utf8Decode(admin_icon).c_str(), nullptr, 0, 0, true));
	}

	return PlayerIcon;
}

FString GetPlatform(int platform)
{
	FString plat;
	switch (platform)
	{
	case 0:
		plat = "Epic";
		break;
	case 1:
		plat = "Steam";
		break;
	case 2:
		plat = "XBOX";
		break;
	case 3:
		plat = "Playstation";
		break;
	default:
		break;
	}
	return plat;
}

FString GetChatSendModeIntString(int SendMode)
{
	/*GlobalChat = 0x0,
	GlobalTribeChat = 0x1,
	LocalChat = 0x2,
	AllianceChat = 0x3,*/
	FString chatEnum;
	switch (SendMode)
	{
	case 0:
		chatEnum = "Global";
		break;
	case 1:
		chatEnum = "TRIBE";
		break;
	case 2:
		chatEnum = "LOCAL";
		break;
	case 3:
		chatEnum = "ALLY";
		break;
	default:
		break;
	}
	return chatEnum;
}

FString ParseEmoticons(EChatSendMode::Type SendMode, FString* Message)
{
	//std::string msg = Message->ToString();

	std::string msg = (*Message).ToStringUTF8();

	std::string msgColor = "1,1,1,1";

	switch (SendMode)
	{
	case EChatSendMode::GlobalChat:
		msgColor = DiscordGlobalChat::config["General"]["MsgColors"].value("MessageChatColor", "0,1,1,1");
		break;
	case EChatSendMode::AllianceChat:
		msgColor = DiscordGlobalChat::config["General"]["MsgColors"].value("MessageAllyChatColor", "1,0.5,1,1");
		break;
	case EChatSendMode::LocalChat:
		msgColor = DiscordGlobalChat::config["General"]["MsgColors"].value("MessageLocalChatColor", "0.5,0.5,1,1");
		break;
	case EChatSendMode::GlobalTribeChat:
		msgColor = DiscordGlobalChat::config["General"]["MsgColors"].value("MessageTribeChatColor", "1,1,0.5,1");
	}


	std::string realmsg = "";
	// Regular expression to match emoticons
	std::regex emoticonRegex(":\\)|:\\(|:\\||>:\\)|:o|:D|:d|:P|:p|;\\)");

	// Use regex_iterator to find emoticons in the message
	auto emoticonBegin = std::sregex_iterator(msg.begin(), msg.end(), emoticonRegex);
	auto emoticonEnd = std::sregex_iterator();

	// Process each emoticon separately
	size_t lastPos = 0;
	for (auto it = emoticonBegin; it != emoticonEnd; ++it) {
		auto match = *it;
		size_t startPos = match.position();
		size_t length = match.length();

		std::string text_msg = msg.substr(lastPos, startPos - lastPos);

		// Output text before the emoticon
		realmsg += fmt::format(" <RichColor Color=\"{}\">{}</> ", msgColor, text_msg);

		// emoticon
		realmsg += " " + msg.substr(startPos, length) + " ";

		// Update the last position
		lastPos = startPos + length;
	}

	// Output the remaining text after the last emoticon
	std::string lastmsg = msg.substr(lastPos);

	if (!lastmsg.empty())
		realmsg += fmt::format(" <RichColor Color=\"{}\">{}</> ", msgColor, lastmsg);

	//return realmsg.c_str();
	return FString::FromStringUTF8(realmsg);
}

nlohmann::json GetTribeData(FString SenderTribeName)
{
	nlohmann::json tribeData = DiscordGlobalChat::config["General"].value("Tribes", nlohmann::json().object());

	for (auto& item : tribeData.items()) {
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->warn("SenderTribeName {} Key {}", SenderTribeName.ToString(), item.key());

		if (SenderTribeName.Contains(item.key().c_str()))
		{
			return DiscordGlobalChat::config["General"]["Tribes"].value(item.key(), nlohmann::json().object());
		}
	}

	return nlohmann::json().object();
}

void callbackPostDiscord(bool success, std::string results, std::unordered_map<std::string, std::string> responseHeaders)
{
	if (DiscordGlobalChat::isDebug == true)
		Log::GetLog()->info("success: {}, results: {}", success, results);

	if (!success)
		Log::GetLog()->warn("something went wrong. request results: {}", results);

	return;
};

void PostToDiscord(FPrimalChatMessage* Chat)
{
	if (DiscordGlobalChat::config["Discord"].value("Webhook","").empty())
		return;
	// https://birdie0.github.io/discord-webhooks-guide/discord_webhook.html


	// Filter discord messages
	nlohmann::json msgFilters = DiscordGlobalChat::config["General"]["DiscordMsgFilters"].get<nlohmann::json>();

	if (!msgFilters.empty())
	{
		for (const auto& msgFilter : msgFilters)
		{
			std::string filterValue = msgFilter.get<std::string>();

			if (Chat->Message.ToString().find(filterValue) != std::string::npos)
			{
				// Skip sending message if the player sending @everyone
				return;
			}
		}
	}

	FString tribe = "";
	if (Chat->SenderTribeName.ToStringUTF8() != "")
		tribe = FString::FromStringUTF8(fmt::format("{}", Chat->SenderTribeName.ToStringUTF8()));

	FString sm = "";
	if (Chat->SendMode == EChatSendMode::LocalChat)
		sm = "[LOCAL]";

	FString senderSteamName = Chat->SenderSteamName;
	FString senderPlatform = GetPlatform(Chat->senderPlatform);

	if (!DiscordGlobalChat::config["General"].value("ShowSteamname",false))
		senderSteamName = "";

	if (!DiscordGlobalChat::config["General"].value("ShowTribename",false))
		tribe = "";

	if (!senderPlatform.IsEmpty())
		senderPlatform = FString::FromStringUTF8(fmt::format("[{}]", senderPlatform.ToStringUTF8()));

	if (!DiscordGlobalChat::config["General"].value("ShowPlatform",false))
		senderPlatform = "";

	std::string mapName = DiscordGlobalChat::config["General"].value("MapName","[TI]");

	if (!DiscordGlobalChat::config["General"].value("ShowMapname",false)) mapName = "";


	FString username = FString::FromStringUTF8(fmt::format("{}{} {} {}{}{}",
		mapName,
		senderSteamName.ToStringUTF8(),
		Chat->SenderName.ToStringUTF8(),
		tribe.ToStringUTF8(),
		senderPlatform.ToStringUTF8(),
		sm.ToStringUTF8()));

	FString bot_img = DiscordGlobalChat::config["DiscordBot"].value("BotImageURL","").c_str();

	std::string url = DiscordGlobalChat::config["DiscordBot"].value("WebHook", "");


	// Send to Tribe Channel
	if (Chat->SendMode == EChatSendMode::GlobalTribeChat)
	{
		nlohmann::json tribeData = GetTribeData(Chat->SenderTribeName);

		if (tribeData.empty()) return;

		// post to designated channel
		bot_img = tribeData.value("BotImage", "").c_str();
		url = tribeData.value("WebHook", "").c_str();

		// dont post to discord
		if (url.empty())  return;
	}

	FString msg = FString::FString(L"{{\"content\":\"{}\",\"username\":\"{}\",\"avatar_url\":\"{}\"}}");

	FString output = FString::FromStringUTF8(fmt::format(msg.ToStringUTF8(), Chat->Message.ToStringUTF8(), username.ToStringUTF8(), bot_img.ToStringUTF8()));

	std::vector<std::string> headers = {
		"Content-Type: application/json",
		"User-Agent: DiscordChat/1.0",
		"Connection: keep-alive",
		"Accept: */*"
	};

	std::string content_type = "application/json";

	try
	{
		//static_cast<AShooterGameState*>(AsaApi::GetApiUtils().GetWorld()->GameStateField().Get())->HTTPPostRequest(url, output);

		bool requestSuccess = DiscordGlobalChat::req.CreatePostRequest(url, callbackPostDiscord, output.ToStringUTF8(), content_type, headers);

		if (requestSuccess)
		{
			if (DiscordGlobalChat::isDebug == true)
				Log::GetLog()->info("Message Sent! {} {}", output.ToStringUTF8(), requestSuccess);
		}
	}
	catch (std::exception& error)
	{
		Log::GetLog()->error("PostToDiscordError {}", error.what());
	}
}

void SendDiscord(FPrimalChatMessage* Chat)
{
	if (DiscordGlobalChat::isDebug == true)
		Log::GetLog()->warn("SendDiscord() called");

	try
	{
		
		// Disable sending Local chats
		if (DiscordGlobalChat::config["General"].value("SendLocal", false) == false && Chat->SendMode == 2) return;

		// By Default dont send Ally for privacy
		if (Chat->SendMode == 3) return;

		PostToDiscord(Chat);
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("SendDiscord error {}", error.what());
		throw;
	}
}

void LogChat(FPrimalChatMessage* Chat)
{
	if (DiscordGlobalChat::isDebug == true)
		Log::GetLog()->info("LogChat() Called");

	try
	{
		// If log disabled
		if (DiscordGlobalChat::config["General"]["Logging"].value("LogChat", true) == false) return;

		if (DiscordGlobalChat::config["General"]["Logging"].value("LogTribe", true) == false && Chat->SendMode == 1) return;

		if (DiscordGlobalChat::config["General"]["Logging"].value("LogLocal", true) == false && Chat->SendMode == 2) return;

		if (DiscordGlobalChat::config["General"]["Logging"].value("LogAlly", true) == false && Chat->SendMode == 3) return;

		FString tribe = "";
		// modify tribe
		if (Chat->SenderTribeName.ToString() != "")
		{
			tribe = FString::FromStringUTF8(fmt::format("[{}]", Chat->SenderTribeName.ToStringUTF8()));
		}

		Log::GetLog()->info("{} ({}) {}[{}]({}): {}",
			Chat->SenderSteamName.ToStringUTF8(),
			Chat->SenderName.ToStringUTF8(),
			tribe.ToStringUTF8(),
			GetPlatform(Chat->senderPlatform).ToStringUTF8(),
			GetChatSendModeIntString(Chat->SendMode).ToStringUTF8(),
			Chat->Message.ToStringUTF8());
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error("LogChat error: {}", error.what());
		throw;
	}
}

ModifiedFPrimalChatMessage ConstructMsg(AShooterPlayerController* player_controller, FString* Message, int SendMode, int SenderPlatform)
{
	ModifiedFPrimalChatMessage mfpcm;

	FString playername;
	player_controller->GetPlayerCharacterName(&playername);

	FString steamName;
	unsigned int playerID = player_controller->GetLinkedPlayerID();
	player_controller->GetPlatformNameFromId(&steamName, playerID);

	FPrimalChatMessage msg;
	msg.SenderName = playername;
	msg.SenderSteamName = steamName;
	msg.SenderTribeName = player_controller->GetTribeName();
	msg.SenderId = playerID;
	msg.Message = *Message;
	msg.SenderTeamIndex = player_controller->TargetingTeamField();
	msg.SendMode = GetChatSendModeIntType(SendMode);
	msg.ChatType = GetChatTypeIntType(SendMode);
	msg.SenderIcon = nullptr;
	msg.senderPlatform = (unsigned char)SenderPlatform;
	msg.UserId = player_controller->GetEOSId(); //required on console player chat
	msg.SenderTeamIndex = player_controller->TargetingTeamField();
	msg.SenderIsAdmin = (unsigned char)player_controller->bIsAdmin().Get();
	
	mfpcm.msg = msg;
	mfpcm.spc = *player_controller;
	mfpcm.SendMode = SendMode;
	mfpcm.SenderPlatform = SenderPlatform;

	return mfpcm;
}

void ColorMessage(ModifiedFPrimalChatMessage& mfpcm)
{

}

void SendMessageToAll(ModifiedFPrimalChatMessage mfpcm)
{
	// Get All Players for sending
	const auto& player_controllers = AsaApi::GetApiUtils().GetWorld()->PlayerControllerListField();

	// Player loop
	for (TWeakObjectPtr<APlayerController> pc : player_controllers)
	{
		AShooterPlayerController* shooter_pc = static_cast<AShooterPlayerController*>(pc.Get());

		if (!shooter_pc) { continue; }

		// If sendmode is tribe and receiver is in the same tribe as sender
		if (mfpcm.msg.SendMode == 1 && mfpcm.spc.TargetingTeamField() != shooter_pc->TargetingTeamField()) { continue; }

		ColorMessage(mfpcm);
		
		FString receiverPlayername;
		shooter_pc->GetPlayerCharacterName(&receiverPlayername);

		mfpcm.msg.Receiver = receiverPlayername;

		shooter_pc->ClientChatMessage(mfpcm.msg);
	}
}

#if 0
void SaveSqlCrossServerChat(FPrimalChatMessage* Chat)
{
	if (!DiscordGlobalChat::config["General"]["Crosschat"].value("Enable", true)) return;

	const std::string currentserver = DiscordGlobalChat::config["General"]["Crosschat"].value("CurrentServer", "TI");

	if (currentserver.empty()) return;

	const std::string servers = DiscordGlobalChat::config["General"]["Crosschat"].value("Servers", "SE,Event");

	if (servers.empty()) return;

	if (Chat->Message.IsEmpty()) return;

	std::vector<std::string> serverArray = splitString(servers, ',');
	for (const auto& server : serverArray)
	{
		if (server != "")
		{
			try
			{
				nlohmann::json data = {
					{"EosId", Chat->UserId.ToString()},
					{"SenderName", Chat->SenderName.ToStringUTF8()},
					{"SenderSteamName", Chat->SenderSteamName.ToStringUTF8()},
					{"SenderTribeName", Chat->SenderTribeName.ToStringUTF8()},
					{"Platform", Chat->senderPlatform},
					{"Message", Chat->Message.ToStringUTF8()},
					{"CurrentServer", currentserver},
					{"ServerToSend", server}
				};

				//const FString strData = FString::FromStringUTF8(data.dump());

				const bool is_added = DiscordGlobalChat::chatdatabase->TryAddNewChat(data);
				if (is_added)
				{
					//Log::GetLog()->info("DiscordChat: is_added");
				}
			}
			catch (std::exception& error)
			{
				Log::GetLog()->error("RconChatCallback() error {}", error.what());
				return;
			}
		}
	}
}

#endif

bool ChatMessageCallback(AShooterPlayerController* player_controller, FString* Message, int SendMode, int SenderPlatform, bool spam_check, bool command_executed)
{

#pragma region GateKeeping
	// Player Dead
	if (AsaApi::IApiUtils::IsPlayerDead(player_controller)) return false;

	// Player Spectator
	if (player_controller->PlayerStateField().Get()->bIsSpectator().Get() == true) return false;

	if (spam_check || command_executed) return false;

	// Construct message
	ModifiedFPrimalChatMessage mfpcm = ConstructMsg(player_controller, Message, SendMode, SenderPlatform);

	// log chat
	LogChat(&mfpcm.msg);

	// Not processing local and ally  
	if (SendMode == 2 || SendMode == 3) return false;

#pragma endregion

#pragma region test block
	/*
	FString steamName;
	player_controller->GetPlatformNameFromId(&steamName, player_controller->GetLinkedPlayerID());

	FString playername;
	player_controller->GetPlayerCharacterName(&playername);

	FString tribeName = player_controller->GetTribeName();

	unsigned int playerID = player_controller->LinkedPlayerIDField();

	int tribeID = player_controller->TargetingTeamField();

	FString eosID = player_controller->GetEOSId();

	FString mapname;
	AsaApi::GetApiUtils().GetWorld()->GetMapName(&mapname);

	//OUTPUT: TEST playername SeanMarco | platformname Marco | msg test 123 global | sendmode 0 | senderplatform 1 | spam_check 0 | cmd_exec 0
	Log::GetLog()->info(
		"TEST playername {} | platformname {} | msg {} | sendmode {} | senderplatform {} | spam_check {} | cmd_exec {} | tribename {} | eosID {}",
		playername.ToString(),
		steamName.ToString(),
		Message->ToString(),
		SendMode,
		SenderPlatform,
		std::to_string(spam_check),
		std::to_string(command_executed),
		tribeName.ToString(),
		eosID.ToString()

	);
	*/
#pragma endregion

	SendMessageToDiscord(mfpcm.msg.Message);

	//SendMessageToAll(mfpcm);

	
	// return false to log global chat
	return false;



#if 0
	

	// Cache sender
	// check if exists
	if (!ReadPlayer(player_controller->GetEOSId()))
	{
		// add to db
		if (!AddPlayer(player_controller->GetEOSId(), player_controller->GetLinkedPlayerID(), player_controller->GetCharacterName(), SenderPlatform))
		{
			Log::GetLog()->error("Error inserting player.");
		}
	}

	FString senderName = AsaApi::GetApiUtils().GetCharacterName(player_controller);
	FString senderSteamName = AsaApi::GetApiUtils().GetSteamName(player_controller);
	FString tribename = static_cast<APrimalCharacter*>(player_controller->CharacterField().Get())->TribeNameField();
	int tribe_id = AsaApi::GetApiUtils().GetTribeID(player_controller);
	FString senderPlatform = GetPlatform(SenderPlatform);
	FString mapName = DiscordGlobalChat::config["General"].value("MapName","[TI]").c_str();
	std::string mapNameColor = DiscordGlobalChat::config["General"].value("MapnameColor","0.3,0.3,0.3,1");

	if (!DiscordGlobalChat::config["General"].value("ShowMapname", true) || mapName.IsEmpty())
	{
		mapName = "";
	}
	else
	{
		if (!mapNameColor.empty())
		{
			mapName = FString::Format("<RichColor Color=\"{}\">{}</>", mapNameColor, mapName.ToString());
		}
	}

	FPrimalChatMessage msg;
	msg.SendMode = GetChatSendModeIntType(SendMode);
	msg.ChatType = GetChatTypeIntType(SendMode);
	msg.senderPlatform = SenderPlatform;
	msg.UserId = player_controller->GetEOSId(); //required on console player chat
	msg.SenderTeamIndex = AsaApi::GetApiUtils().GetTribeID(player_controller);
	msg.SenderId = AsaApi::GetApiUtils().GetTribeID(player_controller);
	msg.ReceivedTime = -1.0f;

	const auto& player_controllers = AsaApi::GetApiUtils().GetWorld()->PlayerControllerListField();

	FString colorExists = "";
	if (player_controller->bIsAdmin().Get() == true && DiscordGlobalChat::config["General"].value("AdminEnable", false))
	{
		colorExists = DiscordGlobalChat::config["General"]["MsgColors"].value("AdminColor","1,0,0,1");
		msg.SenderIsAdmin = 1;
	}
	else
	{
		msg.SenderIsAdmin = 0;
		colorExists = GetPlayerColors(player_controller);
	}

	// Sender Icon
	if (DiscordGlobalChat::config["General"].value("ShowIcons", true))
	{
		msg.SenderName = senderName;
		msg.SenderIcon = GetPlayerIcons(&msg, player_controller);
		msg.SenderName = nullptr;
	}
	UTexture2D* currentIcon = msg.SenderIcon;

	// override steamname
	if (!senderSteamName.IsEmpty())
		senderSteamName = FString::FromStringUTF8(fmt::format("({})", senderSteamName.ToStringUTF8()));
	if (!DiscordGlobalChat::config["General"].value("ShowSteamname", true))
		senderSteamName = "";

	// override tribe
	if (!tribename.IsEmpty())
		tribename = FString::FromStringUTF8(fmt::format("[{}]", tribename.ToStringUTF8()));

	if (!DiscordGlobalChat::config["General"].value("ShowTribename",true))
		tribename = "";

	// override platform

	if (!senderPlatform.IsEmpty())
		senderPlatform = FString::FromStringUTF8(fmt::format("[{}]", senderPlatform.ToStringUTF8()));

	if (!DiscordGlobalChat::config["General"].value("ShowPlatform",true))
		senderPlatform = "";

	// override message if has emoticons;
	FString fmtMsgColored = ParseEmoticons(GetChatSendModeIntType(SendMode), Message);

	// TRIBE CHAT
	if (SendMode == 1)
	{
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->info("ChatMessageCallback() TRIBE CHAT");

		FString fmtSender = FString::FromStringUTF8(fmt::format("{}{}{}{} ({}): ",
			senderName.ToStringUTF8(),
			senderSteamName.ToStringUTF8(),
			tribename.ToStringUTF8(),
			senderPlatform.ToStringUTF8(),
			GetChatTypeIntString(SendMode).ToString()));

		// overide color
		std::string tribeColor = DiscordGlobalChat::config["General"]["MsgColors"].value("TribeChatColor","0,1,0,1");

		if (!tribeColor.empty())
			fmtSender = FString::FromStringUTF8(fmt::format("<RichColor Color=\"{}\">{}</>",
				tribeColor,
				fmtSender.ToStringUTF8()));

		FString fmtMsg = FString::FromStringUTF8(fmt::format("{}{}",
			fmtSender.ToStringUTF8(),
			fmtMsgColored.ToStringUTF8()));

		msg.Message = fmtMsg;

		// to remove (TRIBE) in front of the name
		msg.SendMode = EChatSendMode::GlobalChat;
		msg.ChatType = EChatType::GlobalChat;

		//msg.SenderId = (int)AsaApi::GetApiUtils().GetPlayerID(player_controller); //globalize this being redundant

		// Player loop
		for (TWeakObjectPtr<APlayerController> pc : player_controllers)
		{
			AShooterPlayerController* shooter_pc = static_cast<AShooterPlayerController*>(pc.Get());

			// remove compairing tribe id instead
			//FString tribename_pc = static_cast<APrimalCharacter*>(shooter_pc->CharacterField().Get())->TribeNameField();

			int receiverPlatform = GetPlayerPlatform(shooter_pc->GetEOSId());

			int receiver_tribe_id = AsaApi::GetApiUtils().GetTribeID(shooter_pc);

			// Filter tribe
			if (shooter_pc && shooter_pc->GetWorld() && tribe_id == receiver_tribe_id)
			{
				if (receiverPlatform == 2 || receiverPlatform == 3)
				{
					msg.SenderIcon = nullptr;
				}
				else
				{
					msg.SenderIcon = currentIcon;
				}
				Log::GetLog()->info("Message when change mode {}", msg.Message.ToString());
				shooter_pc->ClientChatMessage(msg);
			}
		}

		// Revert to Default just for Logging and Sending to Discord
		msg.Message = FString::FromStringUTF8((*Message).ToStringUTF8());
		msg.SenderName = senderName;
		msg.SenderSteamName = senderSteamName;
		msg.SenderTribeName = tribename;

		// declared before loop
		msg.SendMode = EChatSendMode::GlobalTribeChat;
		msg.ChatType = EChatType::GlobalTribeChat;

		SendDiscord(&msg);
		LogChat(&msg);

		return true;
	}

	// VIP Players (GLOBALCHAT)
	if (colorExists != "")
	{
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->info("ChatMessageCallback() VIP CHAT");

		FString fmtSender = FString::FromStringUTF8(fmt::format("{}{}{}{}:",
			senderName.ToStringUTF8(),
			senderSteamName.ToStringUTF8(),
			tribename.ToStringUTF8(),
			senderPlatform.ToStringUTF8()));

		FString fmtSenderColored = FString::FromStringUTF8(fmt::format("<RichColor Color=\"{}\">{}</>",
			colorExists.ToStringUTF8(),
			fmtSender.ToStringUTF8()));

		msg.Message = FString::FromStringUTF8(fmt::format("{}{}{}", mapName.ToStringUTF8(), fmtSenderColored.ToStringUTF8(), fmtMsgColored.ToStringUTF8()));


		//SendCrossServerChat(msg.Message);

		msg.senderPlatform = 5; //remove platform
		for (TWeakObjectPtr<APlayerController> player_controller : player_controllers)
		{
			AShooterPlayerController* shooter_pc = static_cast<AShooterPlayerController*>(player_controller.Get());

			int receiverPlatform = GetPlayerPlatform(AsaApi::GetApiUtils().GetEOSIDFromController(shooter_pc));

			if (shooter_pc)
			{
				if (receiverPlatform == 2 || receiverPlatform == 3)
				{
					msg.SenderIcon = nullptr;
				}
				else
				{
					msg.SenderIcon = currentIcon;
				}
				shooter_pc->ClientChatMessage(msg);
			}
		}
		msg.senderPlatform = SenderPlatform; // bring back the platform
		msg.Message = FString::FromStringUTF8((*Message).ToStringUTF8());
		msg.SenderName = senderName;
		msg.SenderSteamName = senderSteamName;
		msg.SenderTribeName = tribename;

		SaveSqlCrossServerChat(&msg);

		SendDiscord(&msg);
		LogChat(&msg);

		return true;
	}

	// GLOBAL CHAT
	if (SendMode == 0)
	{
		if (DiscordGlobalChat::isDebug == true)
			Log::GetLog()->info("ChatMessageCallback() GLOBAL CHAT");

		FString fmtMsg = FString::FromStringUTF8(fmt::format("{}{}{}{}{}: {}",
			mapName.ToStringUTF8(),
			senderName.ToStringUTF8(),
			senderSteamName.ToStringUTF8(),
			tribename.ToStringUTF8(),
			senderPlatform.ToStringUTF8(),
			fmtMsgColored.ToStringUTF8()));

		msg.Message = fmtMsg;

		// Loop to all players and send message
		for (TWeakObjectPtr<APlayerController> player_controller : player_controllers)
		{
			AShooterPlayerController* shooter_pc = static_cast<AShooterPlayerController*>(player_controller.Get());

			int receiverPlatform = GetPlayerPlatform(shooter_pc->GetEOSId());

			if (shooter_pc)
			{
				//AShooterCharacter* shooter_c = shooter_pc->GetPlayerCharacter();

				if (receiverPlatform == 2 || receiverPlatform == 3)
				{
					msg.SenderIcon = nullptr;
				}
				else
				{
					msg.SenderIcon = currentIcon;
				}
				msg.Receiver = AsaApi::GetApiUtils().GetCharacterName(shooter_pc);
				shooter_pc->ClientChatMessage(msg);
			}
		}

		//SendCrossServerChat(fmtMsg);

		// Revert to default for logging and sending to discord
		msg.Message = FString::FromStringUTF8((*Message).ToStringUTF8());

		msg.SenderName = senderName;
		msg.SenderSteamName = senderSteamName;
		msg.SenderTribeName = tribename;

		//Log::GetLog()->info("UserId {} SenderId {}", AsaApi::GetApiUtils().GetEOSIDFromController(_this).ToString(), std::to_string(AsaApi::GetApiUtils().GetPlayerID(_this)));

		SaveSqlCrossServerChat(&msg);

		SendDiscord(&msg);
		LogChat(&msg);

		return true;
	}


	return false;

#endif

}

/* End Plugin Utils */


void ReadConfig()
{
	try
	{
		const std::string config_path = AsaApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/" + PROJECT_NAME + "/config.json";
		std::ifstream file{config_path};
		if (!file.is_open())
		{
			throw std::runtime_error("Can't open config file.");
		}
		file >> DiscordGlobalChat::config;

		Log::GetLog()->info("{} config file loaded.", PROJECT_NAME);

		DiscordGlobalChat::isDebug = DiscordGlobalChat::config["General"]["Debug"].get<bool>();

		//Log::GetLog()->warn("Debug {}", DiscordGlobalChat::isDebug);

	}
	catch(const std::exception& error)
	{
		Log::GetLog()->error("Config load failed. ERROR: {}", error.what());
		throw;
	}
}

void LoadDatabase()
{
	Log::GetLog()->warn("LoadDatabase");
	DiscordGlobalChat::pluginTemplateDB = DatabaseFactory::createConnector(DiscordGlobalChat::config["PluginDBSettings"]);

	nlohmann::ordered_json tableDefinition = {};
	if (DiscordGlobalChat::config["PluginDBSettings"].value("UseMySQL", true) == true)
	{
		tableDefinition = {
			{"Id", "INT NOT NULL AUTO_INCREMENT"},
			{"EosId", "VARCHAR(50) NOT NULL"},
			{"PlayerId", "VARCHAR(50) NOT NULL"},
			{"PlayerName", "VARCHAR(50) NOT NULL"},
			{"Platform", "VARCHAR(50)"},
			{"Message", "VARCHAR(255)"},
			{"CurrentServer", "VARCHAR(50)"},
			{"ServerToSend", "VARCHAR(50)"},
			{"CreateAt", "DATETIME DEFAULT CURRENT_TIMESTAMP"},
			{"PRIMARY", "KEY(Id)"},
			{"UNIQUE", "INDEX EosId_UNIQUE (EosId ASC)"}
		};
	}
	else
	{
		tableDefinition = {
			{"Id","INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT"},
			{"EosId","TEXT NOT NULL UNIQUE"},
			{"PlayerId","TEXT"},
			{"PlayerName","TEXT"},
			{"Platform", "TEXT"},
			{"Message", "TEXT"},
			{"CurrentServer", "TEXT"},
			{"ServerToSend", "TEXT"},
			{"CreateAt","TIMESTAMP DEFAULT CURRENT_TIMESTAMP"}
		};
	}

	DiscordGlobalChat::pluginTemplateDB->createTableIfNotExist(DiscordGlobalChat::config["PluginDBSettings"].value("TableName", ""), tableDefinition);


	// PermissionsDB
	if (DiscordGlobalChat::config["PermissionsDBSettings"].value("Enabled", true) == true)
	{
		DiscordGlobalChat::permissionsDB = DatabaseFactory::createConnector(DiscordGlobalChat::config["PermissionsDBSettings"]);
	}

	// PointsDB (ArkShop)
	if (DiscordGlobalChat::config["PointsDBSettings"].value("Enabled", true) == true)
	{
		DiscordGlobalChat::pointsDB = DatabaseFactory::createConnector(DiscordGlobalChat::config["PointsDBSettings"]);
	}
	
}
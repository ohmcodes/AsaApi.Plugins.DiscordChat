# AsaApi.Plugins.DiscordChat
ARK:SA In-game chat to Discord and Crossplay RCON support

# Requirements
- ServerAPI [Download/Install](https://gameservershub.com/forums/resources/ark-survival-ascended-serverapi-crossplay-supported.683/)
- Ofcourse the ARK:SA Server
- Bot Creation
- Discord Channel Integration WebHook

# Features
- Able to send local chat on discord
- Able to set prefix eg: `[Discord]`
- Combination of WebHook and Discord Bot
  - WebHook (Sending Messages with Username and Separated per User)
  - Discord Bot (Fetching all messages in specific channel)
- Fetch Interval can be change
- Logging (Global, Tribe, Local, Ally)
- Tribe chat color (Default green)
- Direct Messaging `/dm <name> <message>` also can set the color
- VIP Tribes messaging can relay to their own Discord server / channel
- VIP players can set their own color
- RCON chat is supported for those who crashing on normal RCON chat `RCON cmd can be set on config.json`
- Plugin reloads both Console and RCON
  - `DiscordChat.Reload`
- Custom Chat Icons
- Platform Chat Icons [Generic, Steam, XBOX, PS]

# Future updates
- Clustered map Chat

# Select ICONS
- [ICONS.MD](https://github.com/ohmcodes/AsaApi.Plugins.DiscordChat/blob/main/icons.md)

# KNOWN ISSUES
- Console Log (No Message received)  Doesn't affect anything - Still waiting for ServerAPI update 


# Blank Config
```
{
  "General": {
    "SendLocal": false,
    "WebHook": "",
    "FetchMessages": true,
    "BotToken": "",
    "ChannelId": "",
    "DiscordSenderPrefix": "[Discord]",
    "FetchInterval": 5,
    "LogChat": false,
    "LogTribe": false,
    "LogLocal": false,
    "LogAlly": false,
    "TribeChatColor": "0,1,0,1",
    "DMChatColor": "0.84,0.52,0,1",
	"ShowPlatform": true,
    "ShowIcons": true
  },
  "Tribes": {},
  "Colors": {},
  "Icons": {},
  "Messages": {
    "DMCmd": "/dm",
    "RconCmd": "DiscordChat",
    "DMerror": "Dirrect message error: Invalid Playername or Message"
  } 
}
```

# Config Commented
```
{
  "General": {
    "SendLocal": true, /* send local chats on discord global chat channel */
    "WebHook": "", /* Global chat messaging ingame to discord */
    "FetchMessages": true, /* enabling fetch message */
    "BotToken": "", /* bot to fetch/send message on discord to game */
    "ChannelId": "", /* channel to recieve global chat ingame */
    "DiscordSenderPrefix": "[Discord] ", /* Sender prefix */
    "FetchInterval": 5, /* chat fetch interval */
    "LogChat": false, /* this logs global chat on server api console */
    "LogTribe": false, /* this logs tribe chat on server api console */
    "LogLocal": false, /* this logs local chat on server api console */
    "LogAlly": false, /* this logs ally chat on server api console */
    "TribeChatColor": "0,1,0,1", /* default green */
    "DMChatColor": "0.84,0.52,0,1", /* default orange */
	"ShowPlatform": true, /* Enables Platform eg: [Steam] */
    "ShowIcons": true /* Enables Icons */
  },
  "Tribes": {
    "VIP TRIBENAME": {
      "WebHook": "", /* webhoook url for sending messages to specific channel */
      "FetchMessages": true, /* enable fetch messages */
      "BotToken": "", /* if the VIP Tribe does their own bot and discord for fetching messages */
      "ChannelId": "", /* VIP Tribe own discord channel id */
      "DiscordSenderPrefix": "[Discord] " /* prefix before playername */
    },
    "VIP TRIBENAME 1": {
      "WebHook": "", 
      "FetchMessages": true,
      "BotToken": "", 
      "ChannelId": "",
      "DiscordSenderPrefix": "[Discord] "
    },
    "VIP TRIBENAME 2": {
      "WebHook": "", 
      "FetchMessages": true,
      "BotToken": "",
      "ChannelId": "",
      "DiscordSenderPrefix": "[Discord] " 
    }
  },
  "Colors": {
    "VIP Playername": "1,0,1,1", /* Red, Green, Blue, Alpha = opacity */
    "VIP Playername 1": "1,1,0,1",
    "VIP Playername 2": "1,1,1,1"
  },
  "Icons": {
    "ICON Player 1": "/Game/PrimalEarth/CoreBlueprints/Items/Artifacts/Icons/Implant_Icon.Implant_Icon",
    "ICON Player 2": "/Game/PrimalEarth/CoreBlueprints/Items/Artifacts/Icons/Implant_Icon_Gamma.Implant_Icon_Gamma",
    "ICON Player 3": "/Game/PrimalEarth/CoreBlueprints/Items/Artifacts/Icons/Implant_Icon_Beta.Implant_Icon_Beta",
    "ICON Player 4": "/Game/PrimalEarth/CoreBlueprints/Items/Artifacts/Icons/Implant_Icon_Alpha.Implant_Icon_Alpha"
  },
  "Messages": {
    "DMCmd": "/dm", /* Direct message /dm playername */
    "RconCmd":  "DiscordChat" /* this can be support different third party apps that uses rcon to send messages */
  } 
}
```

# Support
Feel free to join my Discord server [ArkTools](https://discord.gg/q8rPGprjEJ)

# Give Love!
[Buy me a coffee](https://www.buymeacoffee.com/ohmcodes)

[Paypal](https://www.paypal.com/donate/?business=8389QZ23QRDPE&no_recurring=0&item_name=Game+Server%2FTools+Community+Donations&currency_code=CAD)

# Source code
As soon as I am comfortable with my coding I will release it to the public :)

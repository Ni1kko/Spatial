using SpatialP2CCore.Utils;
using DSharpPlus;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using SpatialP2CCore.SQL;

namespace SpatialP2CCore
{
    public class Client
    {
        internal static DiscordShardedClient SpatialP2C;
        internal static Info Info;
        internal static bool Ready = false;
        internal static List<string> CommandList { get; set; }
        internal static int PresenceIndex { get; set; } = 0;
        internal static Dictionary<int, DiscordClient> Shards{ get; set; }
        internal const string BotRepo = "https://github.com/Ni1kko/SpatialP2C";
        public static DateTime StartedAt { get; private set; } = DateTime.Now;
        public Client(Info configInfo) => Info = configInfo;
        internal static Database DB;
         
        public async Task Startup()
        {
            //-- Verify Token is set
            if (Info.Token.Equals(string.Empty))
            {
                await Console.Out.WriteLineAsync("You forgot to insert your bot token!");
                Console.Write("Press any key to exit...");
                Console.ReadKey();
                return;
            };

            //-- Prefix space
            if (Info.PrefixSpace) Info.Prefix += " ";

            //--Setup DB
            DB = new Database("spatial", pass: "somepassword22");
            
            //-- 
            SpatialP2C = new DiscordShardedClient(new DiscordConfiguration()
            {
                Token = Info.Token,
                TokenType = TokenType.Bot,
                Intents = DiscordIntents.AllUnprivileged,
                AutoReconnect = true,
                MinimumLogLevel = Info.Debug ? LogLevel.Trace : LogLevel.Information,
                ShardCount = Info.ShardCount
            });

            var botID = Info.ID.ToString();
            var EventID = int.Parse(botID.Substring(botID.Length - 4));

            await SpatialP2C.SubscribeEvents();
            await SpatialP2C.Login();
        }
    }
}

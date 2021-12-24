using DSharpPlus;
using DSharpPlus.CommandsNext;
using DSharpPlus.Entities;
using Microsoft.Extensions.Logging;
using System;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace SpatialP2CCore.Utils
{
    // Extension methods must be defined in a static class.
    internal static class Misc 
    {
        /// <summary>
        /// nested message helper
        /// </summary>
        internal class Message
        {
            internal string OnClientJoined(DiscordGuild guild)
            {
                return "{USER} has joined the server!";
            }

            internal string OnClientLeft(DiscordGuild guild)
            {
                return "{USER} has left the server!";
            }

            internal DiscordEmbedBuilder OnLackPerms(CommandContext ctx)
            {
                var emoji = DiscordEmoji.FromName(ctx.Client, ":no_entry:");
                 
                return new DiscordEmbedBuilder
                {
                    Title = "Access denied",
                    Description = $"{emoji} You do not have the permissions required to execute this command.",
                    Color = new DiscordColor(0xFF0000) // red
                };
            }
        }

        public static string ReplacementCallack(this string str, DiscordClient client)
        {
            string replacementCallack(Match match)
            {
                string r = "";
            
                switch (match.Groups[1].Value)
                {
                    case "S":
                        r = (client.Guilds.Count == 1) ? "" : "s";
                        break;
                    case "GUILD_COUNT":
                        r = $"{client.Guilds.Count}";
                        break;
                    case "SHARD_ID":
                        r = $"{client.ShardId}";
                        break;
                    case "SHARD_COUNT":
                        r = $"{client.ShardCount}";
                        break;
                    case "PREFIX":
                        r = Client.Info.Prefix + (Client.Info.PrefixSpace ? " " : "");
                        break;
                }

                return r;
            }

            return Regex.Replace(str, @"\{([A-Z_]+)\}", replacementCallack);
        }

        public static string Uptime(this DateTime timeDate)
        {
            var delta = DateTime.Now - timeDate;
            var days = delta.Days.ToString("n0");
            var hours = delta.Hours.ToString("n0");
            var minutes = delta.Minutes.ToString("n0");
            var seconds = delta.Seconds.ToString("n0");

            var builder = new StringBuilder();
            if (!days.Equals("0")) builder.Append($"{days} days ");
            if (!hours.Equals("0")) builder.Append($"{hours} hours ");
            if (!minutes.Equals("0")) builder.Append($"{minutes} minutes ");
            builder.Append($"{seconds} seconds");

            return builder.ToString();
        }

        public static string Normalize(this string text)
        {
            var wrappedText = text.WrapText();
            string[] lines = wrappedText.Split(new string[] { "\n" }, StringSplitOptions.None);
            var maxlen = lines[0].Length;

            for (int i = 0; i < lines.Length; i++)
            {
                var count = lines[i].Split(new string[] { "" }, StringSplitOptions.None).Length;
                var spaceCount = maxlen - count;
                var append = string.Empty;
                for (var j = 0; j < spaceCount; j++) { append += " "; }
                lines[i] = lines[i] + append;
            }

            var sb = new StringBuilder();
            foreach (var line in lines) { sb.Append(line + '\n'); }
            return sb.ToString();
        }

        public static string WrapText(this string text, int wrapSize = 40)
        {
            int pos, next = 0;
            var sb = new StringBuilder();

            if (wrapSize < 1) return text;
            for (pos = 0; pos < text.Length; pos = next)
            {
                int eol = text.IndexOf(Environment.NewLine, pos);

                if (eol == -1)  next = eol = text.Length; 
                else  next = eol + Environment.NewLine.Length;
                
                if (eol > pos)
                {
                    do
                    {
                        int len = eol - pos;
                        if (len > wrapSize) len = text.BreakLine(pos, wrapSize);
                        
                        sb.Append(text, pos, len);
                        sb.Append(Environment.NewLine);

                        pos += len;

                        while (pos < eol && char.IsWhiteSpace(text[pos])) { pos++; };
                    } while (eol > pos);
                }
                else sb.Append(Environment.NewLine);
            }

            return sb.ToString();
        }

        private static int BreakLine(this string text, int pos, int max)
        {
            int i = max;
            while (i >= 0 && !char.IsWhiteSpace(text[pos + i])) { i--; }
            if (i < 0) return max;
            while (i >= 0 && char.IsWhiteSpace(text[pos + i])){ i--; }
            return i + 1;
        }

        internal static async Task<bool> HasPermission(this DiscordMember member, Permissions permission, CommandContext ctx = null)
        {
            bool allow = false; 
            foreach (var role in member.Roles) { if (role.CheckPermission(permission) == PermissionLevel.Allowed) allow = true; };
            if (!allow && ctx != null) await ctx.RespondAsync(new Message().OnLackPerms(ctx));
            return allow;
        }

        internal static async Task<bool> CanManageGuild(this DiscordMember member) => await member.HasPermission(Permissions.ManageGuild);
        internal static async Task<bool> CanManageMessages(this DiscordMember member) => await member.HasPermission(Permissions.ManageMessages);
        internal static async Task<bool> CanKickMembers(this DiscordMember member) => await member.HasPermission(Permissions.KickMembers);
        internal static async Task<bool> CanBanMembers(this DiscordMember member) => await member.HasPermission(Permissions.BanMembers);
        internal static async Task<bool> CanManageRoles(this DiscordMember member) => await member.HasPermission(Permissions.ManageRoles);
      
        internal static async Task UpdateStatus(this DiscordClient discordClient, UserStatus userStatus, ActivityType activityType = ActivityType.Playing)
        {
            DiscordActivity activity;
            if (discordClient == null) return;
            try
            {
                if (Client.PresenceIndex > Client.Info.PresenceStrings.Length - 1) Client.PresenceIndex = 0;

                string[] presenceStrings = Client.Info.PresenceStrings;
                string selectedPresence = presenceStrings[Client.PresenceIndex];
                string presence = selectedPresence.ReplacementCallack(discordClient);

                activity = new DiscordActivity(presence, activityType);

                await discordClient.UpdateStatusAsync(activity, userStatus);

                Client.PresenceIndex++;
            }
            catch (Exception ex)
            {
                await Console.Out.WriteLineAsync($"Unable To Update Activity: {ex}");
                return;
            }

            //-- Update console
            if (activity != null && userStatus != UserStatus.Offline)
            {
                Console.Title = $"{Client.Info.Name}#{discordClient.CurrentUser.Discriminator} <!@{discordClient.CurrentUser.Id}> {activity.ActivityType.ToString()}: {activity.Name}";
                discordClient.Logger.LogInformation(Events.StatusUpdateID, $"{Client.Info.Name} - Status - ({userStatus.ToString()}) - Uptime ({Client.StartedAt.Uptime()}) - Activity - {activity.ActivityType.ToString()} {activity.Name}");
            }
        }

        internal static async Task SubscribeEvents(this DiscordShardedClient SpatialP2C)
        {
            var events = new Events(SpatialP2C);
            if (events.Subscribed) SpatialP2C.Logger.LogInformation(Events.BotLoginID, $"{Client.Info.Name} Events Subscribed");
            else
            {
                await Console.Out.WriteLineAsync($"Unable to Subscribe {Client.Info.Name} Events");
                Console.Write("Press any key to exit...");
                Console.ReadKey();
                return;
            }
            await Task.CompletedTask;
        }
        
        internal static async Task Login(this DiscordShardedClient SpatialP2C)
        {
            //-- Connect With Discord 
            await SpatialP2C.StartAsync();
            await Task.Delay(-1);
        }
    }
}
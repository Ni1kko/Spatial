using SpatialP2CCore.Utils;
using DSharpPlus;
using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Exceptions;
using DSharpPlus.Entities;
using DSharpPlus.EventArgs;
using DSharpPlus.Interactivity;
using DSharpPlus.Interactivity.Extensions;
using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Threading.Tasks;


namespace SpatialP2CCore
{
    internal sealed class Events
    {
        internal bool Subscribed { get; private set; } = false;
        internal static readonly EventId RegCommandID = new EventId(0, "#Commands");
        internal static readonly EventId BotLoginID = new EventId(1, "#Login");
        internal static readonly EventId GuildConnectedID = new EventId(2, "#G-Connected");
        internal static readonly EventId StatusUpdateID = new EventId(3, "#Status");

        // Bot Events
        internal static class ClientReady
        {
            internal static async Task Method(DiscordClient sender, ReadyEventArgs events)
            {
                Console.CancelKeyPress += async delegate { await sender.DisconnectAsync(); };
                Console.Title = $"{Client.Info.Name} v{Assembly.GetExecutingAssembly().GetName().Version}";

                try
                {
                    await sender.UpdateStatus(UserStatus.Offline);
                }
                catch (Exception ex)
                {
                    await Console.Out.WriteLineAsync($"Unable To Update {Client.Info.Name}'s Activity: {ex}");
                }

                Client.Ready = true;

                await Task.CompletedTask;
            }
        }
        internal static class ClientErrored
        {
            internal static async Task Method(DiscordClient sender, ClientErrorEventArgs events)
            { 
                await Console.Out.WriteLineAsync($"Exception occured {events.Exception}");
                await Task.CompletedTask;
            }
        }
        internal static class ClientGuildAvailable
        {
            internal static async Task Method(DiscordClient sender, GuildCreateEventArgs events)
            {
                sender.Logger.LogInformation(GuildConnectedID, $"Connection with server {events.Guild.Name} has been established");
                await Task.CompletedTask;
            }
        }
        internal static class ClientHeartbeat
        {
            internal static async Task Method(DiscordClient sender, HeartbeatEventArgs events)
            {
                if (Client.Ready) await sender.UpdateStatus(UserStatus.Idle);
                
                await Task.CompletedTask;
            }
        }
        internal static class ClientJoined
        {
            internal static async Task Method(DiscordClient sender, GuildMemberAddEventArgs events)
            {
                await Console.Out.WriteLineAsync(new Misc.Message().OnClientJoined(events.Guild));
            }
        }
        internal static class ClientLeft
        {
            internal static async Task Method(DiscordClient sender, GuildMemberRemoveEventArgs events)
            {
                await Console.Out.WriteLineAsync(new Misc.Message().OnClientLeft(events.Guild));
            }
        }
        internal static class ClientCommand
        {
            private static IReadOnlyDictionary<int, CommandsNextExtension> CommandNextExtension; 
             
            private static async Task MethodSuccses(CommandsNextExtension sender, CommandExecutionEventArgs events)
            {
                Console.Write($"{events.Context.User.Username} successfully executed '{events.Command.QualifiedName}'");
                await Task.CompletedTask;
            }
            private static async Task MethodError(CommandsNextExtension sender, CommandErrorEventArgs events)
            {
                Console.Write($"{events.Context.User.Username} tried executing '{events.Command?.QualifiedName ?? "<unknown command>"}' but it errored: {events.Exception.GetType()}: {events.Exception.Message ?? "<no message>"}");
                if (events.Exception is ChecksFailedException) await events.Context.RespondAsync(new Misc.Message().OnLackPerms(events.Context));
                await Task.CompletedTask;
            }
            internal static async Task Setup(DiscordShardedClient discordClient)
            {
                CommandNextExtension = await discordClient.UseCommandsNextAsync(new CommandsNextConfiguration
                {
                    StringPrefixes = new[] { Client.Info.Prefix },

                    // enable responding in direct messages
                    EnableDms = true,

                    // enable mentioning the bot as a command prefix
                    EnableMentionPrefix = true,

                    DmHelp = true
                });
             
                foreach (var command in CommandNextExtension.Values)
                {
                    //-- Discord Command Listeners
                    command.CommandExecuted += MethodSuccses;
                    command.CommandErrored += MethodError;

                    //-- set up our custom help formatter
                    command.SetHelpFormatter<HelpFormatter>();

                    //-- Register our custom commands
                    //command.RegisterCommands<UserCommands>(); 
                    var SpatialP2CCommandsList = new List<string>() {
                        "AboutCommands",
                        "AdminCommands",
                        "PruneCommands",
                        "UserCommands",
                    };

                    //Loop through list of custom commands
                    foreach (var cmd in SpatialP2CCommandsList)
                    {
                        try
                        {
                            Type cmdType = Type.GetType($"SpatialP2CCore.Commands.{cmd}"); 
                            discordClient.Logger.LogInformation(RegCommandID, $"Registering command: {cmd} on Shard #{command.Client.ShardId}/{Client.Info.ShardCount - 1}");
                            command.RegisterCommands(cmdType);
                        } catch (Exception ex) {
                            discordClient.Logger.LogInformation(RegCommandID, $"Error attempting to register command: {cmd}");
                            Console.WriteLine($"Exception: {ex.Message}");
                            return;
                        };
                    };
                };

                await Task.CompletedTask;
            }
        }
        internal static class ClientInteractivity
        {
            private static IReadOnlyDictionary<int, InteractivityExtension> InteractiveExtension;

            internal static async Task Setup(DiscordShardedClient discordClient)
            {
                InteractiveExtension = await discordClient.UseInteractivityAsync(new InteractivityConfiguration
                {
                    Timeout = TimeSpan.FromMinutes(2)
                });

                await Task.CompletedTask;
            }
        }
        internal static class ClientMessage
        {
            internal static async Task Method(DiscordClient sender, MessageCreateEventArgs events)
            {
                var isBot = events.Author.IsBot;
                var thisBot = events.Author.Id.Equals(Client.Info.ID) && isBot;
                var uidWithNickname = events.Author.ToString().Substring(events.Author.ToString().IndexOf("#"));
                var uid = uidWithNickname.Substring(0, uidWithNickname.IndexOf(" "));
                var authorString = $"By: [{events.Author.Username} {uid} {events.Author.Mention}]";
                var messageBody = events.Message.Content;

                if (string.IsNullOrWhiteSpace(messageBody)) return;

                foreach (var taggedUser in events.MentionedUsers)
                {
                    messageBody.Replace(taggedUser.Mention, $"@{taggedUser.Username}");
                }

                if (isBot && thisBot)
                    await Console.Out.WriteLineAsync($"New SpatialP2C Response: {messageBody}");
                else if (isBot)
                    await Console.Out.WriteLineAsync($"New Bot Response :{messageBody} {authorString}");
                else
                    await Console.Out.WriteLineAsync($"New Message {authorString} Contents: {messageBody}");

                if (messageBody.ToLower().StartsWith("ping"))
                    await events.Message.RespondAsync($"{events.Author.Mention} pong!");
                 
            }
        }

        /// <summary>
        /// Class Constructor
        /// </summary>
        internal Events(DiscordShardedClient SpatialP2C)
        {
            //-- Subscribe SpatialP2Cs `on ready method`
            SpatialP2C.Ready += ClientReady.Method;

            //-- Subscribe SpatialP2Cs `on error method`
            SpatialP2C.ClientErrored += ClientErrored.Method;

            //-- Subscribe SpatialP2Cs `on guild available method`
            SpatialP2C.GuildAvailable += ClientGuildAvailable.Method;

            //-- Subscribe SpatialP2Cs `on heartbeat method`
            SpatialP2C.Heartbeated += ClientHeartbeat.Method;

            //-- Subscribe SpatialP2Cs `on join method`
            SpatialP2C.GuildMemberAdded += ClientJoined.Method;

            //-- Subscribe SpatialP2Cs `on leave method`
            SpatialP2C.GuildMemberRemoved += ClientLeft.Method;

            //-- Setup & register SpatialP2Cs commands
            ClientCommand.Setup(SpatialP2C).GetAwaiter().GetResult();

            //-- Setup & register SpatialP2Cs interactivty
            ClientInteractivity.Setup(SpatialP2C).GetAwaiter().GetResult();

            //-- Subscribe SpatialP2Cs `on message method`
            SpatialP2C.MessageCreated += ClientMessage.Method;

            Subscribed = true;
        }
    }
}

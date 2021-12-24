using SpatialP2CCore.Utils;
using DSharpPlus;
using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Attributes;
using DSharpPlus.Entities;
using DSharpPlus.Interactivity.Extensions;
using System;
using System.Linq;
using System.Threading.Tasks;

/*
    <prefix>admin sudo @member user greet @member2
    <prefix>admin nick @member New Name
    <prefix>admin kick @member You were kicked as part of a test.
    <prefix>admin ban @member You were banned as part of a test
    <prefix>admin msg @member You were messaged as part of a test.
*/

namespace SpatialP2CCore.Commands
{
    [Group("admin"), Description("Administrative commands.")] // give it a description for help purposes
    [Hidden, RequirePermissions(Permissions.ManageGuild)] // let's hide this from the eyes of curious users and restrict this to users who have appropriate permissions
    public class AdminCommands : BaseCommandModule
    {
        // this command will be only executable by the bot's owner
        [Command("sudo"), Description("Executes a command as another user."), Hidden, RequireOwner]
        public async Task Sudo(CommandContext ctx, [Description("Member to execute as.")] DiscordMember member, [RemainingText, Description("Command text to execute.")] string command)
        {
            //.admin sudo @Ⱨ₳₱₱Ɏ₣ɆɆ₮#0001 prune 10

            // note the [RemainingText] attribute on the argument.
            // it will capture all the text passed to the command

            if (!ctx.Member.HasPermission(Permissions.Administrator, ctx).GetAwaiter().GetResult()) return;
            await ctx.TriggerTypingAsync();
            await ctx.Message.DeleteAsync();

            // get the command service, we need this for
            // sudo purposes
            var cmds = ctx.CommandsNext;

            // retrieve the command and its arguments from the given string
            var cmd = cmds.FindCommand(command, out var customArgs);

            // create a fake CommandContext
            var fakeContext = cmds.CreateFakeContext(member, ctx.Channel, command, ctx.Prefix, cmd, customArgs);

            // and perform the sudo
            await cmds.ExecuteCommandAsync(fakeContext);
        }

        [Command("nick"), Description("Gives someone a new nickname."), RequirePermissions(Permissions.ManageNicknames)]
        public async Task ChangeNickname(CommandContext ctx, [Description("Member to change the nickname for.")] DiscordMember Target, [RemainingText, Description("The nickname to give to that user.")] string new_nickname)
        {
            //.admin nick @Ⱨ₳₱₱Ɏ₣ɆɆ₮#0001 SomeNewNameHere

            var Admin = ctx.Member;
            if (!Admin.HasPermission(Permissions.Administrator, ctx).GetAwaiter().GetResult()) return;
            await ctx.TriggerTypingAsync();
            await ctx.Message.DeleteAsync();

            var msg = $"{Admin.DisplayName} Updated your nickname too: {new_nickname}";

            try
            {
                // let's change the nickname, and tell the 
                // audit logs who did it.
                await Target.ModifyAsync(x =>
                {
                    x.Nickname = new_nickname;
                    x.AuditLogReason = $"Changed by {ctx.User.Username} ({ctx.User.Id}).";
                });

                // let's make a simple response.
                var emoji = DiscordEmoji.FromName(ctx.Client, ":+1:");
                
                // and respond with it.
                await ctx.RespondAsync(emoji);
            }
            catch (Exception)
            {
                // oh no, something failed, let the invoker now
                var emoji = DiscordEmoji.FromName(ctx.Client, ":-1:");
                await ctx.RespondAsync(emoji);
                return;
            };

            await Target.SendMessageAsync(msg); //msg them
        }

        [Command("kick"), Description("Kicks a user [Requires KickMember persission]")]
        public async Task KickUser(CommandContext ctx, [Description("Member to kick.")] DiscordMember Target, [RemainingText] string Input = "")
        {
            var Admin = ctx.Member;
            if (!Admin.CanKickMembers().GetAwaiter().GetResult()) return;
            if (Target.IsOwner)
            {
                await ctx.Channel.SendMessageAsync("Can't Kick Owner");
                return;
            }
            await ctx.Message.DeleteAsync();
            
            //Kick Reason Input
            string reason = Input.Trim(new char[0]);

            //Random Reasons
            if (reason.ToLower() == "rand")
            {
                string[] randomMSGS = {
                    "I felt like it",
                    "For the shits & giggles",
                    "As i have more perms than you",
                    "Because i can!",
                    "1.6 see rpuk :joy:",
                    "Not a test, you have been banned",
                    "Finger slipped"
                };
                reason = randomMSGS[new Random().Next(0, randomMSGS.Length)];
            }

            //Reason Not Given
            if (reason == "") reason = "Not Given";
            
            var msg = $"{Admin.DisplayName} kicked you. | Reason: {reason}";
             
            var embed = new DiscordEmbedBuilder
            {
                Title = $"User {Target.DisplayName} was kicked",
                Color = DiscordColor.Goldenrod
            };
            embed.AddField("Kicked By", Admin.DisplayName, true);
            embed.AddField("Reason", reason, true);

            await ctx.Channel.TriggerTypingAsync();
            await Target.SendMessageAsync(msg);
            await Target.RemoveAsync(reason);
            await ctx.Channel.SendMessageAsync(embed);
        }

        [Command("ban"), Description("bans a user [Requires banMember persission]")]
        public async Task BanUser(CommandContext ctx, [Description("Member to ban.")] DiscordMember Target, [RemainingText] string Input = "")
        {
            var Admin = ctx.Member;
            if (!Admin.CanBanMembers().GetAwaiter().GetResult()) return;
            if (Target.IsOwner)
            {
                await ctx.Channel.SendMessageAsync("Can't Ban Owner");
                return;
            }
            await ctx.Message.DeleteAsync();

            //Ban Reason Input
            string reason = Input.Trim(new char[0]);

            //Random Reasons
            if (reason.ToLower() == "rand")
            {
                string[] randomMSGS = {
                    "I felt like it",
                    "For the shits & giggles",
                    "As i have more perms than you",
                    "Because i can!",
                    "1.6 see rpuk :joy:",
                    "Not a test, you have been banned",
                    "Finger slipped"
                };
                reason = randomMSGS[new Random().Next(0, randomMSGS.Length)];
            }

            //Reason Not Given
            if (reason == "") reason = "Not Given";

            var msg = $"{Admin.DisplayName} banned you. | Reason: {reason}";

            var embed = new DiscordEmbedBuilder { 
                Title = $"User {Target.DisplayName} was banned", 
                Color = DiscordColor.Red 
            };
            embed.AddField("Banned By", Admin.DisplayName, true); //add another field to the embeded view
            embed.AddField("Reason", reason, true); //add another field to the embeded view

            await ctx.Channel.TriggerTypingAsync();
            await Target.SendMessageAsync(msg); //msg them
            await Target.BanAsync(7, reason); //exec ban
            await ctx.Channel.SendMessageAsync(embed);//display in text-chat
        }

        [Command("msg"), Description("msg usr [Requires manage messages persission]")]
        public async Task MsgUser(CommandContext ctx, DiscordMember Target, [RemainingText] string Input = "")
        {
            var Admin = ctx.Member;
            if (!Admin.CanManageMessages().GetAwaiter().GetResult()) return;

            await ctx.Message.DeleteAsync();
            await ctx.Channel.TriggerTypingAsync();

            var embed = new DiscordEmbedBuilder { Title = "", Color = DiscordColor.Green }; //Create Embeded View
            if (Input != null)
            {
                await Target.SendMessageAsync(Input.Trim(new char[0])); //message the target 
                embed.AddField($"Message to {Target.DisplayName} ", "has been sent!", false); //add another field to the embeded view  
            }
            else
            {
                embed.AddField($"Message to {Target.DisplayName} Can't be blank!", "not sent!", false); //add another field to the embeded view   
            }

            await ctx.Channel.SendMessageAsync(embed);//display in text-chat  
           
        }

        [Command("poll")]
        public async Task Poll(CommandContext ctx, TimeSpan duration, params DiscordEmoji[] emojiOptions)
        {
            var interactivity = ctx.Client.GetInteractivity();
            var options = emojiOptions.Select(x => x.ToString());

            var pollEmbed = new DiscordEmbedBuilder
            {
                Title = "Poll",
                Description = string.Join(" ", options)
            };

            var pollMessage = await ctx.Channel.SendMessageAsync(embed: pollEmbed).ConfigureAwait(false);

            foreach (var option in emojiOptions)
            {
                await pollMessage.CreateReactionAsync(option).ConfigureAwait(false);
            }

            var result = await interactivity.CollectReactionsAsync(pollMessage, duration).ConfigureAwait(false);
            var distinctResult = result.Distinct();

            var results = distinctResult.Select(x => $"{x.Emoji}: {x.Total}");

            await ctx.Channel.SendMessageAsync(string.Join("\n", results)).ConfigureAwait(false);
        }
    }
}

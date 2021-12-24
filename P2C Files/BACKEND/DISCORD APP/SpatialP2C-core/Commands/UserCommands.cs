using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Attributes;
using DSharpPlus.Entities;
using System.Threading.Tasks;

/*
    <prefix>user greet @member
*/

namespace SpatialP2CCore.Commands
{
    [Group("user"), Description("Contains some user commands.")]
    public class UserCommands : BaseCommandModule
    {
        [Command("greet"), Description("Says hi to specified user."), Aliases("sayhi", "say_hi")]
        public async Task Greet(CommandContext ctx, [Description("The user to say hi to.")] DiscordMember member) // this command takes a member as an argument; you can pass one by username, nickname, id, or mention
        {
            await ctx.TriggerTypingAsync();

            var emoji = DiscordEmoji.FromName(ctx.Client, ":wave:");

            await ctx.RespondAsync($"{emoji} Hello, {member.Mention}!");
        }
    }
}
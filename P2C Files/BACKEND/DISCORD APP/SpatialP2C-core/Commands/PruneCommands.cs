using SpatialP2CCore.Utils;
using DSharpPlus;
using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Attributes;
using DSharpPlus.Entities;
using System.Linq;
using System.Threading.Tasks;

/*
    <prefix>prune 15
    <prefix>prune user @member 15
    <prefix>prune users 15
    <prefix>prune bot @bot 15
    <prefix>prune bots 15
*/

namespace SpatialP2CCore.Commands
{
    [Group("prune"), Description("Deletes messages."), RequirePermissions(Permissions.ManageMessages)]
    public class PruneCommands : BaseCommandModule
    {
        private const int PruneLimit = 50;

        [GroupCommand]
        public async Task PruneAsync(CommandContext ctx, [RemainingText, Description("Number of messages to remove")] int limit)
        {
            if (!ctx.Member.CanManageMessages().GetAwaiter().GetResult()) return;

            limit = (limit == 0) ? PruneLimit : limit;

            var channel = ctx.Channel;
            var messages = await channel.GetMessagesAsync(limit);
            await ctx.Channel.DeleteMessagesAsync(messages);
            await ctx.TriggerTypingAsync();
            await ctx.RespondAsync($"{ctx.Member.Mention} Deleted {messages.Count} {(limit == 1 ? "message" : "messages")}");
        }

        [Command("user"), Description("Prune user messages.")]
        public async Task PruneUser(CommandContext ctx, [Description("Member to execute as.")] DiscordMember member, [RemainingText, Description("Number of user messages to remove")] int limit)
        {
            if (!ctx.Member.CanManageMessages().GetAwaiter().GetResult()) return;
            await ctx.TriggerTypingAsync();
            await ctx.Message.DeleteAsync();

            limit = (limit == 0) ? PruneLimit : limit;

            var messages = await ctx.Channel.GetMessagesAsync(limit);
            var messagesToDelete = messages.Where(client => client.Author.Id == member.Id && !client.Author.IsBot).ToList();
            await ctx.Channel.DeleteMessagesAsync(messagesToDelete);
            
            await ctx.RespondAsync($"{ctx.Member.Mention} Deleted {messagesToDelete.Count} {(limit == 1 ? "message" : "messages")}");
        }

        [Command("users"), Description("Prune users messages")]
        public async Task PruneUsers(CommandContext ctx, [RemainingText, Description("Number of user messages to remove")] int limit)
        {
            if (!ctx.Member.CanManageMessages().GetAwaiter().GetResult()) return;

            limit = (limit == 0) ? PruneLimit : limit;

            var messages = await ctx.Channel.GetMessagesAsync(limit);
            var messagesToDelete = messages.Where(q => !q.Author.IsBot).ToList();
            await ctx.Channel.DeleteMessagesAsync(messagesToDelete);
            await ctx.TriggerTypingAsync();
            await ctx.RespondAsync($"{ctx.Member.Mention} Deleted {messagesToDelete.Count} {(limit == 1 ? "message" : "messages")}");
        }

        [Command("bot"), Description("Prune bot messages.")]
        public async Task PruneBot(CommandContext ctx, [Description("Member to execute as.")] DiscordMember member, [RemainingText, Description("Number of user messages to remove")] int limit)
        {
            if (!ctx.Member.CanManageMessages().GetAwaiter().GetResult()) return;
            await ctx.TriggerTypingAsync();
            await ctx.Message.DeleteAsync();

            limit = (limit == 0) ? PruneLimit : limit;

            var messages = await ctx.Channel.GetMessagesAsync(limit);
            var messagesToDelete = messages.Where(client => client.Author.Id == member.Id && client.Author.IsBot).ToList();
            await ctx.Channel.DeleteMessagesAsync(messagesToDelete);

            await ctx.RespondAsync($"{ctx.Member.Mention} Deleted {messagesToDelete.Count} {(limit == 1 ? "message" : "messages")}");
        }

        [Command("bots"), Description("Prune bots messages")]
        public async Task PruneBots(CommandContext ctx, [RemainingText, Description("Number of bot messages to remove")] int limit)
        {
            if (!ctx.Member.CanManageMessages().GetAwaiter().GetResult()) return;

            limit = (limit == 0) ? PruneLimit : limit;

            var messages = await ctx.Channel.GetMessagesAsync(limit);
            var messagesToDelete = messages.Where(q => q.Author.IsBot).ToList();
            await ctx.Channel.DeleteMessagesAsync(messagesToDelete);
            await ctx.TriggerTypingAsync();
            await ctx.RespondAsync($"{ctx.Member.Mention} Deleted {messagesToDelete.Count} {(limit == 1 ? "message" : "messages")}"); 
        }
    }
}
using SpatialP2CCore.Utils;
using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Attributes;
using DSharpPlus.Entities;
using System;
using System.Threading.Tasks;
using DSharpPlus;

/*
    <prefix>about
    <prefix>about uptime
    <prefix>about repo
    <prefix>about botlink
*/

namespace SpatialP2CCore.Commands
{
    [Group("about"), Description("Contains info about the bot.")]
    public class AboutCommands : BaseCommandModule
    {
        // this command is a group command. It means it can be invoked by just typing <prefix>about
        [GroupCommand]
        public async Task AboutAsync(CommandContext ctx)
        {
            await ctx.TriggerTypingAsync();

            var client = ctx.Client.CurrentUser;
            var embed = new DiscordEmbedBuilder
            {
                Title = $"About {client.Username}",
                Color = DiscordColor.Chartreuse,
                ImageUrl = client.AvatarUrl,
                Description = $"{client.Username} is a bot written and mananged by <@!319393114815070208>",
                Timestamp = DateTime.UtcNow
            };

            embed.AddField("Name", $"{client.Username}#{client.Discriminator}");
            embed.AddField("ID", client.Id.ToString());
            embed.AddField("Server Count", ctx.Client.Guilds.Count.ToString());
            embed.AddField("Uptime", Client.StartedAt.Uptime());

            await ctx.RespondAsync(embed);

            await InviteBot(ctx);
        }

        [Command("uptime"), Description("Displays the amount of time the bot has been live")]
        public async Task Uptime(CommandContext ctx) => await ctx.RespondAsync($"Uptime: {Client.StartedAt.Uptime()}");

        [Command("repo"), Aliases("gitlink", "github_link"), Description("Gives the Github link to the source code"), Hidden, RequireOwner]
        public async Task Source(CommandContext ctx) => await ctx.RespondAsync($"{Client.BotRepo}");
        
        [Command("botlink"), Aliases("linkbot", "bot_link"), Description("Brings up an invite link for the bot"), Hidden]
        public async Task InviteBot(CommandContext ctx)
        {
            DiscordMember client = ctx.Member;
            if (!client.HasPermission(Permissions.Administrator, ctx).GetAwaiter().GetResult()) return;

            await ctx.TriggerTypingAsync();

            var baseurl = "https://discordapp.com/oauth2/authorize?scope=bot";
            var url = baseurl + "&client_id=" + Client.Info.ID + "&permissions=" + 8;

            await ctx.RespondAsync("", embed: new DiscordEmbedBuilder
            {
                Title = $"Click here to have {Client.Info.Name} join your server",
                Color = DiscordColor.Magenta,
                Url = url
            });
        }
    }
}

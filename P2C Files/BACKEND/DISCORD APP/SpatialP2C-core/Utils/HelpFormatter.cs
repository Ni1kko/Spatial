using System.Collections.Generic;
using System.Linq;
using System.Text;
using DSharpPlus;
using DSharpPlus.CommandsNext;
using DSharpPlus.CommandsNext.Converters;
using DSharpPlus.CommandsNext.Entities;

namespace SpatialP2CCore.Utils
{
    class HelpFormatter : BaseHelpFormatter
    {
        private StringBuilder MessageBuilder { get; }

        public HelpFormatter(CommandContext ctx) : base(ctx)
        {
            MessageBuilder = new StringBuilder();
        }

        // this method is called first, it sets the command
        public override BaseHelpFormatter WithCommand(Command command)
        {
            MessageBuilder.Append("Command: ").AppendLine(Formatter.Bold(command.Name)).AppendLine();
            MessageBuilder.Append("Description: ").AppendLine(command.Description).AppendLine();

            if (command is CommandGroup)MessageBuilder.AppendLine("This group has a standalone command.").AppendLine();

            MessageBuilder.Append("Aliases: ").AppendLine(string.Join(", ", command.Aliases)).AppendLine();

            foreach (var overload in command.Overloads)
            {
                if (overload.Arguments.Count == 0) continue;
                MessageBuilder.Append($"[Overload {overload.Priority}] Arguments: ").AppendLine(string.Join(", ", overload.Arguments.Select(xarg => $"{xarg.Name} ({xarg.Type.Name})"))).AppendLine();
            }

            return this;
        }

        // this method is called second, it sets the current group's subcommands
        // if no group is being processed or current command is not a group, it 
        // won't be called
        public override BaseHelpFormatter WithSubcommands(IEnumerable<Command> subcommands)
        {
            MessageBuilder.Append("Subcommands: ").AppendLine(string.Join(", ", subcommands.Select(xc => xc.Name))).AppendLine();
            return this;
        }

        // this is called as the last method, this should produce the final 
        // message, and return it
        public override CommandHelpMessage Build() => new CommandHelpMessage(MessageBuilder.ToString().Replace("\r\n", "\n"));
    }
}

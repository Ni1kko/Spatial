namespace SpatialP2CCore
{
    public class Info
    {
        /// <summary>
        ///  Discord Server Nickname, you can customize your name in a server that you're in (as long as you've been granted the right permissions).
        /// </summary>
        public string Name { get; set; }

        /// <summary>
        /// Discord Bot Token, a short phrase (represented as a jumble of letters and numbers) that acts as a “key” to controlling a Discord Bot. Tokens are used inside bot code to send commands back and forth to the API, which in turn controls bot actions.
        /// </summary>
        public string Token { get; set; }

        /// <summary>
        /// Discord Client ID, a unique numerical identifier used to identify you within Discord's audit logs and within the system itself. It, along with messages, DMs, channels and servers all have unique IDs to help the system function and help audit logs mean anything at all.
        /// </summary>
        public ulong ID { get; set; }

        /// <summary>
        /// Discord Shard Count
        /// </summary>
        public int ShardCount { get; set; }

        /// <summary>
        ///
        /// </summary>
        public bool Debug { get; set; } = false;

        /// <summary>
        ///
        /// </summary>
        public string Prefix { get; set; }

        /// <summary>
        ///
        /// </summary>
        public bool PrefixSpace { get; set; }

        /// <summary>
        ///
        /// </summary>
        public string[] PresenceStrings { get; set; }
    }
}

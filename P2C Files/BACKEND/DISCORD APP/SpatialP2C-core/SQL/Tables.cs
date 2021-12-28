using System;
using ServiceStack.DataAnnotations;
using SpatialP2CCore.P2C;

namespace SpatialP2CCore.SQL
{
    [Alias("guilds")]
    public class Guilds
    {
        [PrimaryKey, AutoIncrement]
        public int id { get; set; }
        public string name { get; set; }
        public ulong guild_id { get; set; }
        public DateTime created_at { get; set; }
        public DateTime updated_at { get; set; }
    }

    [Alias("members")]
    public class Members
    {
        [PrimaryKey, AutoIncrement]
        public int id { get; set; }
        public ulong userid { get; set; }
        public string username { get; set; }
        public DateTime created_at { get; set; }
        public DateTime updated_at { get; set; }
    }

    [Alias("p2c")]
    public class P2C
    {
        [PrimaryKey, AutoIncrement]
        public int id { get; set; }
        public ulong user_id { get; set; }
        public string user_pass { get; set; } = string.Empty;
        public P2CMemberTypes user_type { get; set; }
        public string vanity_id { get; set; }
        public DateTime created_at { get; set; }
        public DateTime expires_at { get; set; }
    }
}

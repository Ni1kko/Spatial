using System;
using ServiceStack.DataAnnotations;

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
}

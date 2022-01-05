using ServiceStack.OrmLite;
using System.Data;

namespace SpatialP2CCore.SQL
{
    internal class Database
    {
        private protected string Name { get; set; }
        private protected string User { get; set; }
        private protected string Pass { get; set; }
        private protected string Host { get; set; }
        private protected string Port { get; set; }
        private protected string Mode { get; set; }

        private protected OrmLiteConnectionFactory Connection { get; set; }

        private protected string ConnectionString() => $"Server={Host};Port={Port};UID={User};Password={Pass};Database={Name};SslMode={Mode}";

        internal Database(string database = "database_name", string user = "root", string pass = "", string host = "127.0.0.1", string port = "3306", string sslmode = "none")
        {
            Name = database;
            User = user;
            Pass = pass;
            Host = host;
            Port = port;
            Mode = sslmode;
            Connection = new OrmLiteConnectionFactory(ConnectionString(), MySqlDialect.Provider);
        }

        internal IDbConnection Connect() => Connection.Open();
    }
}

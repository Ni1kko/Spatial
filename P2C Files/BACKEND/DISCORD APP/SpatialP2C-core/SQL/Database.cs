using System;
using ServiceStack.OrmLite;
using System.Data;

namespace SpatialP2CCore.SQL
{
    public class Database
    {
        private protected static string Name = "Spatial";
        private protected static string Host = "127.0.0.1";
        private protected static string Port = "3306";
        private protected static string User = "";
        private protected static string Pass = "";

        /// <summary>
        /// Opens a database connection
        /// </summary>
        /// <returns>System.Data.IDbConnection</returns>
        public static IDbConnection Connect()
        {
            var dsn = $"Server={Host};Port={Port};UID={User};Password={Pass};Database={Name};SslMode=none";
            var factory = new OrmLiteConnectionFactory(dsn, MySqlDialect.Provider);

            return factory.Open();
        }
    }
}

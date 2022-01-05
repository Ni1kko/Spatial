using System;
using System.Threading.Tasks; 
using SpatialP2CCore;
using SpatialP2CConsole.Misc;
using Console = SpatialP2CConsole.Misc.Console;

namespace SpatialP2CConsole
{
    internal sealed class Program : Console
    {
        internal static async Task Main()
        {
            await WriteLine("Loading");
            //await NewMessage(MessageType.SYSCOMMAND, Message.MINIMIZE);

            var SpatialP2CCFG = await Settings.ConfigLoad();

            //-- Debuging
            #if DEBUG
                SpatialP2CCFG.Debug = true;
            #endif

            var SpatialP2C = new Client(SpatialP2CCFG);

            try
            {
                await SpatialP2C.Startup();
            }
            catch (Exception ex)
            {
                await WriteLine(ex.Message);
                await ReadLine();
                Environment.Exit(-1);
            }
        }
    }
}

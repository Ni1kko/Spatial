using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Threading.Tasks;
using SpatialP2CConsole.Enums;

namespace SpatialP2CConsole.Misc
{
    internal class Console
    {
        [DllImport("user32.dll")]
        private static extern bool SendMessage(IntPtr hWnd, int msg, int wParam, int lParam);
        internal static async Task<bool> NewMessage(MessageType type, Message message)
        {
            bool output = false;
            int messagpeType = 0x0;

            switch (type)
            {
                case MessageType.SYSCOMMAND: messagpeType = 0x0112; break;
            };

            if (messagpeType.Equals(0x0)) return output;

            switch (message)
            {
                case Message.MINIMIZE: output = SendMessage(Process.GetCurrentProcess().MainWindowHandle, messagpeType, 0x0F020, 0); break;
            };
            await Task.Delay(200);
            return output;
        }
        internal static async Task WriteLine(string contents) => await System.Console.Out.WriteLineAsync(contents);
        internal static async Task<string> ReadLine() => await System.Console.In.ReadLineAsync();
        internal static async Task<string> ReadAll() => await System.Console.In.ReadToEndAsync();
        internal static async Task<ConsoleKeyInfo> ReadKey(int delay = 1)
        {
            await Task.Delay(delay * 60);
            return System.Console.ReadKey();
        }
        
    }
}
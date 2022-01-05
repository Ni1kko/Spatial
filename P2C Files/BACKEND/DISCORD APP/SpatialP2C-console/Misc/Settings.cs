using System;
using System.IO;
using System.Threading.Tasks;
using SpatialP2CCore;
using Newtonsoft.Json;

namespace SpatialP2CConsole.Misc
{
    internal class Settings
    {
        private protected static string ConfigFile => Path.Combine(Environment.CurrentDirectory, "SpatialP2C.json");
        private protected static bool ConfigExists()
        {
            if (File.Exists(ConfigFile))
            {
                var fileContent = File.ReadAllText(ConfigFile);
                bool cflags = !string.IsNullOrWhiteSpace(fileContent) || !string.IsNullOrEmpty(fileContent);
                return cflags;
            }
            return false;
        }
        private protected static bool ConfigCreate()
        {
            try
            {
                var defaults = new Info()
                {
                    Name = "Spatial P2C",
                    Token = "YOUR TOKEN HERE",
                    ID = 1234567890987654321,
                    ShardCount = 1,
                    Prefix = ".",
                    PrefixSpace = false,
                    PresenceStrings = new string[] {
                        "Try {PREFIX}help",
                        "Attempting integer division by zero...",
                        "Do you think it's going to load?",
                        "Error Type Number, Not a Number, Expected Number, Number",
                        "Injecting Backdoor Trojan...",
                        "Making another memory error",
                        "Restarting for no reason...",
                        "Soon™",
                        "You have that read wrong",
                        "Bad module error",
                        "New Account Who Dis?",
                        "Do you kno dawae?",
                        "Error: This game is no longer available",
                        "Headless client midlife crisis: Wants head back",
                        "No Keyboard Detected! Press F1 To Continue",
                        "Rooty Tooty, Point and Shooty!",
                        "Server restart in 1 minute!",
                        "Verify Integrity of Game Files...",
                        "Just a fat little squirrel trying to get a nut",
                        "Preparing to find THE way",
                        "Paying for WinRAR"
                    }
                };
                return ConfigSave(defaults);
            }
            catch (Exception)
            {
                return false;
            }
        }
        private protected static bool ConfigSave(Info SpatialP2CInfo)
        {
            try
            {
                string json = JsonConvert.SerializeObject(SpatialP2CInfo, Formatting.Indented);
                if (string.IsNullOrWhiteSpace(json)) return false;
                File.WriteAllText(ConfigFile, json);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }
        internal protected static async Task<Info> ConfigLoad()
        {
            try
            {
                if (!ConfigExists())
                {
                    if (ConfigCreate())
                    {
                        await Console.WriteLine("SpatialP2C.json Created! Please add you 'Token' and 'ID'.");
                        var input = "";
                        while (input.ToLower() != "c")
                        {
                            await Console.WriteLine("Type 'C' to continue and press enter.");
                            input = await Console.ReadLine();
                        }
                    }
                    else File.Delete(ConfigFile);
                    Environment.Exit(1);
                }

                return JsonConvert.DeserializeObject<Info>(File.ReadAllText(ConfigFile));
            }
            catch (Exception)
            {
                return null;
            }
        }
    }
}
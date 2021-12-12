using System;
using System.Diagnostics;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SpatialLoader
{
    public partial class Form1 : Form
    {
        public Form1() => InitializeComponent();
        
        #nullable enable
        private protected Process? GetProcessByName(string name)
        {
            Process[] instances = Process.GetProcessesByName(name);
            return instances.Length > 0 ? instances[0] : null;
        }

        private protected Process? GetSteamProcess() => GetProcessByName("steam");
         
        private protected Process? GetCSGOProcess() => GetProcessByName("csgo");

        private protected bool SteamRunning() => GetSteamProcess()?.Id > 0;

        private protected bool CSGORunning() => GetCSGOProcess()?.Id > 0;

        private protected bool CloseSteam()
        {
            GetSteamProcess()?.Kill();
            Task.Delay(1 * 1000);
            return SteamRunning();
        }
        
        private protected bool CloseCSGO()
        {
            GetCSGOProcess()?.Kill();
            Task.Delay(1 * 1000);
            return CSGORunning();
        }

        private protected bool LaunchSteam(bool kill = false) 
        {
            //already running
            if (SteamRunning())
            {
                if (kill) 
                    CloseSteam(); 
                else 
                    return true;
            }
             
            _= new Process { StartInfo = new ProcessStartInfo { FileName = "steam.exe"} }.Start();
            Task.Delay(2 * 1000);
            return SteamRunning();
        }
         
        private protected bool LaunchCSGO(bool kill = false)
        {
            //already running
            if (CSGORunning())
            {
                if (kill)
                {
                    UpdateStatus("Exiting CSGO", true);
                    CloseCSGO();
                    Task.Delay(1 * 1000);
                }
                else
                    return true;
            }

            UpdateStatus("Loading CSGO", true);
            Process.Start(@"steam://rungameid/730");
            Task.Delay(2 * 1000);
            UpdateStatus();

            return CSGORunning();
        }

        private protected bool LaunchSteamWithoutVAC()
        {
            if (SteamRunning())
            {
                UpdateStatus("Exiting steam", true);
                CloseSteam();
                Task.Delay(1 * 1000);
            }

            UpdateStatus("Disabling VAC", true);
            Process.Start("VacOff.exe");
            
            return SteamRunning();
        }

        private protected bool LoadSpatial()
        {
            UpdateStatus("Loading Spatial.dll", true);
            new Process(){ StartInfo = new ProcessStartInfo { FileName = "Shtreeba.exe", Verb = "runas" } };
            Task.Delay(1 * 1000); 
            return true;
        }

        private protected void UpdateProgressBar(bool reset = false, bool autocomplete = false)
        {
            if(progressBar.Value >= 100)
            {
                progressBar.Value = reset ? 0 : 100;
            }

            if (autocomplete)
            {
                while (progressBar.Value <= 99)
                {
                    progressBar.Value += 1;
                    Task.Delay(1 * 200);
                }

                Task.Delay(2 * 1000);

                progressBar.Value = 0;
            }
            else
            {
                progressBar.Value += 10;
            }
            
        }

        private protected void UpdateStatus(string status = "Spatial", bool cursorWait = false)
        {
            this.Invoke(new Action(() =>
            { 
                this.Title.Text = status; 
                this.Title.Update();
                if (cursorWait)
                    this.Cursor = Cursors.WaitCursor;
                else
                    this.Cursor = Cursors.Default;
            }));
        }

        private protected void LoadupButton_Click(object sender, EventArgs e)
        {
            UpdateStatus("Loading", true);

            if (DisableVacToggle.Checked)
                LaunchSteamWithoutVAC();
            else
                LaunchSteam();
 
            UpdateProgressBar();

            while (!SteamRunning())
            { 
                UpdateStatus("Waiting for Steam", true);
                Task.Delay(3 * 1000);
                UpdateStatus();
            }

            UpdateProgressBar();

            Task.Delay(1 * 1000);

            LaunchCSGO();

            UpdateProgressBar();

            while (!CSGORunning())
            {
                UpdateStatus("Waiting for CSGO", true);
                Task.Delay(3 * 1000);
                UpdateStatus();
            }

            UpdateProgressBar();

            Task.Delay(3 * 1000);

            LoadSpatial();
              
            UpdateProgressBar(false, true);

            Task.Delay(5 * 1000);

            ExitButton_Click(sender, e);
        }

        private protected void ExitButton_Click(object sender, EventArgs e) => Environment.Exit(1);

    }
}

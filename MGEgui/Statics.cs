using System;
using FileVersionInfo=System.Diagnostics.FileVersionInfo;
using Process=System.Diagnostics.Process;
using File=System.IO.File;
using Directory=System.IO.Directory;
using Serializer=System.Runtime.Serialization.Formatters.Binary.BinaryFormatter;
using Application=System.Windows.Forms.Application;
using MessageBox=System.Windows.Forms.MessageBox;
using MessageBoxButtons=System.Windows.Forms.MessageBoxButtons;
using DialogResult=System.Windows.Forms.DialogResult;
using Registry = Microsoft.Win32.Registry;
using RegistryKey = Microsoft.Win32.RegistryKey;
using System.Windows.Forms;
using System.Collections;
using System.Collections.Generic;
using System.Threading;
using MGEgui.DirectX;
using MGEgui.Localization;
using MGEgui.INI;


namespace MGEgui {

    [Serializable]
    public class Technique {
        public string Description;
        public string Name;
        public string[] Lines;

        public Technique() {
            Description="Description";
            Name="Unnamed";
            Lines=new string[0];
        }
    }

    /// <summary>
    /// The list of different macro types
    /// </summary>
    /// <remarks>
    /// Copied from input.h
    /// Make sure that this is always up to date (Needs Console3, Console4, Offhammer)
    /// </remarks>
    public enum MacroType : byte {
        Unused=0,       //This key isn't assigned to anything
        //FD_Console
        Console1=1,     //Enter console command, exit console
        Console2=2,     //Dont open or close console
        //FD_Press
        Hammer1=3,      //Hammer a key while pressed
        Hammer2=4,      //Start hammering a key
        Unhammer=5,     //Stop hammering a key
        AHammer1=6,     //As Hammer except hammers in alternate frames
        AHammer2=7,
        AUnhammer=8,
        Press1=9,       //Other keys pressed while this is pressed
        Press2=10,       //Press keys when this is pressed, but dont depress
        Unpress=11,      //Depress any pressed keys
        //FD_Timer
        BeginTimer=12,   //Start a timer trigger
        EndTimer=13,    //End a timer trigger
        //FD_Graphics
        Graphics=14     //Perform a graphics function
    }

    /// <summary>
    /// What a particular MacroEditerForm is being used for
    /// </summary>
    public enum MacroFormType { Unknown, Editor, Console, Press, Function, Timer, Trigger }

    [Serializable]
    public struct KeyPress {
        public bool Down;
        public byte Code;

        public KeyPress(byte code,bool down) {
            Down=down;
            Code=code;
        }
    }

    [Serializable]
    public class FD_Console {
        public byte Length;             //The size of the string
        public KeyPress[] KeyCodes;     //A list of keycodes
        public string Description;      //A description of the command
        public FD_Console() {
            Length=0;
            KeyCodes=new KeyPress[Statics.MACROS];
            Description="";
        }
    }
    [Serializable]
    public class FD_Press {
        public bool[] KeyStates;    //Last 8 for mouse
        public FD_Press() {
            KeyStates=new bool[Statics.MACROS];
        }
    }
    [Serializable]
    public class FD_Timer {
        public byte TimerID;           //The timer to activate/deactivate
        public FD_Timer() {
            TimerID=0;
        }
    }
    [Serializable]
    public class FD_Graphics {
        public byte function; //The GraphicsFuncs to call in the fake d3d dll
    }

    [Serializable]
    public class Macro {
        public MacroType Type;
        public FD_Console Console;
        public FD_Press Press;
        public FD_Timer Timer;
        public FD_Graphics Graphics;

        public Macro() {
            Console=new FD_Console();
            Press=new FD_Press();
            Timer=new FD_Timer();
            Graphics=new FD_Graphics();
        }
    }
    [Serializable]
    public class Trigger {
        public uint TimeInterval;
        public bool Active;
        public FD_Press data;

        public Trigger() {
            TimeInterval=0;
            Active=false;
            data=new FD_Press();
        }
    }

    public static class MutexCheck {
        private const string MutexName="MGEguiMutex";
        private static System.Threading.Mutex mutex;

        internal static bool PerformCheck() {
            bool n;
            mutex=new System.Threading.Mutex(true, MutexName, out n);
            return n;
        }
    }

    public class Tip {
        public string [] controls;
        public string tip;
        public string defaultTip;

        public Tip (string [] controls, string tip) {
            this.controls = controls;
            this.tip = tip;
            this.defaultTip = tip;
        }

        public void reset () {
            tip = defaultTip;
        }
    }

    public class Str {
        public string text;
        public string defaultText;

        public Str (string text) {
            this.text = text;
            this.defaultText = text;
        }

        public void reset () {
            text = defaultText;
        }
    }

    /// <summary>
    /// Contains some constant variables, misc static functions and the program entry point
    /// </summary>
    public class Statics {
        /// <summary>
        /// Used to ensure that created options files match the dll versions
        /// </summary>
        public const string VersionString = "3.8.5";
        public const string VersionRevision = "212";
        public const string versionString = "v" + VersionString + "-SVN_rev-" + VersionRevision;
        public const byte SaveVersion = 47;
        public const byte DistantLandVersion = 5;

        public const int MACROS=266;
        public const int TRIGGERS=4;

        /* names of used files */
        public const string fn_nul = "NUL";
        public const string fn_mwini = "Morrowind.ini";
        public const string iniFileName = "MGE3/MGE.ini";
        public const string fn_settings = "MGE3/settings";
        public const string fn_oldsettings = "MGE3/old-settings";
        public const string fn_didata = "MGE3/DInput.data";
        public const string fn_macro = "MGE3/Macros.sav";
        public const string fn_triger = "MGE3/Triggers.sav";
        public const string fn_remap = "MGE3/remap.data";
        public const string fn_mwse = "MGE3/mwse";
        public const string fn_dllog = "MGE3/DistantLand.log";
        public const string fn_dlcrash = "MGE3/distant-land-crash-dump.txt";
        public const string fn_mwedll = "MGE3/Morrowind Enhanced.dll";
        public const string fn_datafiles = "Data files";
        public const string fn_shadsav = "Data Files/shaders/default/active.sav";
        public const string fn_dl = "Data files/distantland";
        public const string fn_dlver = "Data files/distantland/version";
        public const string fn_world = "Data files/distantland/world";
        public const string fn_worldds = "Data files/distantland/world.dds";
        public const string fn_worldn = "Data files/distantland/world_n.tga";
        public const string fn_statics = "Data Files/DistantLand/Statics";
        public const string fn_usagedata = "Data files/distantland/statics/usage.data";
        public const string fn_statmesh = "Data files/distantland/statics/static_meshes";
        public const string fn_stattexbsa = "Data Files/DistantLand/Statics/Textures.bsa";
        /* registry keys */
        public const string reg_mw = @"Software\Bethesda Softworks\Morrowind";
        public const string reg_mwe = @"Software\Morrowind Enhanced";
        public const string reg_mw64 = @"Software\Classes\VirtualStore\MACHINE\SOFTWARE\Wow6432Node\Bethesda Softworks\Morrowind";
        public const string reg_mwe64 = @"Software\Classes\VirtualStore\MACHINE\SOFTWARE\Wow6432Node\Morrowind Enhanced";
        public static String reg_morrowind {
            get {
                return reg_key_bethesda == Registry.CurrentUser ? Statics.reg_mw64 : Statics.reg_mw;
            }
        }
        public static String reg_morrowind_enhanced {
            get {
                return reg_key_bethesda == Registry.CurrentUser ? Statics.reg_mwe64 : Statics.reg_mwe;
            }
        }
        public static RegistryKey reg_key_bethesda {
            get {
                if (m_reg_key_bethesda == null) {
                    RegistryKey key = null;
                    try {
                        key = Registry.LocalMachine.OpenSubKey(Statics.reg_mw, true);       // We try standard Mw key at first.
                    } catch {
                        try {
                            key = Registry.CurrentUser.OpenSubKey(Statics.reg_mw64, true);
                        } catch {}
                    }
                    m_reg_key_bethesda = (key != null) ? (key.Name.StartsWith(Registry.LocalMachine.Name) ? Registry.LocalMachine : Registry.CurrentUser) : Registry.LocalMachine;
                    if (key != null) key.Close();
                }
                return m_reg_key_bethesda;
            }
            set {
                m_reg_key_bethesda = value;
            }
        }
        private static RegistryKey m_reg_key_bethesda = null;

        public static string runDir;

        public static int tipReadSpeed;

        public static LocalizationInterface Localizations;

        public static MainForm mf;

        public static Macro[] Macros=new Macro[MACROS];
        public static Trigger[] Triggers=new Trigger[TRIGGERS];
        public static byte[] Remapper=new byte[256];

        public static readonly Serializer formatter=new Serializer();

        public static readonly System.Globalization.CultureInfo Culture = new System.Globalization.CultureInfo("en-US", false);
        public static readonly System.Globalization.NumberFormatInfo NumFormat = Culture.NumberFormat;
        public static readonly System.Text.Encoding UTF8NoPreamble = new System.Text.UTF8Encoding (false);

        private struct CommandArgs {
            public readonly bool styles;
            public readonly bool mutex;

            public CommandArgs(string[] args) {
                styles=true;
                mutex=true;
                foreach(string s in args) {
                    switch(s) {
                        case "-nostyles":
                            styles=false;
                            break;
                        case "-nomutex":
                            mutex=false;
                            break;
                    }
                }
            }
        }

        public static Control [] GetChildControls (Control ctrl) {
            ArrayList controls = new ArrayList ();
            foreach (Control c in ctrl.Controls) {
                controls.Add (c);
                controls.AddRange (GetChildControls (c));
            }
            return (Control [])controls.ToArray (typeof (Control));
        }

        #region strings
        public static Dictionary<string, Str> strings = new Dictionary<string, Str> {
            {"Error", new Str (
                "Error")},
            {"Warning", new Str (
                "Warning")},
            {"Message", new Str (
                "Message")},
            {"Close", new Str (
                "&Close")},
            {"MGEguiRunning", new Str (
                "MGEgui.exe is already running.")},
            {"MWRunning", new Str (
                "Morrowind appears to be currently running.\n" +
                "Please quit the game before executing MGEgui.exe")},
            {"NotMWDir", new Str (
                "MGE must be installed to the Morrowind directory.")},
            {"MGEMissing", new Str (
                "One or more of MGE's files appear to be missing. Please reinstall.")},
            {"MWIncompat", new Str (
                "Your version of Morrowind is not compatible with MGE.\n" +
                "MGE requires a fully patched copy of Bloodmoon (i.e. Morrowind version 1.6.1820).")},
            {"MWCorrupt", new Str (
                "Morrowind.exe appears to be corrupt.\n" +
                "MGE is unable to determine Morrowind's version.")},
            {"DSound", new Str (
                "You appear to have part of an old version of MGE installed.\n" +
                "You need to remove dsound.dll from your Morrowind directory for this version of MGE to function.")},
            {"MWRegistry", new Str (
                "Unable to find Morrowind registry keys. Please run the game before installing MGE.")},
            {"MgeAccess", new Str (
                "Unable to write to registry. Please run MGE as administrator.")},
            {"Translation", new Str (
                "")}
        };
        #endregion

        /// <summary>
        /// Entry point for this program
        /// </summary>
        [STAThread]
        public static void Main(string[] sargs) {
            Thread.CurrentThread.CurrentUICulture = Culture;
            CommandArgs args = new CommandArgs (sargs);

            if (args.styles) Application.EnableVisualStyles ();

            Directory.SetCurrentDirectory (System.IO.Path.GetDirectoryName (Application.ExecutablePath));

            Localizations = new LocalizationInterface ();
            try {
                string [] langfiles = Directory.GetFiles ("MGE3", "*.lng");
                foreach (string langfile in langfiles) {
                    Localizations.Add (langfile);
                }
            } catch { };
            LocalizationInterface.Localization language = null;
            bool autoLanguage = true;
            try {
                INIFile MGEini = new INIFile (iniFileName, new INIFile.INIVariableDef [] { INIFile.iniDefEmpty, MainForm.iniLanguage, MainForm.iniAutoLang });
                language = Localizations [MGEini.getKeyString ("Language")];
                autoLanguage = (MGEini.getKeyValue ("AutoLang") == 1);
                if (language != null) Localizations.ApplyStrings ("", strings, language);
            } catch { }

            if (args.mutex && !MutexCheck.PerformCheck ()) {
                MessageBox.Show (strings ["MGEguiRunning"].text, strings ["Error"].text);
                return;
            }
            Process [] morrowind = Process.GetProcessesByName ("Morrowind");
            foreach (Process p in morrowind) {
                MessageBox.Show (strings ["MWRunning"].text, strings ["Error"].text);
                return;
            }

            if (!File.Exists ("./morrowind.exe") || !File.Exists ("./morrowind.ini") || !Directory.Exists ("data files")) {
                MessageBox.Show (strings ["NotMWDir"].text, strings ["Error"].text);
                return;
            }
            if (!Directory.Exists ("MGE3") || !File.Exists ("./MGE3/MGEfuncs.dll") || !File.Exists ("./d3d8.dll") ||
               !File.Exists ("./dinput8.dll")) {
                MessageBox.Show (strings ["MGEMissing"].text, strings ["Error"].text);
                return;
            }
            //Morrowind version info
            try {
                FileVersionInfo MorrowVersion = FileVersionInfo.GetVersionInfo ("Morrowind.exe");
                if (MorrowVersion.ProductPrivatePart != 722 && MorrowVersion.ProductPrivatePart != 1029 && MorrowVersion.ProductPrivatePart != 1875 && MorrowVersion.ProductPrivatePart != 1820)
				{
                    MessageBox.Show (strings ["MWIncompat"].text, strings ["Error"].text);
                    return;
                }
            } catch {
                MessageBox.Show (strings ["MWCorrupt"].text, strings ["Error"].text);
                return;
            }
            //Check for dsound.dll
            if (File.Exists ("dsound.dll") && new System.IO.FileInfo ("dsound.dll").Length == 16384) {
                try {
                    File.Delete ("dsound.dll");
                } catch {
                    MessageBox.Show (strings ["DSound"].text, strings ["Error"].text);
                    return;
                }
            }

            runDir = System.Windows.Forms.Application.StartupPath;
            //check if MW registry keys exist
            RegistryKey key = Statics.reg_key_bethesda.OpenSubKey(Statics.reg_morrowind);
            if (key != null) { key.Close(); key = null;
            } else {
                MessageBox.Show (strings ["MWRegistry"].text, strings ["Error"].text);
                return;
            }

            //mendres: Check if MGE needs administrator privileges.
            try {
                key = Statics.reg_key_bethesda.OpenSubKey(Statics.reg_morrowind, true);
            } catch (System.Security.SecurityException) {
                MessageBox.Show(strings["MgeAccess"].text, strings["Error"].text);
                return;
            }
            if (key != null) { key.Close(); key = null; }

            //Create the data files directories MGE uses
            if (!Directory.Exists (@"data files\shaders")) {
                Directory.CreateDirectory (@"data files\shaders");
            }
            if (!Directory.Exists (@"data files\MGE meshes")) {
                Directory.CreateDirectory (@"data files\MGE meshes");
            }
            if (!Directory.Exists (@"data files\Textures\MGE")) {
                Directory.CreateDirectory (@"data files\Textures\MGE");
            }
            if (!Directory.Exists (@"data files\MGE videos")) {
                Directory.CreateDirectory (@"data files\MGE videos");
            }
            if (!Directory.Exists (@"data files\shaders\default")) {
                Directory.CreateDirectory (@"data files\shaders\default");
            }
            if (!Directory.Exists (@"data files\shaders\water")) {
                Directory.CreateDirectory (@"data files\shaders\water");
            }
            //Create some structures
            for (int i = 0; i < MACROS; i++) {
                Macros [i] = new Macro ();
            }
            for (int i = 0; i < TRIGGERS; i++) {
                Triggers [i] = new Trigger ();
            }
            DXMain.GetDeviceCaps ();
            mf = new MainForm (language, autoLanguage);
            Application.Run (mf);
        }

    }
}
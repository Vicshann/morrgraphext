using System;
using System.Windows.Forms;
using System.Threading;
using System.ComponentModel;

namespace MGEgui {
    public class RenderWindow : Form {
#region Form designer stuff
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components=null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if(disposing&&(components!=null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(RenderWindow));
            this.cbShader = new System.Windows.Forms.CheckBox();
            this.fpsLabel = new System.Windows.Forms.Label();
            this.fpsTimer = new System.Windows.Forms.Timer(this.components);
            this.RenderPanel = new System.Windows.Forms.Panel();
            this.bBenchmark = new System.Windows.Forms.Button();
            this.cmbCycles = new System.Windows.Forms.ComboBox();
            this.DudMenu = new System.Windows.Forms.ContextMenu();
            this.var_HDR = new System.Windows.Forms.NumericUpDown();
            this.hdrLabel = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.var_HDR)).BeginInit();
            this.SuspendLayout();
            // 
            // cbShader
            // 
            this.cbShader.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.cbShader.Checked = true;
            this.cbShader.CheckState = System.Windows.Forms.CheckState.Checked;
            this.cbShader.Location = new System.Drawing.Point(12, 531);
            this.cbShader.Name = "cbShader";
            this.cbShader.Size = new System.Drawing.Size(100, 17);
            this.cbShader.TabIndex = 0;
            this.cbShader.Text = "Enable shader";
            this.cbShader.CheckedChanged += new System.EventHandler(this.cbShader_CheckedChanged);
            // 
            // fpsLabel
            // 
            this.fpsLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.fpsLabel.AutoSize = true;
            this.fpsLabel.Location = new System.Drawing.Point(121, 533);
            this.fpsLabel.Name = "fpsLabel";
            this.fpsLabel.Size = new System.Drawing.Size(39, 13);
            this.fpsLabel.TabIndex = 0;
            this.fpsLabel.Text = "FPS: 0";
            // 
            // fpsTimer
            // 
            this.fpsTimer.Enabled = true;
            this.fpsTimer.Interval = 1000;
            this.fpsTimer.Tick += new System.EventHandler(this.fpsTimer_Tick);
            // 
            // RenderPanel
            // 
            this.RenderPanel.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.RenderPanel.Location = new System.Drawing.Point(12, 12);
            this.RenderPanel.Name = "RenderPanel";
            this.RenderPanel.Size = new System.Drawing.Size(512, 512);
            this.RenderPanel.TabIndex = 0;
            // 
            // bBenchmark
            // 
            this.bBenchmark.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.bBenchmark.Location = new System.Drawing.Point(449, 528);
            this.bBenchmark.Name = "bBenchmark";
            this.bBenchmark.Size = new System.Drawing.Size(75, 23);
            this.bBenchmark.TabIndex = 2;
            this.bBenchmark.Text = "Benchmark";
            this.bBenchmark.Click += new System.EventHandler(this.bBenchmark_Click);
            // 
            // cmbCycles
            // 
            this.cmbCycles.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.cmbCycles.ContextMenu = this.DudMenu;
            this.cmbCycles.Items.AddRange(new object[] {
            "50",
            "100",
            "500",
            "1000"});
            this.cmbCycles.Location = new System.Drawing.Point(354, 529);
            this.cmbCycles.Name = "cmbCycles";
            this.cmbCycles.Size = new System.Drawing.Size(89, 21);
            this.cmbCycles.TabIndex = 1;
            this.cmbCycles.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.cmbCycles_KeyPress);
            // 
            // var_HDR
            // 
            this.var_HDR.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.var_HDR.DecimalPlaces = 2;
            this.var_HDR.Increment = new decimal(new int[] {
            5,
            0,
            0,
            131072});
            this.var_HDR.Location = new System.Drawing.Point(237, 529);
            this.var_HDR.Maximum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.var_HDR.Name = "var_HDR";
            this.var_HDR.Size = new System.Drawing.Size(82, 20);
            this.var_HDR.TabIndex = 3;
            // 
            // hdrLabel
            // 
            this.hdrLabel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.hdrLabel.AutoSize = true;
            this.hdrLabel.Location = new System.Drawing.Point(197, 533);
            this.hdrLabel.Name = "hdrLabel";
            this.hdrLabel.Size = new System.Drawing.Size(34, 13);
            this.hdrLabel.TabIndex = 4;
            this.hdrLabel.Text = "HDR:";
            // 
            // RenderWindow
            // 
            this.Icon = Properties.Resources.AppIcon;
            this.ClientSize = new System.Drawing.Size(538, 554);
            this.Controls.Add(this.hdrLabel);
            this.Controls.Add(this.var_HDR);
            this.Controls.Add(this.RenderPanel);
            this.Controls.Add(this.cmbCycles);
            this.Controls.Add(this.fpsLabel);
            this.Controls.Add(this.bBenchmark);
            this.Controls.Add(this.cbShader);
            this.MinimumSize = new System.Drawing.Size(546, 581);
            this.Name = "RenderWindow";
            this.Text = "Preview Window";
            this.Shown += new System.EventHandler(this.RenderWindow_Shown);
            this.Closing += new System.ComponentModel.CancelEventHandler(this.RenderWindow_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.var_HDR)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        private CheckBox cbShader;
        private Label fpsLabel;
        private Panel RenderPanel;
        private Button bBenchmark;
        private ComboBox cmbCycles;
        private ContextMenu DudMenu;
		private NumericUpDown var_HDR;
		private Label hdrLabel;
        private System.Windows.Forms.Timer fpsTimer;

        #endregion
#endregion
        public RenderWindow() {
            InitializeComponent();
            panel=RenderPanel;
            UseShader=true;
            This=this;
            cmbCycles.SelectedIndex=1;
        }

        private static RenderWindow This;
        public static Control panel;
        public static Thread thread;
        private static volatile float fps=0;
        private static volatile bool UseShader;
        private static volatile bool Exit=false;

        public static void ThreadStart() {
            int frames;
            DateTime now, then;
            TimeSpan second=TimeSpan.FromSeconds(1);
            while(true) {
                now=DateTime.Now;
                then=now+second;
                frames=0;
                while(DateTime.Now<then) {
                    try { DirectX.Shaders.RenderFrame(UseShader, This.var_HDR.Value); } catch { }
                    frames++;
                }
                fps=(float)frames/(float)(DateTime.Now-now).TotalSeconds;

                if(Exit) return;
            }
        }

        private void RenderWindow_FormClosing(object sender,CancelEventArgs e) {
            if(thread!=null) {
                Exit=true;
                thread.Join();
                Exit=false;
            }
        }

        private void fpsTimer_Tick(object sender,EventArgs e) {
            fpsLabel.Text = "FPS: " + Math.Round(fps, 2).ToString();
            //fpsLabel.Text="FPS: "+fps.ToString();
        }

        private void cbShader_CheckedChanged(object sender,EventArgs e) {
            UseShader=cbShader.Checked;
        }

        private void cmbCycles_KeyPress(object sender,KeyPressEventArgs e) {
            e.Handled=true;
        }

        private void bBenchmark_Click(object sender,EventArgs e) {
            int cycles;
            switch(cmbCycles.SelectedIndex) {
                case 0: cycles=50; break;
                case 1: cycles=100; break;
                case 2: cycles=500; break;
                case 3: cycles=1000; break;
                default: cycles=100; break;
            }
            thread.Suspend();
            DateTime start;
            TimeSpan standard,shader;
            start=DateTime.Now;
            for(int i=0;i<cycles;i++) {
				DirectX.Shaders.RenderFrame(false, This.var_HDR.Value);
            }
            standard=DateTime.Now-start;
            start=DateTime.Now;
            for(int i=0;i<cycles;i++) {
				DirectX.Shaders.RenderFrame(true, This.var_HDR.Value);
            }
            shader=DateTime.Now-start;
            double PerfHit=(((double)shader.Ticks/(double)standard.Ticks)-1.0)*100.0;
            double FrameTime=((double)shader.Milliseconds/1000.0)/(double)cycles;
            MessageBox.Show("Completed "+cycles.ToString()+" frames.\n\n"+
                "Standard time: "+standard.ToString()+"\n"+
                "Shaded time: "+shader.ToString()+"\n"+
                "Time spent rendering each shaded frame: "+FrameTime.ToString("G5")+" seconds\n"+
                "Shader performance hit: "+PerfHit.ToString("G5")+"%\n\n"+
                "Estimated reduction in Morrowind fps:\n"+
                "60: "+(1.0/((1.0/60.0)+FrameTime)).ToString("G5")+"\n"+
                "50: "+(1.0/((1.0/50.0)+FrameTime)).ToString("G5")+"\n"+
                "40: "+(1.0/((1.0/40.0)+FrameTime)).ToString("G5")+"\n"+
                "30: "+(1.0/((1.0/30.0)+FrameTime)).ToString("G5")+"\n"+
                "20: "+(1.0/((1.0/20.0)+FrameTime)).ToString("G5")+"\n"+
                "10: "+(1.0/((1.0/10.0)+FrameTime)).ToString("G5")+"\n\n"+
                "This fps hit is calculated by assuming that morrowind is limited only by your graphics card\n"+
                "Because morrowind is usually cpu limited, the real fps hit is likely to be far lower."
                ,"result");
            thread.Resume();
        }

        private void RenderWindow_Shown(object sender, EventArgs e) {
            thread=new Thread(new ThreadStart(ThreadStart));
            thread.Priority=ThreadPriority.BelowNormal;
            RenderWindow.thread.Start();
        }

    }
}

namespace SpatialLoader
{
    partial class Form1
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderEdges borderEdges1 = new Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderEdges();
            Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderEdges borderEdges2 = new Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderEdges();
            this.TopPanel = new Bunifu.UI.WinForms.BunifuPanel();
            this.Title = new System.Windows.Forms.Label();
            this.ExitButton = new Bunifu.UI.WinForms.BunifuButton.BunifuButton();
            this.bottomPanel = new Bunifu.UI.WinForms.BunifuPanel();
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.bunifuLabel1 = new Bunifu.UI.WinForms.BunifuLabel();
            this.DisableVacToggle = new Bunifu.UI.WinForms.BunifuCheckBox();
            this.LoadupButton = new Bunifu.UI.WinForms.BunifuButton.BunifuButton();
            this.bunifuDragControl1 = new Bunifu.Framework.UI.BunifuDragControl(this.components);
            this.TopPanel.SuspendLayout();
            this.bottomPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // TopPanel
            // 
            this.TopPanel.BackgroundColor = System.Drawing.Color.Transparent;
            this.TopPanel.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("TopPanel.BackgroundImage")));
            this.TopPanel.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.TopPanel.BorderColor = System.Drawing.Color.Transparent;
            this.TopPanel.BorderRadius = 3;
            this.TopPanel.BorderThickness = 1;
            this.TopPanel.Controls.Add(this.Title);
            this.TopPanel.Controls.Add(this.ExitButton);
            this.TopPanel.Location = new System.Drawing.Point(0, 0);
            this.TopPanel.Name = "TopPanel";
            this.TopPanel.ShowBorders = true;
            this.TopPanel.Size = new System.Drawing.Size(241, 28);
            this.TopPanel.TabIndex = 0;
            // 
            // Title
            // 
            this.Title.AutoSize = true;
            this.Title.Font = new System.Drawing.Font("Montserrat", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.Title.Location = new System.Drawing.Point(3, 3);
            this.Title.Name = "Title";
            this.Title.Size = new System.Drawing.Size(71, 22);
            this.Title.TabIndex = 4;
            this.Title.Text = "Spatial";
            // 
            // ExitButton
            // 
            this.ExitButton.AllowAnimations = true;
            this.ExitButton.AllowMouseEffects = true;
            this.ExitButton.AllowToggling = false;
            this.ExitButton.AnimationSpeed = 200;
            this.ExitButton.AutoGenerateColors = false;
            this.ExitButton.AutoRoundBorders = false;
            this.ExitButton.AutoSizeLeftIcon = true;
            this.ExitButton.AutoSizeRightIcon = true;
            this.ExitButton.BackColor = System.Drawing.Color.Transparent;
            this.ExitButton.BackColor1 = System.Drawing.Color.DodgerBlue;
            this.ExitButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("ExitButton.BackgroundImage")));
            this.ExitButton.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.ExitButton.ButtonText = "X";
            this.ExitButton.ButtonTextMarginLeft = 0;
            this.ExitButton.ColorContrastOnClick = 45;
            this.ExitButton.ColorContrastOnHover = 45;
            this.ExitButton.Cursor = System.Windows.Forms.Cursors.Default;
            borderEdges1.BottomLeft = true;
            borderEdges1.BottomRight = true;
            borderEdges1.TopLeft = true;
            borderEdges1.TopRight = true;
            this.ExitButton.CustomizableEdges = borderEdges1;
            this.ExitButton.DialogResult = System.Windows.Forms.DialogResult.None;
            this.ExitButton.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(191)))), ((int)(((byte)(191)))), ((int)(((byte)(191)))));
            this.ExitButton.DisabledFillColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.ExitButton.DisabledForecolor = System.Drawing.Color.FromArgb(((int)(((byte)(168)))), ((int)(((byte)(160)))), ((int)(((byte)(168)))));
            this.ExitButton.FocusState = Bunifu.UI.WinForms.BunifuButton.BunifuButton.ButtonStates.Pressed;
            this.ExitButton.Font = new System.Drawing.Font("Montserrat", 9.749999F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.ExitButton.ForeColor = System.Drawing.Color.White;
            this.ExitButton.IconLeftAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.ExitButton.IconLeftCursor = System.Windows.Forms.Cursors.Default;
            this.ExitButton.IconLeftPadding = new System.Windows.Forms.Padding(11, 3, 3, 3);
            this.ExitButton.IconMarginLeft = 11;
            this.ExitButton.IconPadding = 10;
            this.ExitButton.IconRightAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.ExitButton.IconRightCursor = System.Windows.Forms.Cursors.Default;
            this.ExitButton.IconRightPadding = new System.Windows.Forms.Padding(3, 3, 7, 3);
            this.ExitButton.IconSize = 25;
            this.ExitButton.IdleBorderColor = System.Drawing.Color.DodgerBlue;
            this.ExitButton.IdleBorderRadius = 1;
            this.ExitButton.IdleBorderThickness = 1;
            this.ExitButton.IdleFillColor = System.Drawing.Color.DodgerBlue;
            this.ExitButton.IdleIconLeftImage = null;
            this.ExitButton.IdleIconRightImage = null;
            this.ExitButton.IndicateFocus = false;
            this.ExitButton.Location = new System.Drawing.Point(211, 0);
            this.ExitButton.Name = "ExitButton";
            this.ExitButton.OnDisabledState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(191)))), ((int)(((byte)(191)))), ((int)(((byte)(191)))));
            this.ExitButton.OnDisabledState.BorderRadius = 1;
            this.ExitButton.OnDisabledState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.ExitButton.OnDisabledState.BorderThickness = 1;
            this.ExitButton.OnDisabledState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.ExitButton.OnDisabledState.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(168)))), ((int)(((byte)(160)))), ((int)(((byte)(168)))));
            this.ExitButton.OnDisabledState.IconLeftImage = null;
            this.ExitButton.OnDisabledState.IconRightImage = null;
            this.ExitButton.onHoverState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.ExitButton.onHoverState.BorderRadius = 1;
            this.ExitButton.onHoverState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.ExitButton.onHoverState.BorderThickness = 1;
            this.ExitButton.onHoverState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.ExitButton.onHoverState.ForeColor = System.Drawing.Color.White;
            this.ExitButton.onHoverState.IconLeftImage = null;
            this.ExitButton.onHoverState.IconRightImage = null;
            this.ExitButton.OnIdleState.BorderColor = System.Drawing.Color.DodgerBlue;
            this.ExitButton.OnIdleState.BorderRadius = 1;
            this.ExitButton.OnIdleState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.ExitButton.OnIdleState.BorderThickness = 1;
            this.ExitButton.OnIdleState.FillColor = System.Drawing.Color.DodgerBlue;
            this.ExitButton.OnIdleState.ForeColor = System.Drawing.Color.White;
            this.ExitButton.OnIdleState.IconLeftImage = null;
            this.ExitButton.OnIdleState.IconRightImage = null;
            this.ExitButton.OnPressedState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(96)))), ((int)(((byte)(144)))));
            this.ExitButton.OnPressedState.BorderRadius = 1;
            this.ExitButton.OnPressedState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.ExitButton.OnPressedState.BorderThickness = 1;
            this.ExitButton.OnPressedState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(96)))), ((int)(((byte)(144)))));
            this.ExitButton.OnPressedState.ForeColor = System.Drawing.Color.White;
            this.ExitButton.OnPressedState.IconLeftImage = null;
            this.ExitButton.OnPressedState.IconRightImage = null;
            this.ExitButton.Size = new System.Drawing.Size(27, 25);
            this.ExitButton.TabIndex = 3;
            this.ExitButton.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.ExitButton.TextAlignment = System.Windows.Forms.HorizontalAlignment.Center;
            this.ExitButton.TextMarginLeft = 0;
            this.ExitButton.TextPadding = new System.Windows.Forms.Padding(0);
            this.ExitButton.UseDefaultRadiusAndThickness = true;
            this.ExitButton.Click += new System.EventHandler(this.ExitButton_Click);
            // 
            // bottomPanel
            // 
            this.bottomPanel.BackgroundColor = System.Drawing.Color.Transparent;
            this.bottomPanel.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("bottomPanel.BackgroundImage")));
            this.bottomPanel.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.bottomPanel.BorderColor = System.Drawing.Color.Transparent;
            this.bottomPanel.BorderRadius = 3;
            this.bottomPanel.BorderThickness = 1;
            this.bottomPanel.Controls.Add(this.progressBar);
            this.bottomPanel.Controls.Add(this.bunifuLabel1);
            this.bottomPanel.Controls.Add(this.DisableVacToggle);
            this.bottomPanel.Controls.Add(this.LoadupButton);
            this.bottomPanel.Location = new System.Drawing.Point(0, 27);
            this.bottomPanel.Name = "bottomPanel";
            this.bottomPanel.ShowBorders = true;
            this.bottomPanel.Size = new System.Drawing.Size(238, 79);
            this.bottomPanel.TabIndex = 1;
            // 
            // progressBar
            // 
            this.progressBar.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.progressBar.Location = new System.Drawing.Point(0, 69);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(238, 10);
            this.progressBar.TabIndex = 3;
            this.progressBar.Value = 1;
            // 
            // bunifuLabel1
            // 
            this.bunifuLabel1.AllowParentOverrides = false;
            this.bunifuLabel1.AutoEllipsis = false;
            this.bunifuLabel1.Cursor = System.Windows.Forms.Cursors.Default;
            this.bunifuLabel1.CursorType = System.Windows.Forms.Cursors.Default;
            this.bunifuLabel1.Font = new System.Drawing.Font("Montserrat", 9.749999F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.bunifuLabel1.Location = new System.Drawing.Point(23, 19);
            this.bunifuLabel1.Name = "bunifuLabel1";
            this.bunifuLabel1.RightToLeft = System.Windows.Forms.RightToLeft.No;
            this.bunifuLabel1.Size = new System.Drawing.Size(80, 18);
            this.bunifuLabel1.TabIndex = 2;
            this.bunifuLabel1.Text = "Disable VAC";
            this.bunifuLabel1.TextAlignment = System.Drawing.ContentAlignment.TopLeft;
            this.bunifuLabel1.TextFormat = Bunifu.UI.WinForms.BunifuLabel.TextFormattingOptions.Default;
            // 
            // DisableVacToggle
            // 
            this.DisableVacToggle.AllowBindingControlAnimation = true;
            this.DisableVacToggle.AllowBindingControlColorChanges = false;
            this.DisableVacToggle.AllowBindingControlLocation = true;
            this.DisableVacToggle.AllowCheckBoxAnimation = false;
            this.DisableVacToggle.AllowCheckmarkAnimation = true;
            this.DisableVacToggle.AllowOnHoverStates = true;
            this.DisableVacToggle.AutoCheck = true;
            this.DisableVacToggle.BackColor = System.Drawing.Color.Transparent;
            this.DisableVacToggle.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("DisableVacToggle.BackgroundImage")));
            this.DisableVacToggle.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Zoom;
            this.DisableVacToggle.BindingControlPosition = Bunifu.UI.WinForms.BunifuCheckBox.BindingControlPositions.Right;
            this.DisableVacToggle.BorderRadius = 12;
            this.DisableVacToggle.Checked = false;
            this.DisableVacToggle.CheckState = Bunifu.UI.WinForms.BunifuCheckBox.CheckStates.Unchecked;
            this.DisableVacToggle.Cursor = System.Windows.Forms.Cursors.Default;
            this.DisableVacToggle.CustomCheckmarkImage = null;
            this.DisableVacToggle.Location = new System.Drawing.Point(23, 39);
            this.DisableVacToggle.MinimumSize = new System.Drawing.Size(17, 17);
            this.DisableVacToggle.Name = "DisableVacToggle";
            this.DisableVacToggle.OnCheck.BorderColor = System.Drawing.Color.DodgerBlue;
            this.DisableVacToggle.OnCheck.BorderRadius = 12;
            this.DisableVacToggle.OnCheck.BorderThickness = 2;
            this.DisableVacToggle.OnCheck.CheckBoxColor = System.Drawing.Color.DodgerBlue;
            this.DisableVacToggle.OnCheck.CheckmarkColor = System.Drawing.Color.White;
            this.DisableVacToggle.OnCheck.CheckmarkThickness = 2;
            this.DisableVacToggle.OnDisable.BorderColor = System.Drawing.Color.LightGray;
            this.DisableVacToggle.OnDisable.BorderRadius = 12;
            this.DisableVacToggle.OnDisable.BorderThickness = 2;
            this.DisableVacToggle.OnDisable.CheckBoxColor = System.Drawing.Color.Transparent;
            this.DisableVacToggle.OnDisable.CheckmarkColor = System.Drawing.Color.LightGray;
            this.DisableVacToggle.OnDisable.CheckmarkThickness = 2;
            this.DisableVacToggle.OnHoverChecked.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.DisableVacToggle.OnHoverChecked.BorderRadius = 12;
            this.DisableVacToggle.OnHoverChecked.BorderThickness = 2;
            this.DisableVacToggle.OnHoverChecked.CheckBoxColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.DisableVacToggle.OnHoverChecked.CheckmarkColor = System.Drawing.Color.White;
            this.DisableVacToggle.OnHoverChecked.CheckmarkThickness = 2;
            this.DisableVacToggle.OnHoverUnchecked.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.DisableVacToggle.OnHoverUnchecked.BorderRadius = 12;
            this.DisableVacToggle.OnHoverUnchecked.BorderThickness = 1;
            this.DisableVacToggle.OnHoverUnchecked.CheckBoxColor = System.Drawing.Color.Transparent;
            this.DisableVacToggle.OnUncheck.BorderColor = System.Drawing.Color.DarkGray;
            this.DisableVacToggle.OnUncheck.BorderRadius = 12;
            this.DisableVacToggle.OnUncheck.BorderThickness = 1;
            this.DisableVacToggle.OnUncheck.CheckBoxColor = System.Drawing.Color.Transparent;
            this.DisableVacToggle.Size = new System.Drawing.Size(19, 19);
            this.DisableVacToggle.Style = Bunifu.UI.WinForms.BunifuCheckBox.CheckBoxStyles.Bunifu;
            this.DisableVacToggle.TabIndex = 1;
            this.DisableVacToggle.ThreeState = false;
            this.DisableVacToggle.ToolTipText = null;
            // 
            // LoadupButton
            // 
            this.LoadupButton.AllowAnimations = true;
            this.LoadupButton.AllowMouseEffects = true;
            this.LoadupButton.AllowToggling = false;
            this.LoadupButton.AnimationSpeed = 200;
            this.LoadupButton.AutoGenerateColors = false;
            this.LoadupButton.AutoRoundBorders = false;
            this.LoadupButton.AutoSizeLeftIcon = true;
            this.LoadupButton.AutoSizeRightIcon = true;
            this.LoadupButton.BackColor = System.Drawing.Color.Transparent;
            this.LoadupButton.BackColor1 = System.Drawing.Color.DodgerBlue;
            this.LoadupButton.BackgroundImage = ((System.Drawing.Image)(resources.GetObject("LoadupButton.BackgroundImage")));
            this.LoadupButton.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.LoadupButton.ButtonText = "Load Up";
            this.LoadupButton.ButtonTextMarginLeft = 0;
            this.LoadupButton.ColorContrastOnClick = 45;
            this.LoadupButton.ColorContrastOnHover = 45;
            this.LoadupButton.Cursor = System.Windows.Forms.Cursors.Default;
            borderEdges2.BottomLeft = true;
            borderEdges2.BottomRight = true;
            borderEdges2.TopLeft = true;
            borderEdges2.TopRight = true;
            this.LoadupButton.CustomizableEdges = borderEdges2;
            this.LoadupButton.DialogResult = System.Windows.Forms.DialogResult.None;
            this.LoadupButton.DisabledBorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(191)))), ((int)(((byte)(191)))), ((int)(((byte)(191)))));
            this.LoadupButton.DisabledFillColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.LoadupButton.DisabledForecolor = System.Drawing.Color.FromArgb(((int)(((byte)(168)))), ((int)(((byte)(160)))), ((int)(((byte)(168)))));
            this.LoadupButton.FocusState = Bunifu.UI.WinForms.BunifuButton.BunifuButton.ButtonStates.Pressed;
            this.LoadupButton.Font = new System.Drawing.Font("Montserrat", 8.249999F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.LoadupButton.ForeColor = System.Drawing.Color.White;
            this.LoadupButton.IconLeftAlign = System.Drawing.ContentAlignment.MiddleLeft;
            this.LoadupButton.IconLeftCursor = System.Windows.Forms.Cursors.Default;
            this.LoadupButton.IconLeftPadding = new System.Windows.Forms.Padding(11, 3, 3, 3);
            this.LoadupButton.IconMarginLeft = 11;
            this.LoadupButton.IconPadding = 10;
            this.LoadupButton.IconRightAlign = System.Drawing.ContentAlignment.MiddleRight;
            this.LoadupButton.IconRightCursor = System.Windows.Forms.Cursors.Default;
            this.LoadupButton.IconRightPadding = new System.Windows.Forms.Padding(3, 3, 7, 3);
            this.LoadupButton.IconSize = 25;
            this.LoadupButton.IdleBorderColor = System.Drawing.Color.DodgerBlue;
            this.LoadupButton.IdleBorderRadius = 1;
            this.LoadupButton.IdleBorderThickness = 1;
            this.LoadupButton.IdleFillColor = System.Drawing.Color.DodgerBlue;
            this.LoadupButton.IdleIconLeftImage = null;
            this.LoadupButton.IdleIconRightImage = null;
            this.LoadupButton.IndicateFocus = false;
            this.LoadupButton.Location = new System.Drawing.Point(131, 19);
            this.LoadupButton.Name = "LoadupButton";
            this.LoadupButton.OnDisabledState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(191)))), ((int)(((byte)(191)))), ((int)(((byte)(191)))));
            this.LoadupButton.OnDisabledState.BorderRadius = 1;
            this.LoadupButton.OnDisabledState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.LoadupButton.OnDisabledState.BorderThickness = 1;
            this.LoadupButton.OnDisabledState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(204)))), ((int)(((byte)(204)))), ((int)(((byte)(204)))));
            this.LoadupButton.OnDisabledState.ForeColor = System.Drawing.Color.FromArgb(((int)(((byte)(168)))), ((int)(((byte)(160)))), ((int)(((byte)(168)))));
            this.LoadupButton.OnDisabledState.IconLeftImage = null;
            this.LoadupButton.OnDisabledState.IconRightImage = null;
            this.LoadupButton.onHoverState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.LoadupButton.onHoverState.BorderRadius = 1;
            this.LoadupButton.onHoverState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.LoadupButton.onHoverState.BorderThickness = 1;
            this.LoadupButton.onHoverState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(105)))), ((int)(((byte)(181)))), ((int)(((byte)(255)))));
            this.LoadupButton.onHoverState.ForeColor = System.Drawing.Color.White;
            this.LoadupButton.onHoverState.IconLeftImage = null;
            this.LoadupButton.onHoverState.IconRightImage = null;
            this.LoadupButton.OnIdleState.BorderColor = System.Drawing.Color.DodgerBlue;
            this.LoadupButton.OnIdleState.BorderRadius = 1;
            this.LoadupButton.OnIdleState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.LoadupButton.OnIdleState.BorderThickness = 1;
            this.LoadupButton.OnIdleState.FillColor = System.Drawing.Color.DodgerBlue;
            this.LoadupButton.OnIdleState.ForeColor = System.Drawing.Color.White;
            this.LoadupButton.OnIdleState.IconLeftImage = null;
            this.LoadupButton.OnIdleState.IconRightImage = null;
            this.LoadupButton.OnPressedState.BorderColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(96)))), ((int)(((byte)(144)))));
            this.LoadupButton.OnPressedState.BorderRadius = 1;
            this.LoadupButton.OnPressedState.BorderStyle = Bunifu.UI.WinForms.BunifuButton.BunifuButton.BorderStyles.Solid;
            this.LoadupButton.OnPressedState.BorderThickness = 1;
            this.LoadupButton.OnPressedState.FillColor = System.Drawing.Color.FromArgb(((int)(((byte)(40)))), ((int)(((byte)(96)))), ((int)(((byte)(144)))));
            this.LoadupButton.OnPressedState.ForeColor = System.Drawing.Color.White;
            this.LoadupButton.OnPressedState.IconLeftImage = null;
            this.LoadupButton.OnPressedState.IconRightImage = null;
            this.LoadupButton.Size = new System.Drawing.Size(80, 39);
            this.LoadupButton.TabIndex = 0;
            this.LoadupButton.TextAlign = System.Drawing.ContentAlignment.MiddleCenter;
            this.LoadupButton.TextAlignment = System.Windows.Forms.HorizontalAlignment.Center;
            this.LoadupButton.TextMarginLeft = 0;
            this.LoadupButton.TextPadding = new System.Windows.Forms.Padding(0);
            this.LoadupButton.UseDefaultRadiusAndThickness = true;
            this.LoadupButton.Click += new System.EventHandler(this.LoadupButton_Click);
            // 
            // bunifuDragControl1
            // 
            this.bunifuDragControl1.Fixed = true;
            this.bunifuDragControl1.Horizontal = true;
            this.bunifuDragControl1.TargetControl = this.TopPanel;
            this.bunifuDragControl1.Vertical = true;
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(234, 100);
            this.Controls.Add(this.bottomPanel);
            this.Controls.Add(this.TopPanel);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
            this.Name = "Form1";
            this.Text = "Form1";
            this.TopPanel.ResumeLayout(false);
            this.TopPanel.PerformLayout();
            this.bottomPanel.ResumeLayout(false);
            this.bottomPanel.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private Bunifu.UI.WinForms.BunifuPanel TopPanel;
        private Bunifu.UI.WinForms.BunifuPanel bottomPanel;
        private Bunifu.UI.WinForms.BunifuButton.BunifuButton LoadupButton;
        private Bunifu.UI.WinForms.BunifuCheckBox DisableVacToggle;
        private Bunifu.UI.WinForms.BunifuLabel bunifuLabel1;
        private Bunifu.UI.WinForms.BunifuButton.BunifuButton ExitButton;
        private Bunifu.Framework.UI.BunifuDragControl bunifuDragControl1;
        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label Title;
    }
}


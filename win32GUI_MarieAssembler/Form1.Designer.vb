<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class Form1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.AssembleButton = New System.Windows.Forms.Button()
        Me.GroupBox2 = New System.Windows.Forms.GroupBox()
        Me.InputFileButton = New System.Windows.Forms.Button()
        Me.InputFileBox = New System.Windows.Forms.TextBox()
        Me.GroupBox1 = New System.Windows.Forms.GroupBox()
        Me.LogisimBox = New System.Windows.Forms.TextBox()
        Me.LogisimCheck = New System.Windows.Forms.CheckBox()
        Me.LogisimButton = New System.Windows.Forms.Button()
        Me.RawCheck = New System.Windows.Forms.CheckBox()
        Me.SymbolCheck = New System.Windows.Forms.CheckBox()
        Me.RawButton = New System.Windows.Forms.Button()
        Me.SymbolButton = New System.Windows.Forms.Button()
        Me.RawBox = New System.Windows.Forms.TextBox()
        Me.SymbolBox = New System.Windows.Forms.TextBox()
        Me.ListingCheck = New System.Windows.Forms.CheckBox()
        Me.ListingButton = New System.Windows.Forms.Button()
        Me.ListingBox = New System.Windows.Forms.TextBox()
        Me.AssemblerOutputBox = New System.Windows.Forms.RichTextBox()
        Me.GroupBox3 = New System.Windows.Forms.GroupBox()
        Me.StatusStrip1 = New System.Windows.Forms.StatusStrip()
        Me.HelpLabel = New System.Windows.Forms.Label()
        Me.GroupBox2.SuspendLayout()
        Me.GroupBox1.SuspendLayout()
        Me.GroupBox3.SuspendLayout()
        Me.SuspendLayout()
        '
        'AssembleButton
        '
        Me.AssembleButton.Location = New System.Drawing.Point(78, 333)
        Me.AssembleButton.Name = "AssembleButton"
        Me.AssembleButton.Size = New System.Drawing.Size(75, 25)
        Me.AssembleButton.TabIndex = 1
        Me.AssembleButton.Text = "Assemble"
        Me.AssembleButton.UseVisualStyleBackColor = True
        '
        'GroupBox2
        '
        Me.GroupBox2.AutoSize = True
        Me.GroupBox2.Controls.Add(Me.InputFileButton)
        Me.GroupBox2.Controls.Add(Me.InputFileBox)
        Me.GroupBox2.Location = New System.Drawing.Point(12, 5)
        Me.GroupBox2.Name = "GroupBox2"
        Me.GroupBox2.Size = New System.Drawing.Size(228, 64)
        Me.GroupBox2.TabIndex = 3
        Me.GroupBox2.TabStop = False
        Me.GroupBox2.Text = "Input File"
        '
        'InputFileButton
        '
        Me.InputFileButton.Location = New System.Drawing.Point(197, 19)
        Me.InputFileButton.Name = "InputFileButton"
        Me.InputFileButton.Size = New System.Drawing.Size(25, 23)
        Me.InputFileButton.TabIndex = 0
        Me.InputFileButton.Text = "..."
        Me.InputFileButton.UseVisualStyleBackColor = True
        '
        'InputFileBox
        '
        Me.InputFileBox.Dock = System.Windows.Forms.DockStyle.Left
        Me.InputFileBox.Location = New System.Drawing.Point(3, 19)
        Me.InputFileBox.Name = "InputFileBox"
        Me.InputFileBox.Size = New System.Drawing.Size(188, 23)
        Me.InputFileBox.TabIndex = 1
        '
        'GroupBox1
        '
        Me.GroupBox1.Controls.Add(Me.LogisimBox)
        Me.GroupBox1.Controls.Add(Me.LogisimCheck)
        Me.GroupBox1.Controls.Add(Me.LogisimButton)
        Me.GroupBox1.Controls.Add(Me.RawCheck)
        Me.GroupBox1.Controls.Add(Me.SymbolCheck)
        Me.GroupBox1.Controls.Add(Me.RawButton)
        Me.GroupBox1.Controls.Add(Me.SymbolButton)
        Me.GroupBox1.Controls.Add(Me.RawBox)
        Me.GroupBox1.Controls.Add(Me.SymbolBox)
        Me.GroupBox1.Controls.Add(Me.ListingCheck)
        Me.GroupBox1.Controls.Add(Me.ListingButton)
        Me.GroupBox1.Controls.Add(Me.ListingBox)
        Me.GroupBox1.Location = New System.Drawing.Point(12, 75)
        Me.GroupBox1.Name = "GroupBox1"
        Me.GroupBox1.Size = New System.Drawing.Size(228, 252)
        Me.GroupBox1.TabIndex = 2
        Me.GroupBox1.TabStop = False
        Me.GroupBox1.Text = "Output Files"
        '
        'LogisimBox
        '
        Me.LogisimBox.Enabled = False
        Me.LogisimBox.Location = New System.Drawing.Point(3, 41)
        Me.LogisimBox.Name = "LogisimBox"
        Me.LogisimBox.Size = New System.Drawing.Size(188, 23)
        Me.LogisimBox.TabIndex = 3
        '
        'LogisimCheck
        '
        Me.LogisimCheck.AutoSize = True
        Me.LogisimCheck.Location = New System.Drawing.Point(9, 22)
        Me.LogisimCheck.Name = "LogisimCheck"
        Me.LogisimCheck.Size = New System.Drawing.Size(142, 19)
        Me.LogisimCheck.TabIndex = 0
        Me.LogisimCheck.Text = "Enable Logisim Image"
        Me.LogisimCheck.UseVisualStyleBackColor = True
        '
        'LogisimButton
        '
        Me.LogisimButton.Enabled = False
        Me.LogisimButton.Location = New System.Drawing.Point(197, 41)
        Me.LogisimButton.Name = "LogisimButton"
        Me.LogisimButton.Size = New System.Drawing.Size(25, 23)
        Me.LogisimButton.TabIndex = 2
        Me.LogisimButton.Text = "..."
        Me.LogisimButton.UseVisualStyleBackColor = True
        '
        'RawCheck
        '
        Me.RawCheck.AutoSize = True
        Me.RawCheck.Location = New System.Drawing.Point(7, 183)
        Me.RawCheck.Name = "RawCheck"
        Me.RawCheck.Size = New System.Drawing.Size(110, 19)
        Me.RawCheck.TabIndex = 10
        Me.RawCheck.Text = "Enable Raw Hex"
        Me.RawCheck.UseVisualStyleBackColor = True
        '
        'SymbolCheck
        '
        Me.SymbolCheck.AutoSize = True
        Me.SymbolCheck.Location = New System.Drawing.Point(7, 130)
        Me.SymbolCheck.Name = "SymbolCheck"
        Me.SymbolCheck.Size = New System.Drawing.Size(134, 19)
        Me.SymbolCheck.TabIndex = 5
        Me.SymbolCheck.Text = "Enable Symbol Table"
        Me.SymbolCheck.UseVisualStyleBackColor = True
        '
        'RawButton
        '
        Me.RawButton.Enabled = False
        Me.RawButton.Location = New System.Drawing.Point(197, 208)
        Me.RawButton.Name = "RawButton"
        Me.RawButton.Size = New System.Drawing.Size(25, 23)
        Me.RawButton.TabIndex = 6
        Me.RawButton.Text = "..."
        Me.RawButton.UseVisualStyleBackColor = True
        '
        'SymbolButton
        '
        Me.SymbolButton.Enabled = False
        Me.SymbolButton.Location = New System.Drawing.Point(197, 155)
        Me.SymbolButton.Name = "SymbolButton"
        Me.SymbolButton.Size = New System.Drawing.Size(25, 23)
        Me.SymbolButton.TabIndex = 7
        Me.SymbolButton.Text = "..."
        Me.SymbolButton.UseVisualStyleBackColor = True
        '
        'RawBox
        '
        Me.RawBox.Enabled = False
        Me.RawBox.Location = New System.Drawing.Point(7, 208)
        Me.RawBox.Name = "RawBox"
        Me.RawBox.Size = New System.Drawing.Size(184, 23)
        Me.RawBox.TabIndex = 8
        '
        'SymbolBox
        '
        Me.SymbolBox.Enabled = False
        Me.SymbolBox.Location = New System.Drawing.Point(7, 155)
        Me.SymbolBox.Name = "SymbolBox"
        Me.SymbolBox.Size = New System.Drawing.Size(184, 23)
        Me.SymbolBox.TabIndex = 9
        '
        'ListingCheck
        '
        Me.ListingCheck.AutoSize = True
        Me.ListingCheck.Location = New System.Drawing.Point(7, 76)
        Me.ListingCheck.Name = "ListingCheck"
        Me.ListingCheck.Size = New System.Drawing.Size(99, 19)
        Me.ListingCheck.TabIndex = 4
        Me.ListingCheck.Text = "Enable Listing"
        Me.ListingCheck.UseVisualStyleBackColor = True
        '
        'ListingButton
        '
        Me.ListingButton.Enabled = False
        Me.ListingButton.Location = New System.Drawing.Point(197, 101)
        Me.ListingButton.Name = "ListingButton"
        Me.ListingButton.Size = New System.Drawing.Size(25, 23)
        Me.ListingButton.TabIndex = 2
        Me.ListingButton.Text = "..."
        Me.ListingButton.UseVisualStyleBackColor = True
        '
        'ListingBox
        '
        Me.ListingBox.Enabled = False
        Me.ListingBox.Location = New System.Drawing.Point(6, 101)
        Me.ListingBox.Name = "ListingBox"
        Me.ListingBox.Size = New System.Drawing.Size(185, 23)
        Me.ListingBox.TabIndex = 3
        '
        'AssemblerOutputBox
        '
        Me.AssemblerOutputBox.Dock = System.Windows.Forms.DockStyle.Fill
        Me.AssemblerOutputBox.Location = New System.Drawing.Point(3, 19)
        Me.AssemblerOutputBox.Name = "AssemblerOutputBox"
        Me.AssemblerOutputBox.ReadOnly = True
        Me.AssemblerOutputBox.Size = New System.Drawing.Size(438, 331)
        Me.AssemblerOutputBox.TabIndex = 4
        Me.AssemblerOutputBox.Text = ""
        '
        'GroupBox3
        '
        Me.GroupBox3.Controls.Add(Me.AssemblerOutputBox)
        Me.GroupBox3.Location = New System.Drawing.Point(246, 5)
        Me.GroupBox3.Name = "GroupBox3"
        Me.GroupBox3.Size = New System.Drawing.Size(444, 353)
        Me.GroupBox3.TabIndex = 5
        Me.GroupBox3.TabStop = False
        Me.GroupBox3.Text = "Assembler Output"
        '
        'StatusStrip1
        '
        Me.StatusStrip1.Location = New System.Drawing.Point(0, 365)
        Me.StatusStrip1.Name = "StatusStrip1"
        Me.StatusStrip1.Size = New System.Drawing.Size(702, 22)
        Me.StatusStrip1.TabIndex = 6
        Me.StatusStrip1.Text = "StatusStrip1"
        '
        'HelpLabel
        '
        Me.HelpLabel.Anchor = System.Windows.Forms.AnchorStyles.None
        Me.HelpLabel.AutoSize = True
        Me.HelpLabel.Location = New System.Drawing.Point(8, 371)
        Me.HelpLabel.Name = "HelpLabel"
        Me.HelpLabel.Size = New System.Drawing.Size(0, 15)
        Me.HelpLabel.TabIndex = 7
        Me.HelpLabel.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'Form1
        '
        Me.AllowDrop = True
        Me.AutoScaleDimensions = New System.Drawing.SizeF(7.0!, 15.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(702, 387)
        Me.Controls.Add(Me.AssembleButton)
        Me.Controls.Add(Me.GroupBox1)
        Me.Controls.Add(Me.GroupBox2)
        Me.Controls.Add(Me.GroupBox3)
        Me.Controls.Add(Me.HelpLabel)
        Me.Controls.Add(Me.StatusStrip1)
        Me.Name = "Form1"
        Me.Text = "Marie Assembler GUI"
        Me.GroupBox2.ResumeLayout(False)
        Me.GroupBox2.PerformLayout()
        Me.GroupBox1.ResumeLayout(False)
        Me.GroupBox1.PerformLayout()
        Me.GroupBox3.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents GroupBox2 As GroupBox
    Friend WithEvents InputFileButton As Button
    Friend WithEvents InputFileBox As TextBox
    Friend WithEvents GroupBox1 As GroupBox
    Friend WithEvents RawCheck As CheckBox
    Friend WithEvents SymbolCheck As CheckBox
    Friend WithEvents RawButton As Button
    Friend WithEvents SymbolButton As Button
    Friend WithEvents RawBox As TextBox
    Friend WithEvents SymbolBox As TextBox
    Friend WithEvents ListingCheck As CheckBox
    Friend WithEvents ListingButton As Button
    Friend WithEvents LogisimButton As Button
    Friend WithEvents ListingBox As TextBox
    Friend WithEvents LogisimBox As TextBox
    Friend WithEvents LogisimCheck As CheckBox
    Friend WithEvents AssembleButton As Button
    Friend WithEvents AssemblerOutputBox As RichTextBox
    Friend WithEvents GroupBox3 As GroupBox
    Friend WithEvents StatusStrip1 As StatusStrip
    Friend WithEvents HelpLabel As Label
End Class

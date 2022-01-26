Imports System.IO

Public Class Form1

    Declare Unicode Function EXT_Assemble Lib "DynamicMarieAssembler.dll" Alias "VisualBasicEntryPoint" (InputPath As Char(), LogisimPath As Char(), RawPath As Char(), SymbolPath As Char(), ListingPath As Char()) As Int32

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
    End Sub

    Private Sub PickFileOpen(ByVal Title As String, ByRef Box As TextBox)
        Dim Dialog As OpenFileDialog = New OpenFileDialog()
        Dialog.FileName = Box.Text
        Dialog.Title = Title
        If Dialog.ShowDialog() = DialogResult.OK Then
            Box.Text = Dialog.FileName
        End If
    End Sub

    Private Sub PickFileSave(ByVal Title As String, ByVal Filter As String, ByRef Box As TextBox)
        Dim Dialog As SaveFileDialog = New SaveFileDialog()
        Dialog.FileName = Box.Text
        Dialog.Title = Title
        Dialog.Filter = Filter
        If Dialog.ShowDialog() = DialogResult.OK Then
            Box.Text = Dialog.FileName
        End If
    End Sub

    Private Sub UpdateOutputControls(ByVal NewVal As Boolean, ByRef Box As TextBox, ByRef Butt As Button)
        Box.Enabled = NewVal
        Butt.Enabled = NewVal
    End Sub



    Private Sub LogisimCheck_CheckedChanged(sender As Object, e As EventArgs) Handles LogisimCheck.CheckedChanged
        UpdateOutputControls(sender.Checked, LogisimBox, LogisimButton)
    End Sub

    Private Sub ListingCheck_CheckedChanged(sender As Object, e As EventArgs) Handles ListingCheck.CheckedChanged
        UpdateOutputControls(sender.Checked, ListingBox, ListingButton)
    End Sub

    Private Sub SymbolCheck_CheckedChanged(sender As Object, e As EventArgs) Handles SymbolCheck.CheckedChanged
        UpdateOutputControls(sender.Checked, SymbolBox, SymbolButton)
    End Sub

    Private Sub RawCheck_CheckedChanged(sender As Object, e As EventArgs) Handles RawCheck.CheckedChanged
        UpdateOutputControls(sender.Checked, RawBox, RawButton)
    End Sub
    ' ---------------

    Private Sub InputFileButton_Click(sender As Object, e As EventArgs) Handles InputFileButton.Click
        PickFileOpen("What file shall I assemble?", InputFileBox)
        LogisimBox.Text = Path.ChangeExtension(InputFileBox.Text, ".LogisimImage")
        ListingBox.Text = Path.ChangeExtension(InputFileBox.Text, ".lst")
        SymbolBox.Text = Path.ChangeExtension(InputFileBox.Text, ".sym")
        RawBox.Text = Path.ChangeExtension(InputFileBox.Text, ".hex")
    End Sub

    Private Sub LogisimButton_Click(sender As Object, e As EventArgs) Handles LogisimButton.Click
        PickFileSave("Where shall the Logisim Image go?", "Logism Image (*.LogisimImage)|", LogisimBox)
    End Sub

    Private Sub ListingButton_Click(sender As Object, e As EventArgs) Handles ListingButton.Click
        PickFileSave("Where shall the Listing File go?", "Assembly Listing (*.lst)", ListingBox)
    End Sub

    Private Sub SymbolButton_Click(sender As Object, e As EventArgs) Handles SymbolButton.Click
        PickFileSave("Where shall the Symbol Table go?", "Symbol Table (*.sym)", SymbolBox)
    End Sub

    Private Sub RawButton_Click(sender As Object, e As EventArgs) Handles RawButton.Click
        PickFileSave("Where shall the Raw Hex File go?", "Raw Hex (*.hex)", RawBox)
    End Sub

    Private Sub AssembleButton_Click(sender As Object, e As EventArgs) Handles AssembleButton.Click
        Dim LogisimPath As Char()
        LogisimPath = ""

        Dim RawPath As Char()
        RawPath = ""

        Dim SymbolPath As Char()
        SymbolPath = ""

        Dim ListingPath As Char()
        ListingPath = ""

        If LogisimCheck.Checked Then
            LogisimPath = LogisimBox.Text
        End If

        If RawCheck.Checked Then
            RawPath = RawBox.Text
        End If

        If SymbolCheck.Checked Then
            SymbolPath = SymbolBox.Text
        End If

        If ListingCheck.Checked Then
            ListingPath = ListingBox.Text
        End If

        Dim Result As Integer = EXT_Assemble(InputFileBox.Text, LogisimPath, RawPath, SymbolPath, ListingPath)
        AssemblerOutputBox.Text = My.Computer.FileSystem.ReadAllText(".\stdout.txt")
        If AssemblerOutputBox.Text = "" Then AssemblerOutputBox.Text = "Assembled successfully."
    End Sub

    Private Sub AssembleButton_MouseEnter(sender As Object, e As EventArgs) Handles AssembleButton.MouseEnter
        HelpLabel.Text = "Assembles the input file and produces the enabled output files."
    End Sub
    Private Sub AssembleButton_MouseLeave(sender As Object, e As EventArgs) Handles AssembleButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub InputFileBox_MouseEnter(sender As Object, e As EventArgs) Handles InputFileBox.MouseEnter
        HelpLabel.Text = "The path to the file that will be assembled."
    End Sub

    Private Sub InputFileBox_MouseLeave(sender As Object, e As EventArgs) Handles InputFileBox.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub InputFileButton_MouseEnter(sender As Object, e As EventArgs) Handles InputFileButton.MouseEnter
        HelpLabel.Text = "Click to open a File Picker."
    End Sub

    Private Sub InputFileButton_MouseLeave(sender As Object, e As EventArgs) Handles InputFileButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub ListingBox_MouseEnter(sender As Object, e As EventArgs) Handles ListingBox.MouseEnter
        HelpLabel.Text = "The path where the Listing File will be output to."
    End Sub

    Private Sub ListingBox_MouseLeave(sender As Object, e As EventArgs) Handles ListingBox.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub ListingButton_MouseEnter(sender As Object, e As EventArgs) Handles ListingButton.MouseEnter
        HelpLabel.Text = "Click to open a File Picker."
    End Sub

    Private Sub ListingButton_MouseLeave(sender As Object, e As EventArgs) Handles ListingButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub ListingCheck_MouseEnter(sender As Object, e As EventArgs) Handles ListingCheck.MouseEnter
        HelpLabel.Text = "When checked, the Listing File will be output to the path below."
    End Sub

    Private Sub ListingCheck_MouseLeave(sender As Object, e As EventArgs) Handles ListingCheck.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub LogisimBox_MouseEnter(sender As Object, e As EventArgs) Handles LogisimBox.MouseEnter
        HelpLabel.Text = "The path where the Logisim Image will be output to."
    End Sub

    Private Sub LogisimBox_MouseLeave(sender As Object, e As EventArgs) Handles LogisimBox.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub LogisimButton_MouseEnter(sender As Object, e As EventArgs) Handles LogisimButton.MouseEnter
        HelpLabel.Text = "Click to open a File Picker."
    End Sub

    Private Sub LogisimButton_MouseLeave(sender As Object, e As EventArgs) Handles LogisimButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub LogisimCheck_MouseEnter(sender As Object, e As EventArgs) Handles LogisimCheck.MouseEnter
        HelpLabel.Text = "When checked, the Logisim Image will be output to the path below."
    End Sub

    Private Sub LogisimCheck_MouseLeave(sender As Object, e As EventArgs) Handles LogisimCheck.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub RawBox_MouseEnter(sender As Object, e As EventArgs) Handles RawBox.MouseEnter
        HelpLabel.Text = "The path where the Raw Hex file will be output to."
    End Sub

    Private Sub RawBox_MouseLeave(sender As Object, e As EventArgs) Handles RawBox.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub RawButton_MouseEnter(sender As Object, e As EventArgs) Handles RawButton.MouseEnter
        HelpLabel.Text = "Click to open a File Picker"
    End Sub

    Private Sub RawButton_MouseLeave(sender As Object, e As EventArgs) Handles RawButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub RawCheck_MouseEnter(sender As Object, e As EventArgs) Handles RawCheck.MouseEnter
        HelpLabel.Text = "When checked, the Raw Hex file will be output to the path below."
    End Sub

    Private Sub RawCheck_MouseLeave(sender As Object, e As EventArgs) Handles RawCheck.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub SymbolBox_MouseEnter(sender As Object, e As EventArgs) Handles SymbolBox.MouseEnter
        HelpLabel.Text = "The path where the Symbol Table will be output to."
    End Sub

    Private Sub SymbolBox_MouseLeave(sender As Object, e As EventArgs) Handles SymbolBox.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub SymbolButton_MouseEnter(sender As Object, e As EventArgs) Handles SymbolButton.MouseEnter
        HelpLabel.Text = "Click to open a File Picker."
    End Sub

    Private Sub SymbolButton_MouseLeave(sender As Object, e As EventArgs) Handles SymbolButton.MouseLeave
        HelpLabel.Text = ""
    End Sub

    Private Sub SymbolCheck_MouseEnter(sender As Object, e As EventArgs) Handles SymbolCheck.MouseEnter
        HelpLabel.Text = "When checked, the Symbol Table will be output to the path below."
    End Sub

End Class

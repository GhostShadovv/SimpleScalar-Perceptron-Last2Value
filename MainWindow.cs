using System;
using System.IO;
using Gtk;
using Mono.Unix;
using InterfataPerceptron;
using System.Timers;

public partial class MainWindow : Window
{
    private double VhtSizeLast;
    private string MainFolder;
    private ITerminalCommand _terminalCommand;
    public string SimpleSimPath { get; set; }
    public int Perceptrons => (int)spbPerceptrons.Value;
    public int MaxInstr => (int)spbMaxInstr.Value;
    public int Threshold => (int)spbThreshold.Value;
    public int VhtDim => (int)spbVhtSize.Value;
    public int History => (int)spbHistorySize.Value;
    public string Benchmark => cbBenchmarks.ActiveText;
    public DataManagement DataManaged { get; set; }
    public Fixed Fixed => fixed1;
    private IChart _chart;
    private IFileManager _fileManager;
    private Timer timer = new Timer(2000);


    public MainWindow() : base(WindowType.Toplevel)
    {
        Build();
        _terminalCommand = new TerminalCommand(this);
        _chart = new Chart(this);
        _fileManager = new FileManager(this);
        MainFolder = Directory.GetCurrentDirectory();
        SimpleSimPath = UnixPath.Combine(MainFolder, "simplesim-3.0");
        string[] Benchmarks = Directory.GetFiles(UnixPath.Combine(SimpleSimPath, "benchmarks"));
        foreach (var item in Benchmarks)
        {
            if (item.EndsWith(".ss", StringComparison.Ordinal))
            {
                cbBenchmarks.AppendText(UnixPath.GetFileName(item));
            }
        }
        timer.Elapsed += TimerElapsed;
        timer.Enabled = true;
        WidthRequest = 600;
        HeightRequest = 800;

    }

    private void TimerElapsed(object sender, ElapsedEventArgs e)
    {
        lStatus.Text = "Waiting";
    }

    internal void SetProcInfo(string value)
    {
        lInfo.Text = value;
    }
    internal void SetStatus(string value)
    {
        lStatus.Text = value;
        timer.Start();
    }

    protected void OnDeleteEvent(object sender, DeleteEventArgs a)
    {
        Application.Quit();
        a.RetVal = true;
    }
       
    protected void VhtSizeValueChanged(object sender, EventArgs e)
    {
        double temp;
        if (spbVhtSize.Value.Equals(VhtSizeLast))
        {
            return;
        }
        if (spbVhtSize.Value > VhtSizeLast)
        {
            temp = ((spbVhtSize.Value - 1) * 2);
        }
        else
        {
            temp = ((spbVhtSize.Value + 1) / 2);
        }
        spbVhtSize.ValueChanged -= VhtSizeValueChanged;
        spbVhtSize.Value = temp;
        spbVhtSize.ValueChanged += VhtSizeValueChanged;
        VhtSizeLast = spbVhtSize.Value;
        spbVhtSize.Text = temp.ToString();
    }

    protected void BtnSimulateClick(object sender, EventArgs e)
    {
        System.Threading.Thread t = new System.Threading.Thread(new System.Threading.ThreadStart(SetStatusExecute)); 
        _terminalCommand.AssambleCommand();
        _terminalCommand.ExecuteCommand();
        _fileManager.GetData();
        _chart.Init();
        _chart.Draw();
    }

    private void SetStatusExecute()
    {
        lStatus.Text = "Executing";
    }
}

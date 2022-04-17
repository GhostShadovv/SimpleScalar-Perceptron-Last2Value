using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading.Tasks;

namespace InterfataPerceptron
{
    public class TerminalCommand : ITerminalCommand
    {
        private Dictionary<string, string> benchmarksDictionary = new Dictionary<string, string>();

        private readonly MainWindow _mainWindow;
        private string simulator;
        private string redir;
        private string maxInstr;
        private string threshold;
        private string percep;
        private string history;
        private string vht;
        private  string benchmarkss;



        public TerminalCommand(MainWindow mainWindow)
        {
            _mainWindow = mainWindow;

            benchmarksDictionary.Add("benchmarks/applu.ss", "< benchmarks/applu.in");
            benchmarksDictionary.Add("benchmarks/apsi.ss", "< benchmarks/apsi.in");
            benchmarksDictionary.Add("benchmarks/hydro2d.ss", "< benchmarks/hydro2d.in");
            benchmarksDictionary.Add("benchmarks/go.ss", "< benchmarks/9stone21.in");
            benchmarksDictionary.Add("benchmarks/su2cor.ss", "< benchmarks/su2cor.in");
            benchmarksDictionary.Add("benchmarks/swim.ss", "< benchmarks/swim2.in");
            benchmarksDictionary.Add("benchmarks/tomcatv.ss", "< benchmarks/tomcatv.in");
            benchmarksDictionary.Add("benchmarks/cc1.ss", "< benchmarks/1stmt.i");
            benchmarksDictionary.Add("benchmarks/li.ss", "benchmarks/*.lsp {uses benchmarks/au.lsp, " +
                "benchmarks/deriv.lsp, benchmarks/destrum0.lsp, benchmarks/tak0.lsp, " +
                "benchmarks/xit.lsp, benchmarks/boyer.lsp, benchmarks/destru0.lsp, " +
                "benchmarks/browse.lsp, benchmarks/div2.lsp, benchmarks/ctak.lsp, " +
                "benchmarks/puzzle0.lsp, benchmarks/takr.lsp, benchmarks/dderiv.lsp, " +
                "benchmarks/triang.lsp}");
        }

            public void AssambleCommand()
        {
            simulator = "./sim-perceptron";
            redir = "results/dataout.res";
            maxInstr = _mainWindow.MaxInstr.ToString();
            threshold = _mainWindow.Threshold.ToString();
            percep = _mainWindow.Perceptrons.ToString();
            history = _mainWindow.History.ToString();
            vht = _mainWindow.VhtDim.ToString();
            benchmarkss = "benchmarks/" + _mainWindow.Benchmark;
        }

        public void ExecuteCommand()
        {
            var task = Task.Factory.StartNew(() => { 
                string command = $"{simulator} -redir:sim {redir} -max:inst {maxInstr} -threshold {threshold} -percep {percep} -history {history} -vht {vht} {benchmarkss} {benchmarksDictionary[benchmarkss]} && exit";
                var proc = new Process
                {
                    StartInfo = new ProcessStartInfo
                    {
                        FileName = "/bin/bash",
                        WorkingDirectory = _mainWindow.SimpleSimPath,
                        Arguments = "-c \"" + command + "\"",
                        UseShellExecute = false,
                        RedirectStandardOutput = true,
                        RedirectStandardError = true,
                        RedirectStandardInput = true,
                        CreateNoWindow = true
                        
                    }
                };
                proc.Start();
                proc.WaitForExit();
                _mainWindow.SetProcInfo(proc.StandardError.ReadToEnd());
                _mainWindow.SetStatus("Done");
            });
            task.Wait();
        }
    }
}

using System;
using System.IO;
using System.Linq;

namespace InterfataPerceptron
{
    public class FileManager : IFileManager
    {
        private readonly DataManagement data;
        private readonly MainWindow _mainWindow;
        public FileManager(MainWindow mainWindow)
        {
            data = new DataManagement();
            _mainWindow = mainWindow;
        }

        public void GetData()
        {
            var values = new[] { "Incarcare VHT Table", "missVht", "nrWrong", "nrCorrect",
                "nrCorrectThresh", "sim_num_loads", "sim_elapsed_time"};
            using (StreamReader st = new StreamReader("simplesim-3.0/results/dataout.res"))
            {
                while (!st.EndOfStream)
                {
                    string line = st.ReadLine();
                    if (values.Any(line.Contains))
                    {
                        var arguments = line.Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries).Take(2).ToArray();
                        switch (arguments[0])
                        {
                            case "missVht":
                                data.MissVht = int.Parse(arguments[1]);
                                break;
                            case "sim_elapsed_time":
                                data.ElapsedTime = int.Parse(arguments[1]);
                                break;
                            case "nrWrong":
                                data.NrWrong = int.Parse(arguments[1]);
                                break;
                            case "nrCorrect":
                                data.NrCorrect = int.Parse(arguments[1]);
                                break;
                            case "nrCorrectThresh":
                                data.NrCorrectThresh = int.Parse(arguments[1]);
                                break;
                            case "sim_num_loads":
                                data.NumLoads = int.Parse(arguments[1]);
                                break;
                            case "Incarcare":
                                {
                                    string load = line.Split(new string[] { " " }, StringSplitOptions.RemoveEmptyEntries).Last();
                                    if (double.TryParse(load, out double vhtLoad))
                                    {
                                        data.LoadVHT = vhtLoad;
                                    }
                                    break;
                                }
                        }
                    }

                }
            }
            _mainWindow.DataManaged = data;

        }
    }
}

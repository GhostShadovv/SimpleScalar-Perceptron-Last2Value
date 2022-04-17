using System;
using Gtk;
using OxyPlot;
using OxyPlot.Axes;
using OxyPlot.GtkSharp;
using OxyPlot.Series;
using System.Collections.Generic;
using System.Linq;

namespace InterfataPerceptron
{
    public class Chart : IChart
    {
        private readonly MainWindow _mainWindow;
        private PlotView plotView;
        private PlotView plotView2;

        private PlotModel _plotModel;
        private PlotModel _plotModel2;

        private BarSeries seriesAccuracy;
        private BarSeries seriesAccuracyConf;
        private BarSeries seriesLoadCap;
        private BarSeries seriesWrong;
        private BarSeries seriesCorrect;
        private BarSeries seriesCorrectThresh;

        public Chart(MainWindow mainWindow)
        {
            _mainWindow = mainWindow;
        }

        public void Draw()
        {
            double Accuracy = _mainWindow.DataManaged.NrCorrect / (double)_mainWindow.DataManaged.NumLoads * 100;
            double AccuracyConf = _mainWindow.DataManaged.NrCorrectThresh / (double)_mainWindow.DataManaged.NumLoads * 100;
            double LoadCap = _mainWindow.DataManaged.LoadVHT;

            List<double> tmpList = new List<double>
            {
                Accuracy,
                AccuracyConf,
                LoadCap
            };


            int maxValue = (int)tmpList.Max() + 2;
            maxValue = maxValue > 100 ? 100 : maxValue;
            _plotModel.Axes.Add(new LinearAxis { Maximum = maxValue, Position = AxisPosition.Bottom, AxislineStyle = LineStyle.None, Key = "Value" });

            int nrWrong = _mainWindow.DataManaged.NrWrong;
            int nrCorrect = _mainWindow.DataManaged.NrCorrect;
            int nrCorrectThresh = _mainWindow.DataManaged.NrCorrectThresh;
            tmpList = new List<double>
            {
                nrWrong,
                nrCorrect,
                nrCorrectThresh
            };
            maxValue = (int)tmpList.Max() + 2;
            _plotModel2.Axes.Add(new LinearAxis { Maximum = maxValue, Position = AxisPosition.Bottom, AxislineStyle = LineStyle.None, Key = "Value" });

            seriesAccuracy = SetSeries( OxyColors.Aqua, "Accuracy");
            seriesAccuracyConf = SetSeries( OxyColors.Lime, "Accuracy with threshold");
            seriesLoadCap = SetSeries( OxyColors.Orange, "VHT Load");
            seriesWrong = SetSeries(OxyColors.Red, "nrWrong");
            seriesCorrect = SetSeries(OxyColors.Green, "nrCorrect");
            seriesCorrectThresh = SetSeries(OxyColors.Turquoise, "nrCorrectThreshold");



            seriesAccuracy.Items.Add(new BarItem { Value = Accuracy });
            seriesAccuracyConf.Items.Add(new BarItem { Value = AccuracyConf });
            seriesLoadCap.Items.Add(new BarItem { Value = LoadCap });         

            seriesWrong.Items.Add(new BarItem { Value = nrWrong });
            seriesCorrect.Items.Add(new BarItem { Value = nrCorrect });
            seriesCorrectThresh.Items.Add(new BarItem { Value = nrCorrectThresh });


            _plotModel.Series.Add(seriesAccuracy);
            _plotModel.Series.Add(seriesAccuracyConf);
            _plotModel.Series.Add(seriesLoadCap);
            _plotModel2.Series.Add(seriesWrong);
            _plotModel2.Series.Add(seriesCorrect);
            _plotModel2.Series.Add(seriesCorrectThresh);
            plotView.Model = _plotModel;
            plotView2.Model = _plotModel2;
        }

        private BarSeries SetSeries(OxyColor fillColor, string title)
        {
             return new BarSeries()
            {
                LabelFormatString = "{0:0.##}",
                LabelPlacement = LabelPlacement.Inside,
                Title = title,
                StrokeColor = OxyColors.Black,
                StrokeThickness = 1,
                FillColor = fillColor,
                XAxisKey = "Value",
                YAxisKey = "Category"
            };
        }

        public void Init()
        {
            plotView = new PlotView();
            plotView2 = new PlotView();
            plotView.SetSizeRequest(300, 400);
            plotView2.SetSizeRequest(300, 400);
            Box box = new VBox(false, 0);
            Box box2 = new HBox(false, 0);
            box2.PackStart(plotView, true, false, 0);
            box2.PackStart(plotView2, true, false, 0);
            box.PackStart(box2, true, false, 400);
            _mainWindow.Fixed.Add(box);
            box.Add(box2);
            plotView.Show();
            plotView2.Show();
            box.Show();
            box2.Show();

            SetChart();
        }

        private void SetChart()
        {
            _plotModel = new PlotModel();
            _plotModel2 = new PlotModel();

            CategoryAxis c = new CategoryAxis { Position = AxisPosition.Left, Key = "Category" };
            c.Labels.Add(_mainWindow.Benchmark);
            _plotModel.Axes.Add(c);

            CategoryAxis c2 = new CategoryAxis { Position = AxisPosition.Left, Key = "Category" };
            c2.Labels.Add(_mainWindow.Benchmark);
            _plotModel2.Axes.Add(c2);
        }
    }
}

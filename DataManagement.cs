using System;
using System.IO;

namespace InterfataPerceptron
{
    public class DataManagement
    {
        /*
    missVht                        5118 # number of misses in vht
    nrWrong                        2819 # total number of wrong predictions
    nrCorrect                      4054 # total number of correct predictions
    nrCorrectThresh                2206 # total number of correct predictions but the output

    sim_num_loads                 11991 # total number of loads executed

    sim_elapsed_time                  1 # total simulation time in seconds

    Incarcare VHT Table 64/512 = 12.500000 */
        private int missVht;
        private int nrWrong;
        private int nrCorrect;
        private int nrCorrectThresh;
        private int numLoads;
        private int elapsedTime;
        private double loadVHT;

        public int MissVht { get => missVht; set => missVht = value; }
        public int NrWrong { get => nrWrong; set => nrWrong = value; }
        public int NrCorrect { get => nrCorrect; set => nrCorrect = value; }
        public int NrCorrectThresh { get => nrCorrectThresh; set => nrCorrectThresh = value; }
        public int NumLoads { get => numLoads; set => numLoads = value; }
        public int ElapsedTime { get => elapsedTime; set => elapsedTime = value; }
        public double LoadVHT { get => loadVHT; set => loadVHT = value; }

        public DataManagement()
        {
        }


    }
}

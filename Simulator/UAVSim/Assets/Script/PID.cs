using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Assets.Script
{
    class PID
    {
        private double Kp, Ki, Kd;
        private double I;
        private double lastMeasurement;

        private double limMin, limMax, offset;

        public PID(double Kp, double Ki, double Kd)
        {
            this.Kp = Kp; this.Ki = Ki; this.Kd = Kd;

            offset = 0;
            limMin = -10000;
            limMax = 10000;

        }

        public void setLimits(double limMin, double limMax, double offset)
        {
            this.limMin = limMin; this.limMax = limMax; this.offset = offset;
        }

        public double update(double setpoint, double measurement, double dt)
        {

            double error = setpoint - measurement;

            double P  = Kp * error;
                   I += Ki * error * dt;
            double D  = Kd * (measurement - lastMeasurement) / dt;

            if (I > limMax)
                I = limMax;
            else if (I < limMin)
                I = limMin;

            double controller_out = offset + P + I - D;

            if (controller_out > limMax)
                controller_out = limMax;
            else if (controller_out < limMin)
                controller_out = limMin;

            lastMeasurement = measurement;

            return controller_out;

        }

    }
}
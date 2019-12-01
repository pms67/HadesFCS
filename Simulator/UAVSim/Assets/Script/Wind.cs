using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

namespace Assets.Script
{
    class Wind
    {
        public double Lu, Lv, Lw;
        public double sigma_u, sigma_v, sigma_w;

        private double[] u_w, v_w, w_w;
        private double[] wind_u, wind_v, wind_w;

        private System.Random randGen;

        public Wind()
        {
            randGen = new System.Random();

            u_w = new double[] { 0, 0 };
            v_w = new double[] { 0, 0, 0 };
            w_w = new double[] { 0, 0, 0 };

            wind_u = new double[] { 0, 0 };
            wind_v = new double[] { 0, 0, 0 };
            wind_w = new double[] { 0, 0, 0 };

        }

        public double[] getWind(double[] wind_field, double[] attitude, double altitude, double airspeed, double turbulence, double dt)
        {

            double[] totalWind = getTurbulence(altitude, airspeed, turbulence, dt);

            double sph = System.Math.Sin(attitude[0]); double sth = System.Math.Sin(attitude[1]); double sps = System.Math.Sin(attitude[2]);
            double cph = System.Math.Cos(attitude[0]); double cth = System.Math.Cos(attitude[1]); double cps = System.Math.Cos(attitude[2]);

            totalWind[0] += (cth * cps) * wind_field[0] + (cth * sps) * wind_field[1] - sth * wind_field[2];
            totalWind[1] += (sph * sth * cps - cph * sps) * wind_field[0] + (sph * sth * sps + cph * cps) * wind_field[1] + (sph * cth) * wind_field[2];
            totalWind[2] += (cph * sth * cps + sph * sps) * wind_field[0] + (cph * sth * sps - sth * cps) * wind_field[1] + (cph * cth) * wind_field[2];

            return totalWind;

        }

        private double[] getTurbulence(double altitude, double airspeed, double turbulence, double dt)
        {

            u_w[1] = u_w[0]; v_w[2] = v_w[1]; v_w[1] = v_w[0]; w_w[2] = w_w[1]; w_w[1] = w_w[0];
            wind_u[1] = wind_u[0]; wind_v[2] = wind_v[1]; wind_v[1] = wind_v[0]; wind_w[2] = wind_w[1]; wind_w[1] = wind_w[0];

            // Dryden gust model parameters
            setParameters(altitude, turbulence);

            // Generate random noise
            u_w[0] = randGaussian(0, 1);
            v_w[0] = randGaussian(0, 1);
            w_w[0] = randGaussian(0, 1);

            // Filter noise
            double a1 = sigma_u * System.Math.Sqrt(2 * airspeed / Lu);
            double a2 = airspeed / Lu;
            wind_u[0] = (dt * a1 * (u_w[0] + u_w[1]) - (dt * a2 - 2) * wind_u[1]) / (2 + dt * a2);

            double b1 = sigma_v * System.Math.Sqrt(3 * airspeed / Lv);
            double b2 = airspeed / (System.Math.Sqrt(3) * Lv);
            double b3 = airspeed / Lv;

            double Av = 4 + sq(b3 * dt) + 4 * b3 * dt;
            double Bv = 2 * sq(b3 * dt) - 8;
            double Cv = 4 + sq(b3 * dt) - 4 * b3 * dt;
            double Dv = b1 * b2 * sq(dt) + 2 * b1 * dt;
            double Ev = 2 * b1 * b2 * sq(dt);
            double Fv = b1 * b2 * sq(dt) - 2 * b1 * dt;

            wind_v[0] = (Dv * v_w[0] + Ev * v_w[1] + Fv * v_w[2] - Bv * wind_v[1] - Cv * wind_v[2]) / Av;

            double c1 = sigma_w * System.Math.Sqrt(3 * airspeed / Lw);
            double c2 = airspeed / (System.Math.Sqrt(3) * Lw);
            double c3 = airspeed / Lw;

            double Aw = 4 + sq(c3 * dt) + 4 * c3 * dt;
            double Bw = 2 * sq(c3 * dt) - 8;
            double Cw = 4 + sq(c3 * dt) - 4 * c3 * dt;
            double Dw = c1 * c2 * sq(dt) + 2 * c1 * dt;
            double Ew = 2 * c1 * c2 * sq(dt);
            double Fw = c1 * c2 * sq(dt) - 2 * c1 * dt;

            wind_w[0] = (Dw * w_w[0] + Ew * w_w[1] + Fw * w_w[2] - Bw * wind_w[1] - Cw * wind_w[2]) / Aw;

            return new double[] { wind_u[0], wind_v[0], wind_w[0] };

        }

        private void setParameters(double altitude, double turbulence)
        {
            Lu = 0.6055 * altitude + 169.7;
            Lv = Lu;
            Lw = 0.8782 * altitude + 6.091;

            sigma_u = (0.0008 * altitude + 1.02) * (1 + turbulence);
            sigma_v = sigma_u;
            sigma_w = (0.001455 * altitude + 0.6273) * (1 + turbulence);
        }

        private double randGaussian(double mean, double stdDev)
        {
            double u1 = 1 - randGen.NextDouble();
            double u2 = 1 - randGen.NextDouble();

            double randStdNormal = System.Math.Sqrt(-2 * System.Math.Log(u1)) * System.Math.Sin(2 * System.Math.PI * u2);

            return mean + stdDev * randStdNormal;
        }

        private double sq(double val) { return val * val; }

    }
}
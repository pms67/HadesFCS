using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using UnityEngine;

namespace Assets.Script
{
    class Aircraft
    {

        // Aircraft properties
        private double mass = 6.9;

        private double Jx = 0.5476;
        private double Jy = 0.9746;
        private double Jz = 1.458;
        private double Jxz = 0.10;

        private double I;
        private double I1;
        private double I2;
        private double I3;
        private double I4;
        private double I5;
        private double I6;
        private double I7;
        private double I8;

        private double S = 0.553;
        private double b = 2.35;
        private double c = 0.261;
        private double e = 0.79;

        private double C_L_0 = 0.4752;
        private double C_L_alpha = 5.848;
        private double C_L_q = 8.757;
        private double C_L_delta_e = -0.0101;

        private double C_D_0 = 0.0201;
        private double C_D_alpha = 0;
        private double C_D_q = 0;
        private double C_D_delta_e = 0.000079;

        private double C_m_0 = -0.067;    
        private double C_m_alpha = -0.8206;
        private double C_m_q = -16.82;
        private double C_m_delta_e = -0.32; //-0.03202;

        private double M = 50;
        private double alpha_0 = 0.4712;
        private double epsilon = 0.1592;
        private double C_D_p = 0;
        
        private double C_Y_0 = 0;
        private double C_Y_beta = -0.1822;
        private double C_Y_p = 0;
        private double C_Y_r = 0;
        private double C_Y_delta_a = 0;
        private double C_Y_delta_r = -0.002047;

        private double C_l_0 = 0;
        private double C_l_beta = -0.02355;
        private double C_l_p = -0.5653;
        private double C_l_r = 0.1273;
        private double C_l_delta_a = 0.06651; //0.006651;
        private double C_l_delta_r = 0.00015;

        private double C_n_0 = 0;
        private double C_n_beta = 0.0694;
        private double C_n_p = 0.0317;        
        private double C_n_r = -0.06665;
        private double C_n_delta_a = 0.000078;
        private double C_n_delta_r = -0.000929;

        // Propulsion
        private double S_prop = 0.2027;
        private double k_motor = 20;
        private double k_T_p = 0;
        private double k_omega = 0;
        private double C_prop = 1.0;

        // Environment
        public double g = 9.81;
        public double rho = 1.225;

        // State
        public double pn, pe, pd; // Inertial north, east, down position
        public double u, v, w; // Body velocity
        public double phi, theta, psi; // Euler angles (roll, pitch, heading)
        public double p, q, r; // Body roll rate

        // Quaternions
        public double e0, e1, e2, e3;
        private static double lambda = 10; // Gradient descent gain

        // Variables
        public double Va; // Airspeed
        public double alpha; // Angle of attack (RADIANS)
        public double beta; // Side-slip angle (RADIANS)

        // Forces and moments (body frame)
        private double Fx, Fy, Fz;
        private double l, m, n;

        // Control inputs
        public double delta_t, delta_a, delta_e, delta_r;

        public Aircraft()
        {
            // Reset states
            resetState();

            // Calculate inertia terms
            I = Jx * Jz - Jxz * Jxz;
            I1 = Jxz * (Jx - Jy + Jz) / I;
            I2 = (Jz * (Jz - Jy) + Jxz * Jxz) / I;
            I3 = Jz / I;
            I4 = Jxz / I;
            I5 = (Jz - Jx) / Jy;
            I6 = Jxz / Jy;
            I7 = ((Jx - Jy) * Jx + Jxz * Jxz) / I;
            I8 = Jx / I;
            
        }

        public void initState(double[] pos, double[] vel, double[] att)
        {

            pn = pos[0]; pe = pos[1]; pd = pos[2];
            u = vel[0]; v = vel[2]; w = vel[2]; 
            phi = att[0]; theta = att[1]; psi = att[2];

            // Set quaternions
            e0 = cos(psi / 2) * cos(theta / 2) * cos(phi / 2) + sin(psi / 2) * sin(theta / 2) * sin(phi / 2);
            e1 = cos(psi / 2) * cos(theta / 2) * sin(phi / 2) - sin(psi / 2) * sin(theta / 2) * cos(phi / 2);
            e2 = cos(psi / 2) * sin(theta / 2) * cos(phi / 2) + sin(psi / 2) * cos(theta / 2) * sin(phi / 2);
            e3 = sin(psi / 2) * cos(theta / 2) * cos(phi / 2) - cos(psi / 2) * sin(theta / 2) * sin(phi / 2);

        }

        public void resetState()
        {

            pn = pe = pd = u = v = w = phi = theta = psi = p = q = r = 0;
            e0 = 1;
            e1 = e2 = e3 = 0;

        }

        public void setControls(double[] ctr)
        {

            delta_t = ctr[0];
            delta_a = ctr[1];
            delta_e = ctr[2];
            delta_r = ctr[3];

        }

        public void checkHeight()
        {
            if (pd < 0)
            {
                pd = 0;
                w = 0;
            }
        }

        public void update(double dt, double[] wind_vec)
        {

            // Update model
            integrateRKQAT(dt, wind_vec);

            // Check height
            checkHeight();

            // Get airspeed, angle of attack, and sideslip angle
            double[] abva = AlphaBetaAirspeed(u - wind_vec[0], v - wind_vec[1], w - wind_vec[2]);
            alpha = abva[0]; beta = abva[1]; Va = abva[2];

            // Convert quaternions to Euler angles
            phi = System.Math.Atan2(2 * (e0 * e1 + e2 * e3), sq(e0) + sq(e3) - sq(e1) - sq(e2));
            double tmp = 2 * (e0 * e2 - e1 * e3);
            if (tmp >= 1)
            {
                theta = System.Math.Asin(1);
            } else if (tmp <= -1)
            {
                theta = System.Math.Asin(-1);
            } else
            {
                theta = System.Math.Asin(tmp);
            }
            psi = System.Math.Atan2(2 * (e0 * e3 + e1 * e2), sq(e0) + sq(e1) - sq(e2) - sq(e3));

        }

        /*
         * 
         * 
         * HELPER FUNCTIONS
         * 
         * 
         */

        public double sin(double val) { return System.Math.Sin(val); }
        public double cos(double val) { return System.Math.Cos(val); }
        public double tan(double val) { return System.Math.Tan(val); }
        public double exp(double val) { return System.Math.Exp(val); }
        public double sign(double val) { return System.Math.Sign(val); }
        public double sq(double val) { return (val * val); }

        private double[] AlphaBetaAirspeed(double u_, double v_, double w_)
        {

            // Airspeed
            double Va_ = System.Math.Sqrt(u_ * u_ + v_ * v_ + w_ * w_);

            // Angle of attack
            double alpha_ = 0;
            if (u_ > 0)
                alpha_ = System.Math.Atan2(w_, u_);

            // Side-slip Angle
            double beta_ = 0;
            if (Va_ > 0)
                beta_ = System.Math.Asin(v_ / Va_);

            return new double[] { alpha_, beta_, Va_ };

        }

        /*
         * 
         * 
         * FORCES AND MOMENTS 
         * 
         * 
         */

        private double CL(double alpha_)
        {

            double sigmoid = (1 + exp(-M * (alpha_ - alpha_0)) + exp(M * (alpha_ + alpha_0))) /
                             ((1 + exp(-M * (alpha_ - alpha_0))) * (1 + exp(M * (alpha_ + alpha_0))));

            double a_ = (1 - sigmoid) * (C_L_0 + C_L_alpha * alpha_);
            double sa = sin(alpha_);
            double b_ = sigmoid * (2 * sign(alpha_) * sa * sa * cos(alpha_));

            if (alpha_ >= 0)
                return a_ + b_;
            else
                return a_ - b_;
            
        }

        private double CD(double alpha_)
        {

            double AR = (b * b) / S; // Aspect ratio 

            double a_ = (C_L_0 + C_L_alpha * alpha_) * (C_L_0 + C_L_alpha * alpha_);
            double b_ = System.Math.PI * e * AR;

            return (C_D_p + (a_ / b_));

        }

        private double[] getForcesQAT(double Va_, double alpha_, double beta_, double e0_, double e1_, double e2_, double e3_, double p_, double q_, double r_, double[] ctr_inputs) // Body frame
        {

            double DELTA_t = ctr_inputs[0]; double DELTA_a = ctr_inputs[1]; double DELTA_e = ctr_inputs[2]; double DELTA_r = ctr_inputs[3];

            // Dynamic pressure * span
            double fact = 0.5 * rho * Va_ * Va_ * S;
            double fact2 = 0.25 * rho * Va_ * S;

            // Gravity		
            double F_grav_X = mass * g * 2 * (e1_ * e3_ - e2_ * e0_);
            double F_grav_Y = mass * g * 2 * (e2_ * e3_ + e1_ * e0_);
            double F_grav_Z = mass * g * (sq(e3_) + sq(e0_) - sq(e1_) - sq(e2_));

            // Aerodynamic
            double CDa = CD(alpha_);
            double CLa = CL(alpha_);

            double sa = sin(alpha_);
            double ca = cos(alpha_);

            double CX_alpha = -CDa * ca + CLa * sa;
            double CX_q = -C_D_q * ca + C_L_q * sa;
            double CX_delta_e = -C_D_delta_e * ca + C_L_delta_e * sa;

            double CZ_alpha = -CDa * sa - CLa * ca;
            double CZ_q = -C_D_q * sa - C_L_q * ca;
            double CZ_delta_e = -C_D_delta_e * sa - C_L_delta_e * ca;

            double F_aero_X = fact * CX_alpha + fact2 * c * q_ * CX_q + fact * DELTA_e * CX_delta_e;
            double F_aero_Y = fact * C_Y_0 + fact * C_Y_beta * beta_ + fact2 * b * p_ * C_Y_p
                           + fact2 * b * r_ * C_Y_r + fact * C_Y_delta_a * DELTA_a + fact * C_Y_delta_r * DELTA_r;
            double F_aero_Z = fact * CZ_alpha + fact2 * c * q_ * CZ_q + fact * DELTA_e * CZ_delta_e;

            // Propulsion
            double F_prop_X = 0.5 * rho * S_prop * C_prop * ((k_motor * DELTA_t) * (k_motor * DELTA_t) - Va_ * Va_);


            return new double[] { F_grav_X + F_aero_X + F_prop_X, F_grav_Y + F_aero_Y, F_grav_Z + F_aero_Z };

        }

        private double[] getMoments(double Va_, double alpha_, double beta_, double p_, double q_, double r_, double[] ctr_inputs)
        {

            double DELTA_t = ctr_inputs[0]; double DELTA_a = ctr_inputs[1]; double DELTA_e = ctr_inputs[2]; double DELTA_r = ctr_inputs[3];

            // Dynamic pressure * span
            double fact = 0.5 * rho * Va_ * Va_ * S;
            double fact2 = 0.25 * rho * Va_ * S;

            double M_aero_l = fact * b * C_l_0 + fact * b * C_l_beta * beta_ + fact2 * b * b * p_ * C_l_p
                           + fact2 * b * b * r_ * C_l_r + fact * b * C_l_delta_a * DELTA_a + fact * b * C_l_delta_r * DELTA_r;
            double M_aero_m = fact * c * C_m_0 + fact * c * C_m_alpha * alpha_ + fact2 * c * c * q_ * C_m_q + fact * c * C_m_delta_e * DELTA_e;
            double M_aero_n = fact * b * C_n_0 + fact * b * C_n_beta * beta_ + fact2 * b * b * p_ * C_n_p + fact2 * b * b * r_ * C_n_r + fact * b * C_n_delta_a * DELTA_a + fact * b * C_n_delta_r * DELTA_r;

            double M_prop_l = -k_T_p * (k_omega * DELTA_t) * (k_omega * DELTA_t);

            return new double[] { M_aero_l + M_prop_l, M_aero_m, M_aero_n };

        }


        /*
         *
         *
         * DYNAMICS 
         * 
         * 
         */

        double[] dynamicsQAT(double[] vec, double[] ctr_inputs, double[] wind_vec)
        {

            // Expand state vector
            double u_ = vec[0]; double v_ = vec[1]; double w_ = vec[2];
            double e0_ = vec[3]; double e1_ = vec[4]; double e2_ = vec[5]; double e3_ = vec[6];
            double p_ = vec[7]; double q_ = vec[8]; double r_ = vec[9];

            // Calculate alpha, beta, and airspeed
            double[] abVa = AlphaBetaAirspeed(u_ - wind_vec[0], v_ - wind_vec[1], w_ - wind_vec[2]);
            double alpha_ = abVa[0]; double beta_ = abVa[1]; double Va_ = abVa[2];

            // Calculate forces and moments
            double[] forces = getForcesQAT(Va_, alpha_, beta_, e0_, e1_, e2_, e3_, p_, q_, r_, ctr_inputs);
            double Fx_ = forces[0]; double Fy_ = forces[1]; double Fz_ = forces[2];

            double[] moments = getMoments(Va_, alpha_, beta_, p_, q_, r_, ctr_inputs);
            double l_ = moments[0]; double m_ = moments[1]; double n_ = moments[2];

            // State derivatives (Chapter 3, p.36) (Quaternions Appendix B, p. 256)
            double dpn_dt = (sq(e1_) + sq(e0_) - sq(e2_) - sq(e3_)) * u_ + 2 * (e1_ * e2_ - e3_ * e0_) * v_ + 2 * (e1_ * e3_ + e2_ * e0_) * w_;
            double dpe_dt = 2 * (e1_ * e2_ + e3_ * e0_) * u_ + (sq(e2_) + sq(e0_) - sq(e1_) - sq(e3_)) * v_ + 2 * (e2_ * e3_ - e1_ * e0_) * w_;
            double dpd_dt = -2 * (e1_ * e3_ - e2_ * e0_) * u_ - 2 * (e2_ * e3_ + e1_ * e0_) * v_ - (sq(e3_) + sq(e0_) - sq(e1_) - sq(e2_)) * w_;

            double du_dt = r_ * v_ - q_ * w_ + Fx_ / mass;
            double dv_dt = p_ * w_ - r_ * u_ + Fy_ / mass;
            double dw_dt = q_ * u_ - p_ * v_ + Fz_ / mass;

            double _e_ = sq(e0_) + sq(e1_) + sq(e2_) + sq(e3_);

            double de0_dt = 0.5 * (lambda * (1 - _e_) * e0_ - p_ * e1_ - q_ * e2_ - r_ * e3_);
            double de1_dt = 0.5 * (p_ * e0_ + lambda * (1 - _e_) * e1_ + r_ * e2_ - q_ * e3_);
            double de2_dt = 0.5 * (q_ * e0_ - r_ * e1_ + lambda * (1 - _e_) * e2_ + p_ * e3_);
            double de3_dt = 0.5 * (r_ * e0_ + q_ * e1_ - p_ * e2_ + lambda * (1 - _e_) * e3_);

            double dp_dt = (I1 * p_ * q_ - I2 * q_ * r_) + (I3 * l_ + I4 * n_);
            double dq_dt = (I5 * p_ * r_ - I6 * (p_ * p_ - r_ * r_)) + (m_ / Jy);
            double dr_dt = (I7 * p_ * q_ - I1 * q_ * r_) + (I4 * l_ + I8 * n_);

            return new double[] {dpn_dt, dpe_dt, dpd_dt,
                 du_dt, dv_dt, dw_dt,
                 de0_dt, de1_dt, de2_dt, de3_dt,
                 dp_dt, dq_dt, dr_dt};

        }

        /*
         * 
         * 
         * INTEGRATION
         * 
         * 
         */

        void integrateRKQAT(double h, double[] wind_vec)
        {

            double[] inputs = new double[] { delta_t, delta_a, delta_e, delta_r };

            double h2 = h / 2.0;
            double h6 = h / 6.0;

            double[] a_ = dynamicsQAT(new double[] {u, v, w,
                                                e0, e1, e2, e3,
                                                p, q, r}, inputs, wind_vec);

            double[] b_ = dynamicsQAT(new double[] {u  + h2 * a_[3],  v  + h2 * a_[4],  w  + h2 * a_[5],
                                                e0 + h2 * a_[6],  e1 + h2 * a_[7],  e2 + h2 * a_[8], e3 + h2 * a_[9],
                                                p  + h2 * a_[10], q  + h2 * a_[11], r  + h2 * a_[12]}, inputs, wind_vec);

            double[] c_ = dynamicsQAT(new double[] {u  + h2 * b_[3],  v  + h2 * b_[4],  w  + h2 * b_[5],
                                                e0 + h2 * b_[6],  e1 + h2 * b_[7],  e2 + h2 * b_[8], e3 + h2 * b_[9],
                                                p  + h2 * b_[10], q  + h2 * b_[11], r  + h2 * b_[12]}, inputs, wind_vec);

            double[] d_ = dynamicsQAT(new double[] {u  + h * c_[3],  v  + h * c_[4],  w  + h * c_[5],
                                                e0 + h * c_[6],  e1 + h * c_[7],  e2 + h * c_[8], e3 + h * c_[9],
                                                p  + h * c_[10], q  + h * c_[11], r  + h * c_[12]}, inputs, wind_vec);


            // Update state
            pn += h6 * (a_[0] + 2 * b_[0] + 2 * c_[0] + d_[0]);
            pe += h6 * (a_[1] + 2 * b_[1] + 2 * c_[1] + d_[1]);
            pd += h6 * (a_[2] + 2 * b_[2] + 2 * c_[2] + d_[2]);

            u += h6 * (a_[3] + 2 * b_[3] + 2 * c_[3] + d_[3]);
            v += h6 * (a_[4] + 2 * b_[4] + 2 * c_[4] + d_[4]);
            w += h6 * (a_[5] + 2 * b_[5] + 2 * c_[5] + d_[5]);

            e0 += h6 * (a_[6] + 2 * b_[6] + 2 * c_[6] + d_[6]);
            e1 += h6 * (a_[7] + 2 * b_[7] + 2 * c_[7] + d_[7]);
            e2 += h6 * (a_[8] + 2 * b_[8] + 2 * c_[8] + d_[8]);
            e3 += h6 * (a_[9] + 2 * b_[9] + 2 * c_[9] + d_[9]);

            p += h6 * (a_[10] + 2 * b_[10] + 2 * c_[10] + d_[10]);
            q += h6 * (a_[11] + 2 * b_[11] + 2 * c_[11] + d_[11]);
            r += h6 * (a_[12] + 2 * b_[12] + 2 * c_[12] + d_[12]);

        }

        /*
         * 
         * 
         * LINEARISATION 
         * 
         * 
         */

        public void linearise(string filename)
        {

            if (System.IO.File.Exists(filename))
            {
                Debug.Log("File already exists. Overwriting...");
                System.IO.File.Delete(filename);
            }

            System.IO.StreamWriter sw = System.IO.File.CreateText(filename);

            double[,] jA = jacobianA(new double[] { u, v, w,
                                             phi, theta, psi,
                                             p, q, r}, new double[] { delta_t, delta_a, delta_e, delta_r }, 1E-9);
            double[,] jB = jacobianB(new double[] { u, v, w,
                                             phi, theta, psi,
                                             p, q, r}, new double[] { delta_t, delta_a, delta_e, delta_r }, 1E-9);

            sw.WriteLine("Linearised State-Space Model\n");
            sw.WriteLine("States: pn=" + pn + ",pe=" + pe + ",pd=" + pd + ";" +
                                  "u=" + u + ",v=" + v + ",w=" + w + ";" +
                                  "phi=" + phi + ",theta=" + theta + ",psi=" + psi + ";" +
                                  "p=" + p + ",q=" + q + ",r=" + r);
            sw.WriteLine("Inputs: delta_t=" + delta_t + ",delta_a=" + delta_a + ",delta_e=" + delta_e + ",delta_r=" + delta_r);
            sw.WriteLine("\n\nA = [ ...");
            for (int i = 0; i < jA.GetLength(0); i++)
            {
                for (int j = 0; j < jA.GetLength(1); j++)
                {
                    sw.Write(" " + jA[i, j] + " ");
                }
                sw.WriteLine("; ...");
            }

            sw.WriteLine("];\n\n");

            sw.WriteLine("B = [ ...");
            for (int i = 0; i < jB.GetLength(0); i++)
            {
                for (int j = 0; j < jB.GetLength(1); j++)
                {
                    sw.Write(" " + jB[i, j] + " ");
                }
                sw.WriteLine("; ...");
            }

            sw.WriteLine("];");

            sw.WriteLine("C = diag(ones(1, 12));");
            sw.WriteLine("D = 0;");
            sw.WriteLine("sys = ss(A, B, C, D);");
            sw.WriteLine("sys.Name = 'UAV';");
            sw.WriteLine("sys.InputName = {'Thrust' 'Aileron' 'Elevator' 'Rudder'};");
            sw.WriteLine("sys.InputUnit = {'' 'rad' 'rad' 'rad'};");
            sw.WriteLine("sys.StateName = {'pn' 'pe' 'h' 'u' 'v' 'w' 'phi' 'theta' 'psi' 'p' 'q' 'r'};");
            sw.WriteLine("sys.StateUnit = {'m' 'm' 'm' 'm/s' 'm/s' 'm/s' 'rad' 'rad' 'rad' 'rad/s' 'rad/s' 'rad/s'};");
            sw.WriteLine("sys.OutputName = {'pn' 'pe' 'h' 'u' 'v' 'w' 'phi' 'theta' 'psi' 'p' 'q' 'r'};");
            sw.WriteLine("sys.OutputUnit = {'m' 'm' 'm' 'm/s' 'm/s' 'm/s' 'rad' 'rad' 'rad' 'rad/s' 'rad/s' 'rad/s'};");
            sw.WriteLine("\n\n\n");

            // Calculate transfer functions
            double C_p_p = I3 * C_l_p + I4 * C_n_p;
            double C_p_delta_a = I3 * C_l_delta_a + I4 * C_n_delta_a;

            double a_phi1 = -0.5 * rho * sq(Va) * S * sq(b) * C_p_p / (2 * Va);
            double a_phi2 = 0.5 * rho * sq(Va) * S * b * C_p_delta_a;

            double a_beta1 = -rho * Va * S / (2 * mass) * C_Y_beta;
            double a_beta2 = rho * Va * S / (2 * mass) * C_Y_delta_r;

            double a_theta1 = -rho * sq(Va) * c * S / (2 * Jy) * C_m_q * c / (2 * Va);
            double a_theta2 = -rho * sq(Va) * c * S / (2 * Jy) * C_m_alpha;
            double a_theta3 =  rho * sq(Va) * c * S / (2 * Jy) * C_m_delta_e;

            sw.WriteLine("Transfer Functions (control surface deflections in RADIANS)\n\n");
            sw.WriteLine("Aileron to Roll Angle: TF_DELTA_A_PHI = " + a_phi2 + "/(s * (s + " + a_phi1 + "));");
            sw.WriteLine("Rudder to Sideslip Angle: TF_DELTA_R_BETA = " + a_beta2 + "/(s + " + a_beta1 + ");");
            sw.WriteLine("Elevator to Pitch Angle: TF_DELTA_E_THETA = " + a_theta3 + "/(s^2 + " + a_theta1 + "*s + " + a_theta2 + ");");
            sw.WriteLine("Pitch Angle to Altitude: TF_THETA_ALTITUDE = " + Va + "/s;");

            sw.Close();

        }

        private double[,] jacobianA(double[] state, double[] inputs, double delta)
        {

            double[,] jacA = new double[12, 12];

            double[] initial_derivatives = dynamics(state, inputs);

            for (int i = 0; i < state.Length; i++)
            {

                state[i] += delta;
                double[] final_derivatives = dynamics(state, inputs);

                for (int j = 0; j < state.Length; j++)
                    jacA[i, j] = (final_derivatives[j] - initial_derivatives[j]) / delta;

                state[i] -= delta;

            }

            return jacA;

        }

        private double[,] jacobianB(double[] state, double[] inputs, double delta)
        {

            double[,] jacB = new double[12, 4];

            double[] initial_derivatives = dynamics(state, inputs);

            for (int i = 0; i < inputs.Length; i++)
            {

                inputs[i] += delta;
                double[] final_derivatives = dynamics(state, inputs);

                for (int j = 0; j < state.Length; j++)
                    jacB[j, i] = (final_derivatives[j] - initial_derivatives[j]) / delta;

                inputs[i] -= delta;


            }

            return jacB;

        }


        /*
         * 
         *
         * OLD EULER METHODS
         * BUT STILL USED TO CALCULATE JACOBIANS
         * 
         */

        double[] dynamics(double[] vec, double[] ctr_inputs)
        {

            // Expand state vector
            double u_ = vec[0]; double v_ = vec[1]; double w_ = vec[2];
            double phi_ = vec[3]; double theta_ = vec[4]; double psi_ = vec[5];
            double p_ = vec[6]; double q_ = vec[7]; double r_ = vec[8];

            // Calculate alpha, beta, and airspeed
            double[] abVa = AlphaBetaAirspeed(u_, v_, w_);
            double alpha_ = abVa[0]; double beta_ = abVa[1]; double Va_ = abVa[2];

            // Calculate forces and moments
            double[] forces = getForces(Va_, alpha_, beta_, theta_, phi_, p_, q_, r_, ctr_inputs);
            double Fx_ = forces[0]; double Fy_ = forces[1]; double Fz_ = forces[2];

            double[] moments = getMoments(Va_, alpha_, beta_, p_, q_, r_, ctr_inputs);
            double l_ = moments[0]; double m_ = moments[1]; double n_ = moments[2];

            // Re-usable trigonometric functions
            double sphi = sin(phi_); double st = sin(theta_); double spsi = sin(psi_);
            double cphi = cos(phi_); double ct = cos(theta_); double cpsi = cos(psi_);
            double tt = tan(theta_);

            // State derivatives (Chapter 3, p.36)
            double dpn_dt = (ct * cpsi) * u_ + (sphi * st * cpsi - cphi * spsi) * v_ + (cphi * st * cpsi + sphi * spsi) * w_;
            double dpe_dt = (ct * spsi) * u_ + (sphi * st * spsi + cphi * cpsi) * v_ + (cphi * st * spsi - sphi * cpsi) * w_;
            //double dpd_dt = (-st) * u_ + (sphi * ct) * v_ + (cphi * ct) * w_;
            double dpd_dt = -((-st) * u_ + (sphi * ct) * v_ + (cphi * ct) * w_);

            double du_dt = r_ * v_ - q_ * w_ + Fx_ / mass;
            double dv_dt = p_ * w_ - r_ * u_ + Fy_ / mass;
            double dw_dt = q_ * u_ - p_ * v_ + Fz_ / mass;

            double dphi_dt = p_ + (sphi * tt) * q_ + (cphi * tt) * r_;
            double dtheta_dt = cphi * q_ - sphi * r_;
            double dpsi_dt = (sphi / ct) * q_ + (cphi / ct) * r_;

            double dp_dt = (I1 * p_ * q_ - I2 * q_ * r_) + (I3 * l_ + I4 * n_);
            double dq_dt = (I5 * p_ * r_ - I6 * (p_ * p_ - r_ * r_)) + (m_ / Jy);
            double dr_dt = (I7 * p_ * q_ - I1 * q_ * r_) + (I4 * l_ + I8 * n_);

            return new double[] {dpn_dt, dpe_dt, dpd_dt,
                 du_dt, dv_dt, dw_dt,
                 dphi_dt, dtheta_dt, dpsi_dt,
                 dp_dt, dq_dt, dr_dt};

        }

        void integrateRK(double h)
        {

            double[] inputs = new double[] { delta_t, delta_a, delta_e, delta_r };

            double h2 = h / 2.0;
            double h6 = h / 6.0;

            double[] a_ = dynamics(new double[] {u, v, w,
                                             phi, theta, psi,
                                             p, q, r}, inputs);

            double[] b_ = dynamics(new double[] {u   + h2 * a_[3], v + h2 * a_[4],  w   + h2 * a_[5],
                                             phi + h2 * a_[6], theta + h2 * a_[7],  psi + h2 * a_[8],
                                             p   + h2 * a_[9], q     + h2 * a_[10], r   + h2 * a_[11]}, inputs);

            double[] c_ = dynamics(new double[] {u   + h2 * b_[3], v + h2 * b_[4],  w   + h2 * b_[5],
                                             phi + h2 * b_[6], theta + h2 * b_[7],  psi + h2 * b_[8],
                                             p   + h2 * b_[9], q     + h2 * b_[10], r   + h2 * b_[11]}, inputs);

            double[] d_ = dynamics(new double[] {u  + h * c_[3],  v + h * c_[4],  w   + h * c_[5],
                                             phi + h * c_[6], theta + h * c_[7],  psi + h * c_[8],
                                             p   + h * c_[9], q     + h * c_[10], r   + h * c_[11]}, inputs);


            // Update state
            pn += h6 * (a_[0] + 2 * b_[0] + 2 * c_[0] + d_[0]);
            pe += h6 * (a_[1] + 2 * b_[1] + 2 * c_[1] + d_[1]);
            pd += h6 * (a_[2] + 2 * b_[2] + 2 * c_[2] + d_[2]);

            u += h6 * (a_[3] + 2 * b_[3] + 2 * c_[3] + d_[3]);
            v += h6 * (a_[4] + 2 * b_[4] + 2 * c_[4] + d_[4]);
            w += h6 * (a_[5] + 2 * b_[5] + 2 * c_[5] + d_[5]);

            phi += h6 * (a_[6] + 2 * b_[6] + 2 * c_[6] + d_[6]);
            theta += h6 * (a_[7] + 2 * b_[7] + 2 * c_[7] + d_[7]);
            psi += h6 * (a_[8] + 2 * b_[8] + 2 * c_[8] + d_[8]);

            // Limit attitude angles
            double PI = System.Math.PI;

            if (phi > 2 * PI)
                phi -= 2 * PI;
            else if (phi < -2 * PI)
                phi += 2 * PI;

            if (theta > 2 * PI)
                theta -= 2 * PI;
            else if (theta < -2 * PI)
                theta += 2 * PI;

            if (psi > 2 * PI)
                psi -= 2 * PI;
            else if (psi < -2 * PI)
                psi += 2 * PI;

            p += h6 * (a_[9] + 2 * b_[9] + 2 * c_[9] + d_[9]);
            q += h6 * (a_[10] + 2 * b_[10] + 2 * c_[10] + d_[10]);
            r += h6 * (a_[11] + 2 * b_[11] + 2 * c_[11] + d_[11]);

        }

        private double[] getForces(double Va_, double alpha_, double beta_, double theta_, double phi_, double p_, double q_, double r_, double[] ctr_inputs) // Body frame
        {

            double DELTA_t = ctr_inputs[0]; double DELTA_a = ctr_inputs[1]; double DELTA_e = ctr_inputs[2]; double DELTA_r = ctr_inputs[3];

            // Dynamic pressure * span
            double fact = 0.5 * rho * Va_ * Va_ * S;
            double fact2 = 0.25 * rho * Va_ * S;

            // Gravity		
            double F_grav_X = -mass * g * sin(theta_);
            double F_grav_Y = mass * g * cos(theta_) * sin(phi_);
            double F_grav_Z = mass * g * cos(theta_) * cos(phi_);

            // Aerodynamic
            double CDa = CD(alpha_);
            double CLa = CL(alpha_);

            double sa = sin(alpha_);
            double ca = cos(alpha_);

            double CX_alpha = -CDa * ca + CLa * sa;
            double CX_q = -C_D_q * ca + C_L_q * sa;
            double CX_delta_e = -C_D_delta_e * ca + C_L_delta_e * sa;

            double CZ_alpha = -CDa * sa - CLa * ca;
            double CZ_q = -C_D_q * sa - C_L_q * ca;
            double CZ_delta_e = -C_D_delta_e * sa - C_L_delta_e * ca;

            double F_aero_X = fact * CX_alpha + fact2 * c * q_ * CX_q + fact * DELTA_e * CX_delta_e;
            double F_aero_Y = fact * C_Y_0 + fact * C_Y_beta * beta_ + fact2 * b * p_ * C_Y_p
                           + fact2 * b * r_ * C_Y_r + fact * C_Y_delta_a * DELTA_a + fact * C_Y_delta_r * DELTA_r;
            double F_aero_Z = fact * CZ_alpha + fact2 * c * q_ * CZ_q + fact * DELTA_e * CZ_delta_e;

            // Propulsion
            double F_prop_X = 0.5 * rho * S_prop * C_prop * ((k_motor * DELTA_t) * (k_motor * DELTA_t) - Va_ * Va_);

            return new double[] { F_grav_X + F_aero_X + F_prop_X, F_grav_Y + F_aero_Y, F_grav_Z + F_aero_Z };

        }

        /*
         * 
         * TRIM
         * 
         */

        public double[] trim(double alphainit, double betainit, double phiinit, double Va_, double R_, double gamma_)
        {
            resetState();

            // Gradient descent
            int N = 100000;
            double epsilon_ = 1E-6;
            double rate = 1E-5;

            int n = 0;
            double J1 = 1;

            double alpha_ = alphainit; double beta_ = betainit; double phi_ = phiinit;

            while (J1 > 1E-7 && n < N)
            {

                J1 = J(alpha_, beta_, phi_, Va_, R_, gamma_);

                double alpha2_ = alpha_ + epsilon_;
                double J2_alpha = J(alpha2_, beta_, phi_, Va_, R_, gamma_);
                double dJdalpha = (J2_alpha - J1) / epsilon_;

                double beta2_ = beta_ + epsilon_;
                double J2_beta = J(alpha_, beta2_, phi_, Va_, R_, gamma_);
                double dJdbeta = (J2_beta - J1) / epsilon_;

                double phi2_ = phi_ + epsilon_;
                double J2_phi = J(alpha_, beta_, phi2_, Va_, R_, gamma_);
                double dJdphi = (J2_phi - J1) / epsilon_;

                alpha_ -= rate * dJdalpha;
                beta_ -= rate * dJdbeta;
                phi_ -= rate * dJdphi;

                n++;

            }

            // Compute trimmed states and set as current
            Va = Va_;
            alpha = alpha_;
            beta = beta_;
            phi = phi_;
            theta = alpha + gamma_;
            psi = 0;

            Debug.Log(phi * 180.0 / 3.141);

            u = Va_ * cos(alpha) * cos(beta);
            v = Va_ * sin(beta);
            w = Va_ * sin(alpha) * cos(beta);

            p = -(Va_ / R_) * sin(theta);
            q = (Va_ / R_) * sin(phi) * cos(theta);
            r = (Va_ / R_) * cos(phi) * cos(theta);

            // Set trimmed inputs
            delta_e = ((Jxz * (sq(p) - sq(r)) + (Jx - Jz) * p * r)) / (0.5 * rho * sq(Va) * c * S);
            delta_e = delta_e - C_m_0 - C_m_alpha * alpha - C_m_q * c * q / (2 * Va);
            delta_e = delta_e / C_m_delta_e;

            double C_X_alpha = -CD(alpha) * cos(alpha) + CL(alpha) * sin(alpha);
            double C_X_q = -C_D_q * cos(alpha) + C_L_q * sin(alpha);
            double C_X_delta_e = -C_D_delta_e * cos(alpha) + C_L_delta_e * sin(alpha);

            delta_t = 2 * mass * (-r * v + q * w + g * sin(theta)) -
                            rho * sq(Va) * S * (C_X_alpha + C_X_q * c * q / (2 * Va) + C_X_delta_e * delta_e);
            delta_t = delta_t / (rho * S_prop * C_prop * sq(k_motor));
            delta_t = delta_t + sq(Va) / sq(k_motor);
            delta_t = System.Math.Sqrt(delta_t);

            double C_p_0 = I3 * C_l_0 + I4 * C_n_0;
            double C_p_beta = I3 * C_l_beta + I4 * C_n_beta;
            double C_p_p = I3 * C_l_p + I4 * C_n_p;
            double C_p_r = I3 * C_l_r + I4 * C_n_r;

            double f1 = (-I1 * p * q + I2 * q * r) / (0.5 * rho * sq(Va) * S * b) -
                        C_p_0 - C_p_beta * beta - C_p_p * b * p / (2 * Va) - C_p_r * b * r / (2 * Va);

            double C_r_0 = I4 * C_l_0 + I8 * C_n_0;
            double C_r_beta = I4 * C_l_beta + I8 * C_n_beta;
            double C_r_p = I4 * C_l_p + I8 * C_n_p;
            double C_r_r = I4 * C_l_r + I8 * C_n_r;

            double f2 = (-I7 * p * q + I1 * q * r) / (0.5 * rho * sq(Va) * S * b) -
                        C_r_0 - C_r_beta * beta - C_r_p * b * p / (2 * Va) - C_r_r * b * r / (2 * Va);

            double C_p_delta_a = I3 * C_l_delta_a + I4 * C_n_delta_a;
            double C_p_delta_r = I3 * C_l_delta_r + I4 * C_n_delta_r;
            double C_r_delta_a = I4 * C_l_delta_a + I8 * C_n_delta_a;
            double C_r_delta_r = I4 * C_l_delta_r + I8 * C_n_delta_r;

            double det = C_p_delta_a * C_r_delta_r - C_p_delta_r * C_r_delta_a;

            delta_a = (C_r_delta_r * f1 - C_p_delta_r * f2) / det;
            delta_r = (-C_r_delta_a * f1 + C_p_delta_a * f2) / det;

            // Set quaternions
            e0 = cos(psi / 2) * cos(theta / 2) * cos(phi / 2) + sin(psi / 2) * sin(theta / 2) * sin(phi / 2);
            e1 = cos(psi / 2) * cos(theta / 2) * sin(phi / 2) - sin(psi / 2) * sin(theta / 2) * cos(phi / 2);
            e2 = cos(psi / 2) * sin(theta / 2) * cos(phi / 2) + sin(psi / 2) * cos(theta / 2) * sin(phi / 2);
            e3 = sin(psi / 2) * cos(theta / 2) * cos(phi / 2) - cos(psi / 2) * sin(theta / 2) * sin(phi / 2);

            return new double[] { delta_t, delta_a, delta_e, delta_r };

        }

        // Cost function
        private double J(double alpha_, double beta_, double phi_, double Va_, double R_, double gamma_)
        {


            double[] xdot_star = new double[] {0, 0, Va_ * sin(gamma_),
                                               0, 0, 0,
                                               0, 0, (Va_ / R_) * cos(gamma_),
                                               0, 0, 0};

            // Trimmed states
            double u_ = Va_ * cos(alpha_) * cos(beta_);
            double v_ = Va_ * sin(beta_);
            double w_ = Va_ * sin(alpha_) * cos(beta_);
            double theta_ = alpha_ + gamma_;
            double p_ = -(Va_ / R_) * sin(theta_);
            double q_ = (Va_ / R_) * sin(phi_) * cos(theta_);
            double r_ = (Va_ / R_) * cos(phi_) * cos(theta_);

            // Trimmed inputs
            double delta_e_ = ((Jxz * (sq(p_) - sq(r_)) + (Jx - Jz) * p_ * r_)) / (0.5 * rho * sq(Va_) * c * S);
            delta_e_ = delta_e_ - C_m_0 - C_m_alpha * alpha_ - C_m_q * c * q_ / (2 * Va_);
            delta_e_ = delta_e_ / C_m_delta_e;

            double C_X_alpha = -CD(alpha_) * cos(alpha_) + CL(alpha_) * sin(alpha_);
            double C_X_q = -C_D_q * cos(alpha_) + C_L_q * sin(alpha_);
            double C_X_delta_e = -C_D_delta_e * cos(alpha_) + C_L_delta_e * sin(alpha_);

            double delta_t_ = 2 * mass * (-r_ * v_ + q_ * w_ + g * sin(theta_)) -
                            rho * sq(Va_) * S * (C_X_alpha + C_X_q * c * q_ / (2 * Va_) + C_X_delta_e * delta_e_);
            delta_t_ = delta_t_ / (rho * S_prop * C_prop * sq(k_motor));
            delta_t_ = delta_t_ + sq(Va_) / sq(k_motor);
            delta_t_ = System.Math.Sqrt(delta_t_);

            double C_p_0 = I3 * C_l_0 + I4 * C_n_0;
            double C_p_beta = I3 * C_l_beta + I4 * C_n_beta;
            double C_p_p = I3 * C_l_p + I4 * C_n_p;
            double C_p_r = I3 * C_l_r + I4 * C_n_r;

            double f1 = (-I1 * p_ * q_ + I2 * q_ * r_) / (0.5 * rho * sq(Va_) * S * b) -
                        C_p_0 - C_p_beta * beta_ - C_p_p * b * p_ / (2 * Va_) - C_p_r * b * r_ / (2 * Va_);

            double C_r_0 = I4 * C_l_0 + I8 * C_n_0;
            double C_r_beta = I4 * C_l_beta + I8 * C_n_beta;
            double C_r_p = I4 * C_l_p + I8 * C_n_p;
            double C_r_r = I4 * C_l_r + I8 * C_n_r;

            double f2 = (-I7 * p_ * q_ + I1 * q_ * r_) / (0.5 * rho * sq(Va_) * S * b) -
                        C_r_0 - C_r_beta * beta_ - C_r_p * b * p_ / (2 * Va_) - C_r_r * b * r_ / (2 * Va_);

            double C_p_delta_a = I3 * C_l_delta_a + I4 * C_n_delta_a;
            double C_p_delta_r = I3 * C_l_delta_r + I4 * C_n_delta_r;
            double C_r_delta_a = I4 * C_l_delta_a + I8 * C_n_delta_a;
            double C_r_delta_r = I4 * C_l_delta_r + I8 * C_n_delta_r;

            double det = C_p_delta_a * C_r_delta_r - C_p_delta_r * C_r_delta_a;

            double delta_a_ = (C_r_delta_r * f1 - C_p_delta_r * f2) / det;
            double delta_r_ = (-C_r_delta_a * f1 + C_p_delta_a * f2) / det;

            double C_Z = -CD(alpha_) * sin(alpha_) - CL(alpha_) * cos(alpha_);
            double C_Z_q = -C_D_q * sin(alpha_) - C_L_q * cos(alpha_);
            double C_Z_delta_e = -C_D_delta_e * sin(alpha_) - C_L_delta_e * cos(alpha_);

            double[] f = new double[] {0, 0, u_ * sin(theta_) - v_ * sin(phi_) * cos(theta_) - w_ * cos(phi_) * cos(theta_),
                                       r_ * v_ - q_ * w_ - g * sin(theta_) + (rho * sq(Va_) * S / (2 * mass)) * (C_X_alpha + C_X_q * c * q_ / (2 * Va_) + C_X_delta_e * delta_e_) + (rho * S_prop * C_prop / (2 * mass)) * (sq(k_motor * delta_t_) - sq(Va_)),
                                       p_ * w_ - r_ * u_ + g * cos(theta_) * sin(phi_) + (rho * sq(Va_) * S / (2 * mass)) * (C_Y_0 + C_Y_beta * beta_ + C_Y_p * b * p_ / (2 * Va_) + C_Y_r * b * r_ / (2 * Va_) + C_Y_delta_a * delta_a_ + C_Y_delta_r * delta_r_),
                                       q_ * u_ - p_ * v_ + g * cos(theta_) * cos(phi_) + (rho * sq(Va_) * S / (2 * mass)) * (C_Z + C_Z_q * c * q_ / (2 * Va_) + C_Z_delta_e * delta_e_),
                                       p_ + q_ * sin(phi_) * tan(theta_) + r_ * cos(phi_) * tan(theta_),
                                       q_ * cos(phi_) - r_ * sin(phi_),
                                       q_ * sin(phi_) / cos(theta_) + r_ * cos(phi_) / cos(theta_),
                                       I1 * p_ * q_ - I2 * q_ * r_ + 0.5 * rho * sq(Va_) * S * b * (C_p_0 + C_p_beta * beta_ + C_p_p * b * p_ / (2 * Va_) + C_p_r * b * r_ / (2 * Va_) + C_p_delta_a * delta_a_ + C_p_delta_r * delta_r_),
                                       I5 * p_ * r_ - I6 * (sq(p_) - sq(r_)) + (rho * sq(Va_) * S * c / (2 * Jy)) * (C_m_0 + C_m_alpha * alpha_ + C_m_q * c * q_ / (2 * Va_) + C_m_delta_e * delta_e_),
                                       I7 * p_ * q_ - I1 * q_ * r_ + 0.5 * rho * sq(Va_) * S * b * (C_r_0 + C_r_beta * beta_ + C_r_p * b * p_ / (2 * Va_) + C_r_r * b * r_ / (2 * Va_) + C_r_delta_a * delta_a_ + C_r_delta_r * delta_r_)};

            double cost = 0;

            for (int i = 0; i < f.Length; i++)
                cost += sq(xdot_star[i] - f[i]);

            return cost;

        }

        /*
         * 
         * MISC
         * 
         */

        public void printArray(double[,] arr)
        {

            string s = "[";

            for (int i = 0; i < arr.GetLength(0); i++)
            {

                s += "";

                for (int j = 0; j < arr.GetLength(1); j++)
                {
                    s += " " + arr[i, j] + " ";
                }

                s += "; ... \n";

            }

            s += "];";

            Debug.Log(s);


        }


        /*
         * 
         * LOAD AIRCRAFT PARAMETERS FROM FILE 
         * 
         */

        public int loadAircraftParams(string filename)
        {
            // Clear all parameters (in case some are unused in the file)
            clearAircraftParams();

            // Read parameter file
            int nParams = 0;

            try
            {

                System.IO.StreamReader file = new System.IO.StreamReader(filename);
                string line;


                while ((line = file.ReadLine()) != null)
                {
                    line = line.Replace(" ", ""); // Remove white spaces

                    if (line.Contains("mass=")) // Mass
                    {
                        line = line.Replace("mass=", "");
                        mass = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("Jx=")) // Inertia
                    {
                        line = line.Replace("Jx=", "");
                        Jx = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("Jy="))
                    {
                        line = line.Replace("Jy=", "");
                        Jy = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("Jz="))
                    {
                        line = line.Replace("Jz=", "");
                        Jz = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("Jxz="))
                    {
                        line = line.Replace("Jxz=", "");
                        Jxz = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("S=")) // Geometry
                    {
                        line = line.Replace("S=", "");
                        S = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("b="))
                    {
                        line = line.Replace("b=", "");
                        b = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("c="))
                    {
                        line = line.Replace("c=", "");
                        c = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("eff="))
                    {
                        line = line.Replace("eff=", "");
                        e = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_L_0=")) // Longitudinal Aerodynamic Coefficients
                    {
                        line = line.Replace("C_L_0=", "");
                        C_L_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_L_alpha="))
                    {
                        line = line.Replace("C_L_alpha=", "");
                        C_L_alpha = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_L_q="))
                    {
                        line = line.Replace("C_L_q=", "");
                        C_L_q = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_L_delta_e="))
                    {
                        line = line.Replace("C_L_delta_e=", "");
                        C_L_delta_e = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_D_0="))
                    {
                        line = line.Replace("C_D_0=", "");
                        C_D_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_D_alpha="))
                    {
                        line = line.Replace("C_D_alpha=", "");
                        C_D_alpha = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_D_p="))
                    {
                        line = line.Replace("C_D_p=", "");
                        C_D_p = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_D_q="))
                    {
                        line = line.Replace("C_D_q=", "");
                        C_D_q = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_D_delta_e="))
                    {
                        line = line.Replace("C_D_delta_e=", "");
                        C_D_delta_e = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_m_0="))
                    {
                        line = line.Replace("C_m_0=", "");
                        C_m_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_m_alpha="))
                    {
                        line = line.Replace("C_m_alpha=", "");
                        C_m_alpha = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_m_q="))
                    {
                        line = line.Replace("C_m_q=", "");
                        C_m_q = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_m_delta_e="))
                    {
                        line = line.Replace("C_m_delta_e=", "");
                        C_m_delta_e = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("M="))
                    {
                        line = line.Replace("M=", "");
                        M = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("alpha_0="))
                    {
                        line = line.Replace("alpha_0=", "");
                        alpha_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("epsilon="))
                    {
                        line = line.Replace("epsilon=", "");
                        epsilon = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_0=")) // Lateral Aerodynamic Coefficients
                    {
                        line = line.Replace("C_Y_0=", "");
                        C_Y_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_beta="))
                    {
                        line = line.Replace("C_Y_beta=", "");
                        C_Y_beta = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_p="))
                    {
                        line = line.Replace("C_Y_p=", "");
                        C_Y_p = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_r="))
                    {
                        line = line.Replace("C_Y_r=", "");
                        C_Y_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_delta_a="))
                    {
                        line = line.Replace("C_Y_delta_a=", "");
                        C_Y_delta_a = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_Y_delta_r="))
                    {
                        line = line.Replace("C_Y_delta_r=", "");
                        C_Y_delta_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_0="))
                    {
                        line = line.Replace("C_l_0=", "");
                        C_l_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_beta="))
                    {
                        line = line.Replace("C_l_beta=", "");
                        C_l_beta = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_p="))
                    {
                        line = line.Replace("C_l_p=", "");
                        C_l_p = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_r="))
                    {
                        line = line.Replace("C_l_r=", "");
                        C_l_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_delta_a="))
                    {
                        line = line.Replace("C_l_delta_a=", "");
                        C_l_delta_a = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_l_delta_r="))
                    {
                        line = line.Replace("C_l_delta_r=", "");
                        C_l_delta_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_0="))
                    {
                        line = line.Replace("C_n_0=", "");
                        C_n_0 = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_beta="))
                    {
                        line = line.Replace("C_n_beta=", "");
                        C_n_beta = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_p="))
                    {
                        line = line.Replace("C_n_p=", "");
                        C_n_p = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_r="))
                    {
                        line = line.Replace("C_n_r=", "");
                        C_n_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_delta_a="))
                    {
                        line = line.Replace("C_n_delta_a=", "");
                        C_n_delta_a = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_n_delta_r="))
                    {
                        line = line.Replace("C_n_delta_r=", "");
                        C_n_delta_r = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("S_prop=")) // Propulsion
                    {
                        line = line.Replace("S_prop=", "");
                        S_prop = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("k_motor="))
                    {
                        line = line.Replace("k_motor=", "");
                        k_motor = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("k_T_p="))
                    {
                        line = line.Replace("k_T_p=", "");
                        k_T_p = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("k_omega="))
                    {
                        line = line.Replace("k_omega=", "");
                        k_omega = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("C_prop="))
                    {
                        line = line.Replace("C_prop=", "");
                        C_prop = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("g="))
                    {
                        line = line.Replace("g=", "");
                        g = System.Convert.ToDouble(line);
                        nParams++;
                    }
                    else if (line.Contains("rho="))
                    {
                        line = line.Replace("rho=", "");
                        rho = System.Convert.ToDouble(line);
                        nParams++;
                    }

                }

                file.Close();

            } catch (System.Exception e)
            {
                Debug.Log("Parameter Read Error: " + e.ToString());
                return -1;
            }

            // Calculate new inertia terms
            I = Jx * Jz - Jxz * Jxz;
            I1 = Jxz * (Jx - Jy + Jz) / I;
            I2 = (Jz * (Jz - Jy) + Jxz * Jxz) / I;
            I3 = Jz / I;
            I4 = Jxz / I;
            I5 = (Jz - Jx) / Jy;
            I6 = Jxz / Jy;
            I7 = ((Jx - Jy) * Jx + Jxz * Jxz) / I;
            I8 = Jx / I;

            // Reset state
            resetState();

            return nParams;

        }

        private void clearAircraftParams()
        {
                  
            mass = 0;
            Jx = 0; Jy = 0; Jz = 0; Jxz = 0;

            I = 0; I1 = 0; I2 = 0; I3 = 0; I4 = 0; I5 = 0; I6 = 0; I7 = 0; I8 = 0;

            S = 0; b = 0; c = 0; e = 0;

            C_L_0 = 0;
            C_D_0 = 0;
            C_m_0 = 0;
            C_L_alpha = 0;
            C_D_alpha = 0;
            C_m_alpha = 0;
            C_L_q = 0;
            C_D_q = 0;
            C_m_q = 0;
            C_L_delta_e = 0;
            C_D_delta_e = 0;
            C_m_delta_e = 0;
            M = 0;
            alpha_0 = 0;
            epsilon = 0;
            C_D_p = 0;
            C_n_delta_r = 0;

            C_Y_0 = 0;
            C_l_0 = 0;
            C_n_0 = 0;
            C_Y_beta = 0;
            C_l_beta = 0;
            C_n_beta = 0;
            C_Y_p = 0;
            C_l_p = 0;
            C_n_p = 0;
            C_Y_r = 0;
            C_l_r = 0;
            C_n_r = 0;
            C_Y_delta_a = 0;
            C_l_delta_a = 0;
            C_n_delta_a = 0;
            C_Y_delta_r = 0;
            C_l_delta_r = 0;

            S_prop = 0; k_motor = 0; k_T_p = 0; k_omega = 0; C_prop = 0;
            g = 9.81; rho = 1.225;
    }

    }
}
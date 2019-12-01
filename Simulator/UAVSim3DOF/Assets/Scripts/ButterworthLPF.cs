using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ButterworthLPF {

    public float output;

    private float wc;

    private float[] u;
    private float[] y;

    public ButterworthLPF(float fc, float fs)
    {
        /* Pre-warp cut-off frequency */
        wc = 2.0f * fs * Mathf.Tan(Mathf.PI * fc / fs);

        u = new float[3];
        y = new float[3];

        Reset();
    }

    public void Reset()
    {
        for (int n = 0; n < 3; n++)
        {
            u[n] = 0.0f;
            y[n] = 0.0f;
        }

        output = 0.0f;
    }

    public float Update(float val, float T)
    {
        u[2] = u[1];
        u[1] = u[0];
        u[0] = val;

        y[2] = y[1];
        y[1] = y[0];
        y[0] = (wc * wc * T * T) * (u[0] + 2.0f * u[1] + u[2]);
        y[0] = y[0] - (wc * wc * T * T - 8.0f) * y[1] - (4.0f - Mathf.Sqrt(2.0f) * wc + wc * wc * T * T) * y[2];
        y[0] = y[0] / (4.0f + Mathf.Sqrt(2.0f) * wc + wc * wc * T * T);

        output = y[0];

        return output;
    }

}

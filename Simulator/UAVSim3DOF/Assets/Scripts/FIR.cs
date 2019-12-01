using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class FIR {

    public float[] coeff;
    public int N;

    public float output;

    private float[] inputs;

    public FIR(float[] coeff, int N)
    {
        this.N = N;

        this.coeff = new float[N];
        inputs = new float[N];

        for (int n = 0; n < N; n++)
        {
            this.coeff[n] = coeff[n];
            inputs[n] = 0.0f;
        }

        output = 0.0f;
    }

    public float Update(float val)
    {
        int n;

        for (n = 0; n < N - 1; n++)
        {
            inputs[n + 1] = inputs[n];
        }
        inputs[0] = val;

        output = 0.0f;
        for (n = 0; n < N; n++)
        {
            output += coeff[n] * inputs[n];
        }

        return output;
    }
 
}

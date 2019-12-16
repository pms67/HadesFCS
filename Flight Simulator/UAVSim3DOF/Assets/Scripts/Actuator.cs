using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Actuator {

    public float gain, tau;
    public float output;

    private float prevInput, prevOutput;

    public Actuator(float gain, float tau)
    {
        this.gain = gain;
        this.tau = tau;
        prevInput = 0.0f;
        prevOutput = 0.0f;
    }

    public float Update(float input, float T)
    {
        output = (gain * T * (input + prevInput) - prevOutput * (T - 2.0f * tau)) / (2.0f * tau + T);


        prevInput = input;
        prevOutput = output;

        return output;
    }

}

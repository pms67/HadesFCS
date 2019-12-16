using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PIController {

    public float output;
    public float Kp, Ki, Kff, limMin, limMax, limMinFF, limMaxFF;
    private float integrator, prevError;

    public PIController(float Kp, float Ki, float limMin, float limMax)
    {
        this.Kp = Kp;
        this.Ki = Ki;
        this.limMin = limMin;
        this.limMax = limMax;
        limMinFF = 0.0f;
        limMaxFF = 0.0f;
        Init();
    }

    public void Init()
    {
        integrator = 0.0f;
        prevError = 0.0f;
        output = 0.0f;
    }

    public void SetFF(float Kff, float limMinFF, float limMaxFF)
    {
        this.Kff = Kff;
        this.limMinFF = limMinFF;
        this.limMaxFF = limMaxFF;
    }

    public float Update(float setpoint, float measurement, float T)
    {
        float error = setpoint - measurement;

        float proportional = Kp * error;

        float ff = Kff * setpoint;

        if (ff > limMaxFF)
        {
            ff = limMaxFF;
        }
        else if (ff < limMinFF)
        {
            ff = limMinFF;
        }

        integrator = integrator + 0.5f * T * Ki * (error + prevError);

        float limMinInt, limMaxInt;

        if (limMax - proportional - ff > 0.0f)
        {
            limMaxInt = limMax - proportional - ff;
        }
        else
        {
            limMaxInt = 0.0f;
        }

        if (limMin - proportional - ff < 0.0f)
        {
            limMinInt = limMin - proportional - ff;
        }
        else
        {
            limMinInt = 0.0f;
        }

        if (integrator > limMaxInt)
        {
            integrator = limMaxInt;
        }
        else if (integrator < limMinInt)
        {
            integrator = limMinInt;
        }

        output = proportional + ff + integrator;

        if (output > limMax)
        {
            output = limMax;
        }
        else if (output < limMin)
        {
            output = limMin;
        }

        prevError = error;

        return output;
    }

}

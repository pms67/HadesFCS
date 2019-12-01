using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/* Look-up table and linear interpolation for throttle setting vs engine thrust */
public class EngineModel {

    public float[] throttleSetting;
    public float[] thrust;
    public int N;

    public EngineModel(float[] throttleSetting, float[] thrust, int N)
    {
        this.N = N;
        this.throttleSetting = new float[N];
        this.thrust = new float[N];

        for (int n = 0; n < N; n++)
        {
            this.throttleSetting[n] = throttleSetting[n];
            this.thrust[n] = thrust[n];
        }
    }

    public float GetThrust(float throttle)
    {
        if (throttle > 100.0f)
        {
            throttle = 100.0f;
        } else if (throttle < 0.0f)
        {
            throttle = 0.0f;
        }
        
        for (int n = 0; n < N - 1; n++)
        {
            if (throttle >= throttleSetting[n] && throttle <= throttleSetting[n + 1])
            {
                float engineThrust = thrust[n] + throttle * (thrust[n + 1] - thrust[n]) / (throttleSetting[n + 1] - throttleSetting[n]);

                return engineThrust;
            }            
        }

        /* Error: throttle not in range! */
        return 0.0f;
    }

}

using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/* Total energy control system */
public class TECS {
    
    public PIController piAirspeed;
    public PIController piAltitude;

    public float ALTITUDE_SATURATION = 10.0f;

    private float mass;
    private const float g = 9.81f;

    public TECS(float Kpt, float Kit, float Kpp, float Kip, float mass)
    {
        this.mass = mass;
        
        piAirspeed = new PIController(Kpt, Kit, 0.0f, 100.0f);
        piAltitude = new PIController(Kpp, Kip, -30.0f * 180.0f / Mathf.PI, 30.0f * 180.0f / Mathf.PI);
    }

    public float[] Update(float VaSetpoint, float Va, float altitudeSetpoint, float altitude, float T)
    {
        float Kerror = 0.5f * mass * (VaSetpoint * VaSetpoint - Va * Va);
        float Uerror = mass * g * (altitudeSetpoint - altitude);

        if (Uerror > mass * g * ALTITUDE_SATURATION)
        {
            Uerror = mass * g * ALTITUDE_SATURATION;
        } else if (Uerror < -mass * g * ALTITUDE_SATURATION)
        {
            Uerror = -mass * g * ALTITUDE_SATURATION;
        }

        float E = Uerror + Kerror;
        float B = Uerror - Kerror;

        piAirspeed.Update(E, 0.0f, T);
        piAltitude.Update(B, 0.0f, T);

        return new float[] { piAirspeed.output, piAltitude.output };

    }

}

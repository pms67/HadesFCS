using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class AircraftDynamics {
    const float mass = 1.56f;
    const float Iyy = 0.0576f;
    const float S = 0.2589f;
    const float c = 0.3302f;

    const float CD0 = 0.01631f;
    const float CDalpha = 0.2108f;
    const float CDq = 0.0f;
    const float CDde = 0.3045f;

    const float CL0 = 0.09167f;
    const float CLalpha = 3.5016f;
    const float CLq = 2.8932f;
    const float CLde = 0.2724f;

    const float Cm0 = -0.02338f;
    const float Cmalpha = -0.5675f;
    const float Cmq = -1.3990f;
    const float Cmde = -0.3254f;

    const float g = 9.8605f;
    const float rho = 1.225f;

    public float[] acState;
    public float airspeed;
    public float angleOfAttack;

    // Sensor measurements
    public float pitotVa, gyroQ, accX, accZ, baroAltitude;

    // Wind model
    DrydenWind wind = new DrydenWind(1.06f, 200.0f, 0.7f, 50.0f);
    
    public AircraftDynamics()
    {
        acState = new float[6];
        pitotVa = gyroQ = accX = accZ = baroAltitude = 0.0f;
    }

    public void Init(float initAirspeedmps, float initPitchAngleRadians, float initAltitudem)
    {
        acState[0] = initAirspeedmps;
        acState[1] = 0.0f;
        acState[2] = 0.0f;
        acState[3] = initPitchAngleRadians;
        acState[4] = 0.0f;
        acState[5] = initAltitudem;

        airspeed = initAirspeedmps;
        angleOfAttack = GetAngleOfAttack(acState[0], acState[1]);
    }

    public void Update(float engineThrust, float elevatorDeflectionRadians, float T)
    {
        int n;
        float[] newState = new float[6];

        float[] k1 = StateDerivatives(acState, new float[] { engineThrust, elevatorDeflectionRadians });
        for (n = 0; n < 6; n++)
        {
            k1[n] *= T;
            newState[n] = acState[n] + 0.5f * k1[n];
        }

        float[] k2 = StateDerivatives(newState, new float[] { engineThrust, elevatorDeflectionRadians });
        for (n = 0; n < 6; n++)
        {
            k2[n] *= T;
            newState[n] = acState[n] + 0.5f * k2[n];
        }

        float[] k3 = StateDerivatives(newState, new float[] { engineThrust, elevatorDeflectionRadians });
        for (n = 0; n < 6; n++)
        {
            k3[n] *= T;
            newState[n] = acState[n] + k3[n];
        }

        float[] k4 = StateDerivatives(newState, new float[] { engineThrust, elevatorDeflectionRadians });
        for (n = 0; n < 6; n++)
        {
            k4[n] *= T;
            acState[n] = acState[n] + (k1[n] + 2.0f * k2[n] + 2.0f * k3[n] + k4[n]) / 6.0f;
        }

        /* Add wind vector */
    //  float[] winduv = wind.Update(airspeed, T);
    //  acState[0] = acState[0] + winduv[0];
    //  acState[1] = acState[1] + winduv[1];

        airspeed = GetAirspeed(acState[0], acState[1]);

        angleOfAttack = GetAngleOfAttack(acState[0], acState[1]);
    }

    public float GetAirspeed(float u, float w)
    {
        return ((float)System.Math.Sqrt(u * u + w * w));
    }

    public float GetAngleOfAttack(float u, float w)
    {
        return ((float)System.Math.Atan2(w, u));
    }

    private float[] ForcesAndMoments(float[] state, float[] inputs)
    {
        float Va = GetAirspeed(state[0], state[1]);
        float alpha = GetAngleOfAttack(state[0], state[1]);

        float q = state[2];

        float dt = inputs[0];
        float de = inputs[1];

        float Fu = dt - 0.5f * rho * Va * Va * S * (CD0 + CDalpha * alpha + CDq * c / (2.0f * Va) * q + CDde * de);
        float Fw = -0.5f * rho * Va * Va * S * (CL0 + CLalpha * alpha + CLq * c / (2.0f * Va) * q + CLde * de);

        float m = 0.5f * rho * Va * Va * S * c * (Cm0 + Cmalpha * alpha + Cmq * c / (2.0f * Va) * q + Cmde * de);

        return new float[] { Fu, Fw, m };
    }

    private float[] StateDerivatives(float[] state, float[] inputs)
    {
        float u = state[0];
        float w = state[1];
        float q = state[2];
        float theta = state[3];

        float[] Fu_Fw_M = ForcesAndMoments(state, inputs);

        float udot = Fu_Fw_M[0] / mass - q * w - g * (float)System.Math.Sin(theta);
        float wdot = Fu_Fw_M[1] / mass + q * u + g * (float)System.Math.Cos(theta);
        float qdot = Fu_Fw_M[2] / Iyy;
        float thetadot = q;
        float xdot = u * (float)System.Math.Cos(theta) + w * (float)System.Math.Sin(theta);
        float ydot = u * (float)System.Math.Sin(theta) - w * (float)System.Math.Cos(theta);

        return new float[] { udot, wdot, qdot, thetadot, xdot, ydot };
    }

    public void GetSensorMeasurements()
    {
        float u = acState[0];
        float w = acState[1];
        float q = acState[2];
        float theta = acState[3];

        pitotVa = GetAirspeed(u, w) + 0.2f * randn();
        gyroQ = q + 0.01f * randn();
        accX = q * w + g * Mathf.Sin(theta) + 0.01f * randn();
        accZ = -q * u - g * Mathf.Cos(theta) + 0.01f * randn();
        baroAltitude = acState[5] + 0.5f * randn();
    }

    private float randn()
    {
        float v1, v2, s;
        do
        {
            v1 = 2.0f * Random.Range(0f, 1f) - 1.0f;
            v2 = 2.0f * Random.Range(0f, 1f) - 1.0f;
            s = v1 * v1 + v2 * v2;
        } while (s >= 1.0f || s == 0f);

        s = Mathf.Sqrt((-2.0f * Mathf.Log(s)) / s);

        return v1 * s;
    }
}

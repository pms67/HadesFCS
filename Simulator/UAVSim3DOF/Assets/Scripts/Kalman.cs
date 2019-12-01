using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Kalman {

    public float theta;

    private float P, Q, R;
    private float lpfGyrCoeff, lpfAccCoeff, lpfVaCoeff;
    public float qFilt, axFilt, azFilt, VaFilt;

    private float g = 9.81f;
    
    public Kalman(float P0, float Q, float R)
    {
        theta = 0.0f;
        P = P0;
        this.Q = Q;
        this.R = R;
        lpfGyrCoeff = 0.0f;
        lpfAccCoeff = 0.0f;
        lpfVaCoeff = 0.0f;
        qFilt = 0.0f;
        axFilt = 0.0f;
        azFilt = 0.0f;
        VaFilt = 0.0f;
    }

    public void SetFilters(float lpfGyrCoeff, float lpfAccCoeff, float lpfVaCoeff)
    {
        this.lpfGyrCoeff = lpfGyrCoeff;
        this.lpfAccCoeff = lpfAccCoeff;
        this.lpfVaCoeff = lpfVaCoeff;
    }

    public float Update(float q, float ax, float az, float Va, float T)
    {
        qFilt = lpfGyrCoeff * qFilt + (1.0f - lpfGyrCoeff) * q;
        axFilt = lpfAccCoeff * axFilt + (1.0f - lpfAccCoeff) * ax;
        azFilt = lpfAccCoeff * azFilt + (1.0f - lpfAccCoeff) * az;
        VaFilt = lpfVaCoeff * VaFilt + (1.0f - lpfVaCoeff) * Va;

        theta = theta + T * qFilt;
        P = P + T * Q;

        float h1 = g * Mathf.Sin(theta);
        float h2 = -qFilt * VaFilt - g * Mathf.Cos(theta);

        float C1 = g * Mathf.Cos(theta);
        float C2 = g * Mathf.Sin(theta);

        float G11 = P * C1 * C1 + R;
        float G12 = P * C1 * C2;
        float G21 = P * C1 * C2;
        float G22 = P * C2 * C2 + R;

        float idet = 1.0f / (G11 * G22 - G12 * G21);

        float tmp = G11;
        G11 = G22 * idet;
        G22 = tmp * idet;
        G12 = -G12 * idet;
        G21 = -G21 * idet;

        float K1 = P * (C1 * G11 + C2 * G21);
        float K2 = P * (C1 * G12 + C2 * G22);

        theta = theta + K1 * (axFilt - h1) + K2 * (azFilt - h2);
        P = (1.0f - (K1 * C1 + K2 * C2)) * P;

        return theta;
    }

}

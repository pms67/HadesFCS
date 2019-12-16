using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DrydenWind {

    public float sigmau, Lu;
    public float sigmaw, Lw;

    private float inuprev, inwprev, inwprev2;
    private float outuprev, outwprev, outwprev2;

    public DrydenWind(float sigmau, float Lu, float sigmaw, float Lw)
    {
        this.sigmau = sigmau;
        this.Lu = Lu;

        this.sigmaw = sigmaw;
        this.Lw = Lw;

        inuprev = inwprev = 0.0f;
        outuprev = outwprev = 0.0f;
    }

    public float[] Update(float Va, float T)
    {
        float k1u = sigmau * Mathf.Sqrt(2.0f * Va / Lu);
        float k2u = Va / Lu;

        float inu = randn();
        float outu = (k1u * T * (inu + inuprev) - outuprev * (k2u * T - 2.0f)) / (2.0f + k2u * T);

        float k1w = sigmaw * Mathf.Sqrt(3.0f * Va / Lw);
        float k2w = Va / (Mathf.Sqrt(3.0f) * Lw);
        float k3w = Va / Lw;

        float inw = randn();
        float outw = k1w * T * ((2.0f + k2w * T) * inw + 2.0f * k2w * T * inwprev + (k2w * T - 2.0f) * inwprev2);
        outw = outw - (2.0f * k3w * k3w * T * T - 8.0f) - (4 - 4.0f * T * k3w + k3w * k3w * T * T);
        outw = outw / (4.0f + 4.0f * T * k3w + k3w * k3w * T * T);

        inuprev = inu;
        inwprev2 = inwprev;
        inwprev = inw;

        outw = 0.0f;

        outuprev = outu;
        outwprev2 = outwprev;
        outwprev = outw;

        return new float[] { outu, outw };
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
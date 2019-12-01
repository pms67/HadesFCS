using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System;

public class Example : MonoBehaviour
{
    public SimplestPlot.PlotType PlotExample = SimplestPlot.PlotType.TimeSeries;
    public int DataPoints = 100;
    private SimplestPlot SimplestPlotScript;
    private float Counter = 0;
    private Color[] MyColors = new Color[2];

    private System.Random MyRandom;
    private float[] XValues;
    private float[] Y1Values;
    private float[] Y2Values;

    private Vector2 Resolution;
    // Use this for initialization
    void Start()
    {
        SimplestPlotScript = GetComponent<SimplestPlot>();

        MyRandom = new System.Random();
        XValues = new float[DataPoints];
        Y1Values = new float[DataPoints];
        Y2Values = new float[DataPoints-2];
        MyColors[0] = Color.white;
        MyColors[1] = Color.blue;

        SimplestPlotScript.SetResolution(new Vector2(300, 300));
        SimplestPlotScript.BackGroundColor = new Color(0.1f, 0.1f, 0.1f, 0.4f);
        SimplestPlotScript.TextColor = Color.yellow;
        for (int Cnt = 0; Cnt < 2; Cnt++)
        {
            SimplestPlotScript.SeriesPlotY.Add(new SimplestPlot.SeriesClass());
            SimplestPlotScript.DistributionPlot.Add(new SimplestPlot.DistributionClass());
            SimplestPlotScript.PhaseSpacePlot.Add(new SimplestPlot.PhaseSpaceClass());

            SimplestPlotScript.SeriesPlotY[Cnt].MyColor = MyColors[Cnt];
            SimplestPlotScript.DistributionPlot[Cnt].MyColor = MyColors[Cnt];
            SimplestPlotScript.PhaseSpacePlot[Cnt].MyColor = MyColors[Cnt];

            SimplestPlotScript.DistributionPlot[Cnt].NumberOfBins = (Cnt + 1) * 5;
        }

        Resolution = SimplestPlotScript.GetResolution();
    }

    // Update is called once per frame
    void Update()
    {
        Counter++;
        PrepareArrays();
        SimplestPlotScript.MyPlotType = PlotExample;
        switch (PlotExample)
        {
            case SimplestPlot.PlotType.TimeSeries:
                SimplestPlotScript.SeriesPlotY[0].YValues = Y1Values;
                SimplestPlotScript.SeriesPlotY[1].YValues = Y2Values;
                SimplestPlotScript.SeriesPlotX = XValues;
                break;
            case SimplestPlot.PlotType.Distribution:
                SimplestPlotScript.DistributionPlot[0].Values = Y1Values;
                SimplestPlotScript.DistributionPlot[1].Values = Y2Values;
                break;
            case SimplestPlot.PlotType.PhaseSpace:
                SimplestPlotScript.PhaseSpacePlot[0].XValues = XValues;
                SimplestPlotScript.PhaseSpacePlot[0].YValues = Y1Values;
                SimplestPlotScript.PhaseSpacePlot[1].XValues = Y1Values;
                SimplestPlotScript.PhaseSpacePlot[1].YValues = Y2Values;
                break;
            default:
                break;
        }
        SimplestPlotScript.UpdatePlot();
    }
    private void PrepareArrays()
    {
        for (int Cnt = 0; Cnt < DataPoints; Cnt++)
        {
            XValues[Cnt] = (Counter + Cnt) * Mathf.PI / (Resolution.x);
            Y1Values[Cnt] = Mathf.Cos(XValues[Cnt]) * 20;
            if (Cnt < DataPoints - 2) Y2Values[Cnt] = Mathf.Sin(XValues[Cnt]) * 10 + 7;
        }
    }
}

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System;
using System.Linq;

public class SimplestPlot : MonoBehaviour
{
    public class SeriesClass
    {
        public float[] YValues;
        public Color MyColor;
        public SeriesClass() { YValues = new float[0]; MyColor = new Color(1, 1, 1); }
    }
    public class DistributionClass
    {
        public float[] Values;// Extracted values from the variable to plot
        public int NumberOfBins = 10;
        public Color MyColor;
        public DistributionClass() { Values = new float[0]; MyColor = new Color(1, 1, 1); }
    }
    public class PhaseSpaceClass
    {
        public float[] XValues;
        public float[] YValues;
        public Color MyColor;
        public PhaseSpaceClass() { XValues = new float[0]; YValues = new float[0]; MyColor = new Color(1, 1, 1); }
    }
    public enum PlotType
    {
        TimeSeries = 0,
        Distribution = 1,
        PhaseSpace = 2
    }
    private Texture2D PlotTexture;
    private Image PlotImage;
    private Vector2 TextureResolution = new Vector2(100, 100);
    private float StartPosX;
    private float StartPosY;

    // Global Variables
    public Color BackGroundColor = new Color(0, 0, 0, 0);
    public byte Ar = 0;
    private Color PrevBackGroundColor = new Color(0, 0, 0, 0);
    private Color[] BackGroundArray;
    public Color TextColor = new Color(1, 1, 1, 1);
    public int FontSize = 12;
    public PlotType MyPlotType = PlotType.TimeSeries;
    public bool ShowWarnings = true;
    public bool AxesVisible = true;

    // PLot variables
    public List<SeriesClass> SeriesPlotY;
    public float[] SeriesPlotX;
    public List<DistributionClass> DistributionPlot;
    private List<float[]> ActualDistribution;
    public List<PhaseSpaceClass> PhaseSpacePlot;

    private Text PlotYMaxText;
    private Text PlotYMinText;
    private Text PlotXMaxText;
    private Text PlotXMinText;
    private Text WarningText;
    // Use this for initialization
    public void Start()
    {
        PlotImage = GetComponent<Image>();
        if (PlotImage == null) throw new Exception("Simplest plot needs an image component in the same GameObject in order to work.");

        SeriesPlotY = new List<SeriesClass>();
        DistributionPlot = new List<DistributionClass>();
        ActualDistribution = new List<float[]>();
        PhaseSpacePlot = new List<PhaseSpaceClass>();

        SetResolution(TextureResolution);

        SetupText();
    }

    // Getters Setters
    public void SetResolution(Vector2 NewResolution)
    {
        TextureResolution = NewResolution;
        PlotTexture = new Texture2D((int)TextureResolution.x, (int)TextureResolution.y);
        PlotImage.GetComponent<Image>().sprite = Sprite.Create(PlotTexture, new Rect(0, 0, TextureResolution.x, TextureResolution.y), new Vector2(0.0f, 0.0f));
        StartPosX = TextureResolution.x * 0.05f;
        StartPosY = TextureResolution.y * 0.05f;
    }
    public Vector2 GetResolution() { return TextureResolution; }

    // Main Update. Can put this into Update, but might be better to update only when data has changed
    public void UpdatePlot()
    {
        PlotTexture.SetPixels(GetFreshBackGroundArray());
        float[] MinMaxOfPlotsY = new float[2];
        float[] MinMaxOfPlotsX = new float[2];
        float ScaleX = 0;
        float ScaleY = 0;
        if (ShowWarnings) WarningText.text = ConsistencyCheck();
        else WarningText.text = "";

        if (MyPlotType == PlotType.TimeSeries)
        {
            MinMaxOfPlotsX = FindMinMaxSeries(true);
            MinMaxOfPlotsY = FindMinMaxSeries(false);
            ScaleX = (float)(PlotTexture.width - 2 * StartPosX) / (float)(MinMaxOfPlotsX[1] - MinMaxOfPlotsX[0]);
            ScaleY = (float)(PlotTexture.height - 2 * StartPosY) / (float)(MinMaxOfPlotsY[1] - MinMaxOfPlotsY[0]);
            for (int Cnt = 0; Cnt < SeriesPlotY.Count; Cnt++)
                DrawTimeSeriesPlot(ScaleX, ScaleY, SeriesPlotX, MinMaxOfPlotsX[0], SeriesPlotY[Cnt].YValues, MinMaxOfPlotsY[0], SeriesPlotY[Cnt].MyColor);
        }
        else if (MyPlotType == PlotType.Distribution)
        {
            MinMaxOfPlotsX = FindMinMaxOfDistribution();
            MinMaxOfPlotsY = PrepareDistribution(MinMaxOfPlotsX);
            ScaleX = (float)(PlotTexture.width - 2 * StartPosX) / (float)(MinMaxOfPlotsX[1] - MinMaxOfPlotsX[0]);
            ScaleY = (float)(PlotTexture.height - 2 * StartPosY) / (float)(MinMaxOfPlotsY[1] - MinMaxOfPlotsY[0]);
            for (int Cnt = 0; Cnt < DistributionPlot.Count; Cnt++)
            {
                float BinDimention = (MinMaxOfPlotsX[1] - MinMaxOfPlotsX[0]) / DistributionPlot[Cnt].NumberOfBins;
                DrawDistributionPlot(ScaleX, ScaleY, BinDimention, MinMaxOfPlotsY[0], ActualDistribution[Cnt], SeriesPlotY[Cnt].MyColor);
            }
        }
        else
        {
            MinMaxOfPlotsX = FindMinMaxPhase(true);
            MinMaxOfPlotsY = FindMinMaxPhase(false);
            ScaleX = (float)(PlotTexture.width - 2 * StartPosX) / (float)(MinMaxOfPlotsX[1] - MinMaxOfPlotsX[0]);
            ScaleY = (float)(PlotTexture.height - 2 * StartPosY) / (float)(MinMaxOfPlotsY[1] - MinMaxOfPlotsY[0]);
            for (int Cnt = 0; Cnt < PhaseSpacePlot.Count; Cnt++)
                DrawPhaseSpacePlot(ScaleX, ScaleY, PhaseSpacePlot[Cnt].XValues, MinMaxOfPlotsX[0], PhaseSpacePlot[Cnt].YValues, MinMaxOfPlotsY[0], PhaseSpacePlot[Cnt].MyColor);
        }

        UpdateText(MinMaxOfPlotsX, MinMaxOfPlotsY);
        PlotTexture.Apply();
        Resources.UnloadUnusedAssets();
    }

    // Draw Functions

    private void DrawTimeSeriesPlot(float PlotScaleX, float PlotScaleY, float[] XPoints, float MinX, float[] YPoints, float MinY, Color LineColor)
    {
        int FirstX, FirstY, SecondX, SecondY;
        bool NoXValues = XPoints == null || XPoints.Length == 0;
        SecondX = (int)StartPosX;
        SecondY = (int)((YPoints[0] - MinY) * PlotScaleY + StartPosY);

        for (int Cnt = 1; Cnt < YPoints.Length; Cnt++)
        {
            FirstX = SecondX;
            FirstY = SecondY;
            SecondX = NoXValues ? (int)(Cnt * PlotScaleX + StartPosX) : (int)((XPoints[Cnt] - MinX) * PlotScaleX + StartPosX);
            SecondY = (int)((YPoints[Cnt] - MinY) * PlotScaleY + StartPosY);
            DrawLine(PlotTexture, FirstX, FirstY, SecondX, SecondY, LineColor);
        }
    }
    private void DrawDistributionPlot(float PlotScaleX, float PlotScaleY, float BinDimention, float MinY, float[] YPoints, Color LineColor)
    {
        int FirstX, FirstY, SecondX, SecondY;
        FirstY = (int)StartPosY;
        SecondX = (int)StartPosX;

        for (int Cnt = 0; Cnt < YPoints.Length; Cnt++)
        {
            //Draw Vertical line
            FirstX = SecondX;
            SecondY = (int)((YPoints[Cnt] - MinY) * PlotScaleY + StartPosY);
            DrawLine(PlotTexture, FirstX, FirstY, SecondX, SecondY, LineColor);
            //Draw horizontal line
            FirstY = SecondY;
            SecondX += (int)(BinDimention * PlotScaleX);
            DrawLine(PlotTexture, FirstX, FirstY, SecondX, SecondY, LineColor);
        }
        //Draw Last Vertical
        FirstX = SecondX;
        SecondY = (int)StartPosY;
        DrawLine(PlotTexture, FirstX, FirstY, SecondX, SecondY, LineColor);
    }
    private void DrawPhaseSpacePlot(float PlotScaleX, float PlotScaleY, float[] XPoints, float MinX, float[] YPoints, float MinY, Color LineColor)
    {
        int MinPointsNum = Math.Min(XPoints.Length, YPoints.Length);
        int FirstX, FirstY, SecondX, SecondY;
        SecondX = (int)((XPoints[0] - MinX) * PlotScaleX + StartPosX);
        SecondY = (int)((YPoints[0] - MinY) * PlotScaleY + StartPosY);
        for (int Cnt = 1; Cnt < MinPointsNum; Cnt++)
        {
            FirstX = SecondX;
            FirstY = SecondY;
            if (Cnt <= XPoints.Length) SecondX = (int)((XPoints[Cnt] - MinX) * PlotScaleX + StartPosX);
            if (Cnt <= YPoints.Length) SecondY = (int)((YPoints[Cnt] - MinY) * PlotScaleY + StartPosY);

            DrawLine(PlotTexture, FirstX, FirstY, SecondX, SecondY, LineColor);
        }
    }
    private void DrawLine(Texture2D PlotTexture, int x0, int y0, int x1, int y1, Color LineColor)
    {
        int dx = 0;
        int dy = 0;
        int sx = 0;
        int sy = 0;
        dx = Math.Abs(x1 - x0);
        dy = Math.Abs(y1 - y0);
        sx = x0 < x1 ? 1 : -1;
        sy = y0 < y1 ? 1 : -1;
        int err = (dx > dy ? dx : -dy) / 2, e2;

        while (true)
        {
            PlotTexture.SetPixel(x0, y0, LineColor);
            if (x0 == x1 && y0 == y1) break;
            e2 = err;
            if (e2 > -dx) { err -= dy; x0 += sx; }
            if (e2 < dy) { err += dx; y0 += sy; }
        }
    }
    // Support Functios
    private float[] FindMinMaxSeries(bool XNotY)
    {
        float[] MinMaxOnArray = new float[2];
        float[] ToReturn = new float[2];
        ToReturn[0] = float.MaxValue;
        ToReturn[1] = float.MinValue;
        if (XNotY)
        {

            if (SeriesPlotX == null || SeriesPlotX.Length == 0)
            {
                for (int Cnt = 0; Cnt < SeriesPlotY.Count; Cnt++)
                {
                    MinMaxOnArray[1] = SeriesPlotY[Cnt].YValues.Length;
                    ToReturn[1] = Math.Max(ToReturn[1], MinMaxOnArray[1]);
                }
                ToReturn[0] = 1;
            }
            else
            {
                ToReturn[0] = SeriesPlotX.Min();
                ToReturn[1] = SeriesPlotX.Max();
            }
        }
        else
        {
            for (int Cnt = 0; Cnt < SeriesPlotY.Count; Cnt++)
            {
                MinMaxOnArray[1] = SeriesPlotY[Cnt].YValues.Max();
                MinMaxOnArray[0] = SeriesPlotY[Cnt].YValues.Min();
                ToReturn[0] = Math.Min(ToReturn[0], MinMaxOnArray[0]);
                ToReturn[1] = Math.Max(ToReturn[1], MinMaxOnArray[1]);
            }
        }

        return CheckLimits(ToReturn);
    }
    private float[] FindMinMaxPhase(bool XNotY)
    {
        float[] MinMaxOnArray = new float[2];
        float[] ToReturn = new float[2];
        ToReturn[0] = float.MaxValue;
        ToReturn[1] = float.MinValue;
        if (XNotY)
        {
            for (int Cnt = 0; Cnt < PhaseSpacePlot.Count; Cnt++)
            {
                MinMaxOnArray[1] = PhaseSpacePlot[Cnt].XValues.Max();
                MinMaxOnArray[0] = PhaseSpacePlot[Cnt].XValues.Min();
                ToReturn[0] = Math.Min(ToReturn[0], MinMaxOnArray[0]);
                ToReturn[1] = Math.Max(ToReturn[1], MinMaxOnArray[1]);
            }
        }
        else
            for (int Cnt = 0; Cnt < PhaseSpacePlot.Count; Cnt++)
            {
                MinMaxOnArray[1] = PhaseSpacePlot[Cnt].YValues.Max();
                MinMaxOnArray[0] = PhaseSpacePlot[Cnt].YValues.Min();
                ToReturn[0] = Math.Min(ToReturn[0], MinMaxOnArray[0]);
                ToReturn[1] = Math.Max(ToReturn[1], MinMaxOnArray[1]);
            }
        return CheckLimits(ToReturn);
    }
    private float[] FindMinMaxOfDistribution()
    {
        float[] MinMaxOnArray = new float[2];
        float[] ToReturn = new float[2];
        ToReturn[0] = float.MaxValue;
        ToReturn[1] = float.MinValue;
        for (int Cnt = 0; Cnt < DistributionPlot.Count; Cnt++)
        {
            MinMaxOnArray[1] = DistributionPlot[Cnt].Values.Max();
            MinMaxOnArray[0] = DistributionPlot[Cnt].Values.Min();
            ToReturn[0] = Math.Min(ToReturn[0], MinMaxOnArray[0]);
            ToReturn[1] = Math.Max(ToReturn[1], MinMaxOnArray[1]);
        }
        return CheckLimits(ToReturn);
    }
    private float[] PrepareDistribution(float[] MinMax)
    {
        float[] ToReturn = new float[2];
        ToReturn[0] = int.MaxValue;
        ToReturn[1] = int.MinValue;
        ActualDistribution = new List<float[]>();
        for (int Cnt = 0; Cnt < DistributionPlot.Count; Cnt++)
        {
            ActualDistribution.Add(MakeDistribution(DistributionPlot[Cnt].Values, MinMax, Cnt));
            ToReturn[0] = Math.Min(ToReturn[0], ActualDistribution[Cnt].Min());
            ToReturn[1] = Math.Max(ToReturn[1], ActualDistribution[Cnt].Max());
        }

        return ToReturn;
    }
    private float[] MakeDistribution(float[] MainArray, float[] MinMax, int Index)
    {
        float Dist = (float)(MinMax[1] - MinMax[0] + 1) / DistributionPlot[Index].NumberOfBins;
        float[] Distribution = Enumerable.Repeat(0f, DistributionPlot[Index].NumberOfBins).ToArray();

        for (int Cnt = 0; Cnt < MainArray.Length; Cnt++)
        {
            Distribution[(int)Math.Floor((MainArray[Cnt] - MinMax[0]) / Dist)]++;
        }
        return Distribution;
    }

    private string ConsistencyCheck()
    {
        switch (MyPlotType)
        {
            case PlotType.TimeSeries:
                if (SeriesPlotY.Count == 0) return "Empty Series List. Please add some data.";
                for (int Cnt = 0; Cnt < SeriesPlotY.Count; Cnt++)
                    if (SeriesPlotY[Cnt].YValues.Length == 0)
                        return "Series " + Cnt + " Has no Data.";
                if ((SeriesPlotX == null || SeriesPlotX.Length == 0) && SeriesPlotY.Count > 1)// No Xvalues. X axis will be set depenting on the count of YValues
                {
                    for (int Cnt = 0; Cnt < SeriesPlotY.Count - 1; Cnt++)
                        for (int CntInt = Cnt + 1; CntInt < SeriesPlotY.Count; CntInt++)
                            if (SeriesPlotY[Cnt].YValues.Length != SeriesPlotY[CntInt].YValues.Length)
                                return "Inconsistent length of series " + Cnt + "(" + SeriesPlotY[Cnt].YValues.Length + ") and " + CntInt + "(" + SeriesPlotY[CntInt].YValues.Length + ")";
                }
                else
                {
                    for (int Cnt = 0; Cnt < SeriesPlotY.Count; Cnt++)
                        if (SeriesPlotY[Cnt].YValues.Length != SeriesPlotX.Length)
                            return "Inconsistent length of series " + Cnt + "(" + SeriesPlotY[Cnt].YValues.Length + ") and XValues(" + SeriesPlotX.Length + ")";
                }
                break;
            case PlotType.Distribution:
                if (DistributionPlot.Count == 0) return "Empty Distributions List. Please add some data.";
                for (int Cnt = 0; Cnt < DistributionPlot.Count; Cnt++)
                    if (DistributionPlot[Cnt].Values.Length == 0)
                        return "Distribution " + Cnt + " Has no Data.";
                break;
            case PlotType.PhaseSpace:
                if (PhaseSpacePlot.Count == 0) return "Empty Phase Space List. Please add some data.";
                for (int Cnt = 0; Cnt < PhaseSpacePlot.Count; Cnt++)
                {
                    if (PhaseSpacePlot[Cnt].YValues.Length == 0)
                        return "Phase Space " + Cnt + " Has no YData.";
                    else if (PhaseSpacePlot[Cnt].XValues.Length == 0)
                        return "Phase Space " + Cnt + " Has no XData.";
                    else if (PhaseSpacePlot[Cnt].YValues.Length != PhaseSpacePlot[Cnt].XValues.Length)
                        return "Phase Space " + Cnt + " Has inconsistent Data.(" + PhaseSpacePlot[Cnt].YValues.Length + " vs " + PhaseSpacePlot[Cnt].XValues.Length + ")";
                }

                for (int Cnt = 0; Cnt < PhaseSpacePlot.Count - 1; Cnt++)
                    for (int CntInt = Cnt + 1; CntInt < PhaseSpacePlot.Count; CntInt++)
                        if (PhaseSpacePlot[Cnt].YValues.Length != PhaseSpacePlot[CntInt].YValues.Length)
                            return "Inconsistent length of State Spaces " + Cnt + "(" + PhaseSpacePlot[Cnt].YValues.Length + ") and " + CntInt + "(" + PhaseSpacePlot[CntInt].YValues.Length + ")";
                break;
            default:
                throw new Exception("No such PlotType!");
        }
        return "";
    }
    private void UpdateText(float[] MinMaxOfPlotsX, float[] MinMaxOfPlotsY)
    {
        PlotYMaxText.color = TextColor;
        PlotYMinText.color = TextColor;
        PlotXMaxText.color = TextColor;
        PlotXMinText.color = TextColor;
        WarningText.color = TextColor;

        PlotYMaxText.fontSize = FontSize;
        PlotYMinText.fontSize = FontSize;
        PlotXMaxText.fontSize = FontSize;
        PlotXMinText.fontSize = FontSize;
        WarningText.fontSize = FontSize;

        if (AxesVisible && MinMaxOfPlotsY[1] < float.MaxValue && MinMaxOfPlotsY[0] > float.MinValue)
        {
            if (MinMaxOfPlotsY[1] >= MinMaxOfPlotsY[0])
            {
                PlotYMaxText.text = MinMaxOfPlotsY[1].ToString("0.00");
                PlotYMinText.text = MinMaxOfPlotsY[0].ToString("0.00");
            }
            else
            {
                PlotYMaxText.text = "";
                PlotYMinText.text = "";
            }
            if (MinMaxOfPlotsX[0] > MinMaxOfPlotsX[1])
            {
                PlotXMaxText.text = "";
                PlotXMinText.text = "";
            }
            else
            {
                PlotXMaxText.text = MinMaxOfPlotsX[1].ToString("0.00");
                PlotXMinText.text = MinMaxOfPlotsX[0].ToString("0.00");
            }
        }
        else
        {
            PlotYMaxText.text = "";
            PlotYMinText.text = "";
            PlotXMaxText.text = "";
            PlotXMinText.text = "";
        }
    }
    private void SetupText()
    {
        GameObject GOPlotXMinText = new GameObject("XMin", typeof(Text));
        GameObject GOPlotXMaxText = new GameObject("XMax", typeof(Text));
        GameObject GOPlotYMinText = new GameObject("YMin", typeof(Text));
        GameObject GOPlotYMaxText = new GameObject("YMax", typeof(Text));
        GameObject GOWarningText = new GameObject("Warning", typeof(Text));
        GOPlotXMinText.transform.SetParent(transform, false);
        GOPlotXMaxText.transform.SetParent(transform, false);
        GOPlotYMinText.transform.SetParent(transform, false);
        GOPlotYMaxText.transform.SetParent(transform, false);
        GOWarningText.transform.SetParent(transform, false);
        PlotXMinText = GOPlotXMinText.GetComponent<Text>();
        PlotXMaxText = GOPlotXMaxText.GetComponent<Text>();
        PlotYMinText = GOPlotYMinText.GetComponent<Text>();
        PlotYMaxText = GOPlotYMaxText.GetComponent<Text>();
        WarningText = GOWarningText.GetComponent<Text>();

        PlotYMaxText.alignment = TextAnchor.MiddleLeft;
        PlotYMinText.alignment = TextAnchor.MiddleLeft;
        PlotXMaxText.alignment = TextAnchor.MiddleRight;
        PlotXMinText.alignment = TextAnchor.MiddleLeft;
        WarningText.alignment = TextAnchor.MiddleCenter;

        PlotYMaxText.font = Resources.GetBuiltinResource(typeof(Font), "Arial.ttf") as Font;
        PlotYMinText.font = Resources.GetBuiltinResource(typeof(Font), "Arial.ttf") as Font;
        PlotXMaxText.font = Resources.GetBuiltinResource(typeof(Font), "Arial.ttf") as Font;
        PlotXMinText.font = Resources.GetBuiltinResource(typeof(Font), "Arial.ttf") as Font;
        WarningText.font = Resources.GetBuiltinResource(typeof(Font), "Arial.ttf") as Font;

        PlotYMaxText.color = TextColor;
        PlotYMinText.color = TextColor;
        PlotXMaxText.color = TextColor;
        PlotXMinText.color = TextColor;
        WarningText.color = TextColor;

        PlotYMaxText.fontSize = FontSize;
        PlotYMinText.fontSize = FontSize;
        PlotXMaxText.fontSize = FontSize;
        PlotXMinText.fontSize = FontSize;
        WarningText.fontSize = FontSize;

        //These might be nice to make them depend on TextureResolution
        PlotYMaxText.GetComponent<RectTransform>().anchorMin = new Vector2(0, 0.9f);
        PlotYMaxText.GetComponent<RectTransform>().anchorMax = new Vector2(0.1f, 1);
        PlotYMaxText.GetComponent<RectTransform>().offsetMin = new Vector2(0, 0);
        PlotYMaxText.GetComponent<RectTransform>().offsetMax = new Vector2(0, 0);
        PlotYMinText.GetComponent<RectTransform>().anchorMin = new Vector2(0, 0.1f);
        PlotYMinText.GetComponent<RectTransform>().anchorMax = new Vector2(0.1f, 0.2f);
        PlotYMinText.GetComponent<RectTransform>().offsetMin = new Vector2(0, 0);
        PlotYMinText.GetComponent<RectTransform>().offsetMax = new Vector2(0, 0);
        PlotXMaxText.GetComponent<RectTransform>().anchorMin = new Vector2(0.9f, 0);
        PlotXMaxText.GetComponent<RectTransform>().anchorMax = new Vector2(1, 0.1f);
        PlotXMaxText.GetComponent<RectTransform>().offsetMin = new Vector2(0, 0);
        PlotXMaxText.GetComponent<RectTransform>().offsetMax = new Vector2(0, 0);
        PlotXMinText.GetComponent<RectTransform>().anchorMin = new Vector2(0.08f, 0);
        PlotXMinText.GetComponent<RectTransform>().anchorMax = new Vector2(0.18f, 0.1f);
        PlotXMinText.GetComponent<RectTransform>().offsetMin = new Vector2(0, 0);
        PlotXMinText.GetComponent<RectTransform>().offsetMax = new Vector2(0, 0);
        WarningText.GetComponent<RectTransform>().anchorMin = new Vector2(0, 0);
        WarningText.GetComponent<RectTransform>().anchorMax = new Vector2(1, 1);
        WarningText.GetComponent<RectTransform>().offsetMin = new Vector2(0, 0);
        WarningText.GetComponent<RectTransform>().offsetMax = new Vector2(0, 0);
    }
    private float[] CheckLimits(float[] InMinMax)
    {
        if (InMinMax[0] == InMinMax[1])
            InMinMax[1] = InMinMax[0] + 0.01f;
        return InMinMax;
    }
    private Color[] GetFreshBackGroundArray()// This is used for speed. Creating the enumerable on every frame may slow down things
    {
        Color[] ToReturn = new Color[(int)(TextureResolution.x * TextureResolution.y)];
        if (PrevBackGroundColor != BackGroundColor)
        {
            PrevBackGroundColor = BackGroundColor;
            BackGroundArray = Enumerable.Repeat(BackGroundColor, (int)(TextureResolution.x * TextureResolution.y)).ToArray();
        }
        Array.Copy(BackGroundArray, 0, ToReturn, 0, (int)(TextureResolution.x * TextureResolution.y));
        return ToReturn;
    }
}

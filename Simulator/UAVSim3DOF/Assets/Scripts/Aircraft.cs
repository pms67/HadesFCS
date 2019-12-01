using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class Aircraft : MonoBehaviour
{
    /* Initial state */
    const float initAirspeedmps = 15.0f;
    const float initPitchAngleRadians = 5.0f * DEGTORAD;
    const float initAltitudem = 100.0f;

    const float SAMPLE_TIME_SENSORS = 0.01f;
    const float SAMPLE_TIME_KALMAN  = 0.01f;
    const float SAMPLE_TIME_CTRL_AIRSPEED = 0.1f;
    const float SAMPLE_TIME_CTRL_PITCH = 0.05f;
    const float SAMPLE_TIME_CTRL_ALTITUDE = 0.1f;

    /* UAV and camera game object transforms and offset */
    public Transform trans;
    public Transform camTrans;
    private Vector3 camOffset;

    /* Conversion constants */
    const float DEGTORAD = 0.01745329251f;
    const float RADTODEG = 57.2957795131f;

    /* GUI */
    public Text txtAirspeed;
    public Text txtPitch;
    public Text txtAltitude;

    public Text txtAirspeedCtrl;
    public Text txtPitchCtrl;

    public Text txtAirspeedSP;
    public Text txtPitchSP;
    public Text txtAltitudeSP;

    public Button btnCSVLog;

    /* Aircraft dynamics */
    AircraftDynamics ac = new AircraftDynamics();

    /* Actuators */
    Actuator actEngine   = new Actuator(1.0f, 0.1f);
    Actuator actElevator = new Actuator(1.0f, 0.02f);

    /* Engine Model */
    static float[] engineThrottleLookup = new float[] { 0.0f, 10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f, 90.0f, 100.0f };
    static float[] engineThrustLookup   = new float[] { 0.0f, 0.10f, 0.40f, 0.50f, 0.55f, 0.55f, 0.70f, 0.75f, 0.75f, 1.25f,  1.50f };
    EngineModel engine = new EngineModel(engineThrottleLookup, engineThrustLookup, 11);

    /* Controllers */
    PIController piAirspeed = new PIController(25.0f, 2.0f, 0.0f, 100.0f);
    PIController piPitch = new PIController(0.75f, 1.4f, -30.0f * DEGTORAD, 30.0f * DEGTORAD);
    PIController piAltitude = new PIController(0.01f, 0.0f, -15.0f * DEGTORAD, 15.0f * DEGTORAD);

    float airspeedSetpoint = 15.0f;
    float pitchAngleSetpoint = 0.0f;
    float altitudeSetpoint = 110.0f;

    float throttleSetting;
    float engineThrust, elevatorDeflection;

    /* State estimator */
    Kalman kf = new Kalman(10.0f, 0.01f, 0.01f);

    /* Baro filter */
    FIR altFilter = new FIR(new float[]{
  -0.000482494968468395f,
  -0.0019575089731228134f,
  -0.00439051070353915f,
  -0.006253500850467823f,
  -0.004750578050044774f,
  0.00204723458734713f,
  0.012092704307359132f,
  0.01821085907653631f,
  0.01167074968316111f,
  -0.01012522145463007f,
  -0.03701090001536061f,
  -0.047214993120302536f,
  -0.01877513934152713f,
  0.05439974849336837f,
  0.15304635937830294f,
  0.23841936343727257f,
  0.2721898326120533f,
  0.23841936343727257f,
  0.15304635937830294f,
  0.05439974849336837f,
  -0.01877513934152713f,
  -0.047214993120302536f,
  -0.03701090001536061f,
  -0.01012522145463007f,
  0.01167074968316111f,
  0.01821085907653631f,
  0.012092704307359132f,
  0.00204723458734713f,
  -0.004750578050044774f,
  -0.006253500850467823f,
  -0.00439051070353915f,
  -0.0019575089731228134f,
  -0.000482494968468395f
}, 33);

    /* Timers */
    float totalTime;
    float timerSensors, timerCtrlAirspeed, timerCtrlAltitude, timerCtrlPitch, timerStateEstimation;

    /* CSV logger */
    CSVLogger csv = new CSVLogger();
    bool csvLogging = false;

    void Start()
    {
        /* Set simulation timestep */
        Time.fixedDeltaTime = 0.001f;

        /* Store camera to aircraft offset */
        camOffset = camTrans.position - trans.position;

        /* Initialise aircraft */
        ac.Init(initAirspeedmps, initPitchAngleRadians, initAltitudem);

        /* Initialise state estimator */
        kf.SetFilters(0.7f, 0.5f, 0.9f);

        /* Enable feedforward pitch control */
        piPitch.SetFF(1.5f, -5.0f * DEGTORAD, 5.0f * DEGTORAD);

        /* Reset timers */
        totalTime = timerSensors = timerCtrlAirspeed = timerCtrlAltitude = timerCtrlPitch = timerStateEstimation = 0.0f;

        /* Add GUI callbacks */
        btnCSVLog.onClick.AddListener(btnCSVLog_Click);
    }


    void FixedUpdate()
    {
        /* Get current sample time */
        float T = Time.deltaTime;
        totalTime += T;

        if (totalTime - timerSensors >= SAMPLE_TIME_SENSORS)
        {
            /* Read sensors and update state estimator */
            ac.GetSensorMeasurements();

            /* Filter barometer measurements */
            altFilter.Update(ac.baroAltitude);

            timerSensors = totalTime;
        }

        if (totalTime - timerStateEstimation >= SAMPLE_TIME_KALMAN)
        {
            float T_KF = totalTime - timerStateEstimation;

            /* Update state estimate */
            kf.Update(ac.gyroQ, ac.accX, ac.accZ, ac.pitotVa, T_KF);

            timerStateEstimation = totalTime;
        }

        /* Get controller outputs */
        if (totalTime - timerCtrlAirspeed >= SAMPLE_TIME_CTRL_AIRSPEED)
        {
            float T_CTRL_AIRSPEED = totalTime - timerCtrlAirspeed;

            throttleSetting = piAirspeed.Update(airspeedSetpoint, kf.VaFilt, T_CTRL_AIRSPEED);
            
            timerCtrlAirspeed = totalTime;
        }
        
        if (totalTime - timerCtrlAltitude >= SAMPLE_TIME_CTRL_ALTITUDE)
        {
            float T_CTRL_ALTITUDE = totalTime - timerCtrlAltitude;

            pitchAngleSetpoint = piAltitude.Update(altitudeSetpoint, altFilter.output, T_CTRL_ALTITUDE);

            timerCtrlAltitude = totalTime;
        }

        if (totalTime - timerCtrlPitch >= SAMPLE_TIME_CTRL_PITCH)
        {
            float T_CTRL_PITCH = totalTime - timerCtrlPitch;

            piPitch.Update(pitchAngleSetpoint, kf.theta, T_CTRL_PITCH);

            timerCtrlPitch = totalTime;
        }

        /* Calculate elevator deflection */
        elevatorDeflection = -actElevator.Update(piPitch.output, T);

        /* Calculate engine thrust */
        engineThrust = engine.GetThrust(actEngine.Update(throttleSetting, T));

        /* Update aircraft state vector */
        ac.Update(engineThrust, elevatorDeflection, T);

        /* Update 3D position and view */
        trans.eulerAngles = new Vector3(-ac.acState[3] * RADTODEG, 0.0f, 0.0f);
        trans.position = new Vector3(0.0f, ac.acState[5], ac.acState[4]);
        camTrans.position = trans.position + camOffset;

        /* Update GUI */
        UpdateGUI();

        /* Log data */
        if (csvLogging)
        {
            csv.AddLine(new string[] { totalTime.ToString("F6"),
                                       ac.acState[0].ToString("F6"), ac.acState[1].ToString("F6"), ac.acState[2].ToString("F6"), ac.acState[3].ToString("F6"), ac.acState[4].ToString("F6"), ac.acState[5].ToString("F6"),
                                       ac.GetAirspeed(ac.acState[0], ac.acState[1]).ToString("F6"), ac.GetAngleOfAttack(ac.acState[0], ac.acState[1]).ToString("F6"),
                                       kf.qFilt.ToString("F6"), kf.axFilt.ToString("F6"), kf.azFilt.ToString("F6"), kf.VaFilt.ToString("F6"), altFilter.output.ToString("F6"), kf.theta.ToString("F6"),
                                       elevatorDeflection.ToString("F6"), throttleSetting.ToString("F6"),
                                       airspeedSetpoint.ToString("F6"), pitchAngleSetpoint.ToString("F6"), altitudeSetpoint.ToString("F6")});
        }
    }

    /*
     * GUI functions
     */

    void UpdateGUI()
    {
        txtAirspeed.text = kf.VaFilt.ToString("F1") + " (" + ac.airspeed.ToString("F1") + ")";
        txtPitch.text = (kf.theta * RADTODEG).ToString("F1") + " (" + (ac.acState[3] * RADTODEG).ToString("F1") + ")";
        txtAltitude.text = altFilter.output.ToString("F1") + " (" + ac.acState[5].ToString("F1") + ")";

        txtAirspeedCtrl.text = throttleSetting.ToString("F1");
        txtPitchCtrl.text = (elevatorDeflection * RADTODEG).ToString("F1");

        txtAirspeedSP.text = airspeedSetpoint.ToString("F1");
        txtPitchSP.text = (pitchAngleSetpoint * RADTODEG).ToString("F1");
        txtAltitudeSP.text = altitudeSetpoint.ToString("F1");
    }

    void btnCSVLog_Click()
    {
        if (csvLogging)
        {
            btnCSVLog.GetComponentInChildren<Text>().text = "Start Logging";
            csv.WriteFile("log.csv");
        } else
        {
            btnCSVLog.GetComponentInChildren<Text>().text = "Stop Logging";
            csv.New(new string[] { "t", "u", "v", "q", "theta", "x", "h", "Va", "alpha", "gyrq", "accx", "accz", "pitotVa", "baroAlt", "thetahat", "de", "dt", "spAirspeed", "spPitch", "spAltitude" });
        }
        csvLogging = !csvLogging;
    }
}
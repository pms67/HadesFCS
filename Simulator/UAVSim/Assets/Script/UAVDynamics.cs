using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.IO;
using System.IO.Ports;

public class UAVDynamics : MonoBehaviour {

    // Serial (Hardware-in-the-loop)
    SerialPort hwController;
    string hwPort = "COM4";
    bool HILtest = false;    
        
    // GUI
    public Text txtLat; public Text txtLon; public Text txtAlt;
    public Text txtAirspeed;
    public Text txtAlpha; public Text txtBeta;
    public Text txtRoll; public Text txtPitch; public Text txtYaw;

    public Dropdown drpMode;
    public Dropdown drpState;

    public Slider sldAltitude; public Text txtAltSP;
    public Slider sldAirspeed; public Text txtAirspeedSP;

    // Global buttons
    public Button btnLogData;
    public Button btnLinearise;
    public Button btnControlParams;
    public Button btnAircraftParams;
    public Button btnInitCond;
    public Button btnPause;
    public Button btnTrim;
    public Button btnWind;
    public Button btnLoadWaypoints;

    // Control Parameters GUI
    public GameObject guiControlParams;

    // Trim GUI
    public GameObject guiTrim;
    public Button guiTrim_btnTrim, guiTrim_btnClose;
    public Slider guiTrim_north, guiTrim_east, guiTrim_alt;
    public Text guiTrim_txtNorth, guiTrim_txtEast, guiTrim_txtAlt;
    public Text guiTrim_txtTrimmedControls;
    public InputField guiTrim_inpAirspeed, guiTrim_inpFPAngle, guiTrim_inpRadius;

    // Initial Conditions GUI
    public GameObject guiInitCond;
    public Button guiInitCond_btnSet, guiInitCond_btnClose;
    public InputField guiInitCond_n, guiInitCond_e, guiInitCond_alt;
    public InputField guiInitCond_u, guiInitCond_v, guiInitCond_w;
    public InputField guiInitCond_p, guiInitCond_q, guiInitCond_r;
    public InputField guiInitCond_roll, guiInitCond_pitch, guiInitCond_yaw;
    public Dropdown guiInitCond_mode, guiInitCond_state;

    // Wind GUI
    public GameObject guiWind;
    public Button guiWind_btnSet, guiWind_btnClose;
    public Slider guiWind_sldNorth, guiWind_sldEast, guiWind_sldDown, guiWind_sldTurbulence;
    public Text guiWind_txtNorth, guiWind_txtEast, guiWind_txtDown, guiWind_txtTurbulence;
    public Toggle guiWind_tgEnable;

    // Mode and State
    public string mode;
    public string state;
    public string guiState = "RUNNING";

    // Record function
    public bool isRecording = false;
    public List<double[]> data;
    public int numData;
    public double dataDT;

    // Setpoints
    public double sp_airspeed = 0;
    public double sp_roll_angle = 0;
    public double sp_altitude = 0;
    public double sp_pitch_angle = 0;
    public double sp_sideslip = 0;

    // Waypoints
    private List<GameObject> waypointSpheres = new List<GameObject>();
    private LineRenderer waypointLines;
    private LineRenderer uavToWPLine; public GameObject UAVToWaypointLineRenderer;
    private List<double[]> waypoints;
    private int nWaypoints;
    private int curWaypoint;
    private double waypointTolerance; // How close do I have to be to a waypoint?
    private double waypointAirspeed;

    // Aircraft
    Assets.Script.Aircraft uav;

    // Transforms and Offsets
    private Vector3 offset;
    public Transform propTransform;

    // Controllers
    Assets.Script.PID ctr_Airspeed;
    Assets.Script.PID ctr_Pitch;
    Assets.Script.PID ctr_Altitude;
    Assets.Script.PID ctr_Sideslip;
    Assets.Script.PID ctr_Roll;

    // Control inputs
    double delta_t, delta_a, delta_e, delta_r;

    // Random noise generation
    private System.Random randGen = new System.Random();

    // Wind
    Assets.Script.Wind WindGenerator = new Assets.Script.Wind();
    double wind_field_north, wind_field_east, wind_field_down, wind_turbulence;
    bool wind_enabled;

    float phi_int = 0.0f;
    float p_int = 0.0f;

    void Start () {

        // Check if HIL testing
      //  hwController = new SerialPort(hwPort, 115200);
      //  hwController.DataReceived += new SerialDataReceivedEventHandler(HILDataReceived);
      //  hwController.Open();

        // Initialise GUI elements
        initGUI();
        initWaypoints();
        
        // Get initial offsets
        offset = transform.position;

        // Initialise aircraft and set initial conditions
        uav = new Assets.Script.Aircraft();
        uav.initState(new double[] { 0, 0, 100 }, new double[] { 0, 0, 0 }, new double[] { 0, 0, 0 });
        
        // Initialise controllers
        ctr_Airspeed = new Assets.Script.PID(0.01, 0.5, 0);
        ctr_Airspeed.setLimits(0, 1, 0);

        ctr_Pitch = new Assets.Script.PID(1, 0.01, 0); // Ziegler-Nichols
        ctr_Pitch.setLimits(-45, 45, 0); // Maximum elevator deflections

        ctr_Altitude = new Assets.Script.PID(1, 0, 0); // "Visual Inspection"
        ctr_Altitude.setLimits(-25, 25, 0); // Maximum pitch setpoints

        ctr_Sideslip = new Assets.Script.PID(0, 0, 0);
        ctr_Sideslip.setLimits(-10, 10, 0);

        ctr_Roll = new Assets.Script.PID(2.0, 0, 0.5); // Ziegler-Nichols
        ctr_Roll.setLimits(-45, 45, 0);

        // Initial setpoints
        sp_airspeed = 16.1;
        sp_altitude = 100;

        // Initial mode and state
        drpMode.value  = 1; mode  = "ASSISTED";
        drpState.value = 2; state = "CRUISE";

        // Initial wind settings
        wind_field_north = 0; wind_field_east = 0; wind_field_down = 0; wind_turbulence = 0; wind_enabled = false;
       
	}	

	void Update () {

        if (guiState == "RUNNING")
        {

            // Get sample time
            double dt = Time.deltaTime;

            // Perform actions in current state
            performMode(dt);

            // Pass control inputs to aircraft (deflections in DEGREES, however aircraft class works exclusively in RADIANS)
            uav.setControls(new double[] { delta_t, Mathf.Deg2Rad * delta_a, Mathf.Deg2Rad * delta_e, Mathf.Deg2Rad * delta_r });

            // Get wind vector
            double[] wind_vec = { 0, 0, 0 };
            if (wind_enabled)
            {
                double[] wind_field_vec = { wind_field_north, wind_field_east, wind_field_down };
                wind_vec = WindGenerator.getWind(wind_field_vec, new double[] { uav.phi, uav.theta, uav.psi }, uav.pd, uav.Va, wind_turbulence, dt);
            }

            // Update UAV simulation
            uav.update(dt, wind_vec);

            // Record data
            if (isRecording)
            {
                data.Add(new double[] {dataDT, delta_t, delta_a, delta_e, delta_r, uav.Va, Mathf.Rad2Deg * uav.alpha, Mathf.Rad2Deg * uav.beta, sp_airspeed, sp_roll_angle, sp_pitch_angle, sp_altitude, sp_sideslip,
                                       uav.pn, uav.pe, uav.pd, uav.u, uav.v, uav.w, Mathf.Rad2Deg * uav.phi, Mathf.Rad2Deg * uav.theta, Mathf.Rad2Deg * uav.psi, Mathf.Rad2Deg * uav.p, Mathf.Rad2Deg * uav.q, Mathf.Rad2Deg * uav.r});
                dataDT += dt; numData++;
            }

            // Update GUI measurements block
            GUI_updateMeasurements();

            // Update 3D world view
            transform.eulerAngles = new Vector3((float)(Mathf.Rad2Deg * -uav.theta), (float)(Mathf.Rad2Deg * uav.psi), (float)(Mathf.Rad2Deg * -uav.phi));
            transform.position = new Vector3((float)uav.pe, (float)uav.pd, (float)uav.pn) + offset;

            // Rotate propeller
            float propSpeed = Mathf.Pow((float)(delta_t * 80), 2.0f) * 0.25f;
            propTransform.Rotate(Vector3.back * propSpeed * Time.deltaTime);
                     

        }
                
    }

    // Mode Handlers
    void performMode(double dt)
    {

        if (mode == "MANUAL")
        {
            hideWaypoints();
            modeManual();
        }
        else if (mode == "ASSISTED")
        {
            hideWaypoints();
            modeAssisted(dt);
        }
        else // Autonomous mode
        {
            modeAutonomous(dt);
        }

    }

    private void modeManual()
    {
        // Thrust
        if (Input.GetKey(KeyCode.UpArrow))
        {
            delta_t += 0.1;

            if (delta_t > 1.0)
                delta_t = 1.0;
            else if (delta_t < 0.0)
                delta_t = 0.0;

        }
        else if (Input.GetKey(KeyCode.DownArrow))
        {
            delta_t -= 0.1;
        }

        // Aileron
        if (Input.GetKey(KeyCode.A))
        {
            delta_a += -0.1;
        }
        else if (Input.GetKey(KeyCode.D))
        {
            delta_a += 0.1;
        }

        // Elevator
        if (Input.GetKey(KeyCode.W))
        {
            delta_e += 0.1;
        }
        else if (Input.GetKey(KeyCode.S))
        {
            delta_e -= 0.1;
        }

        // Rudder
        if (Input.GetKey(KeyCode.Y))
        {
            delta_r += -0.1;
        }
        else if (Input.GetKey(KeyCode.X))
        {
            delta_r += 0.1;
        }

        return;
    }

    private void modeAssisted(double dt)
    {
        bool useMouse = true;

        if (useMouse) {

            // Airspeed
            if (Input.GetKey(KeyCode.UpArrow))
            {
                sp_airspeed += 1;
            }
            else if (Input.GetKey(KeyCode.DownArrow))
            {
                sp_airspeed -= 1;
            }

            // Roll
            if (Input.GetKey(KeyCode.A))
            {
                sp_roll_angle -= 5;
            }
            else if (Input.GetKey(KeyCode.D))
            {
                sp_roll_angle += 5;
            }

            // Pitch
            if (Input.GetKey(KeyCode.W))
            {   
                sp_pitch_angle -= 2.5;
            }
            else if (Input.GetKey(KeyCode.S))
            {
                sp_pitch_angle += 2.5;
            }

      //      Debug.Log("Airspeed: " + sp_airspeed.ToString() + " | Roll: " + sp_roll_angle.ToString()
        //                + " | Pitch: " + sp_pitch_angle.ToString() + " | Elevator: " + delta_e.ToString() + " | Aileron: " +
          //              delta_e.ToString());

        } else
        {

            sp_roll_angle = Input.GetAxis("Roll") * 25.0f;
            sp_pitch_angle = Input.GetAxis("Pitch") * 15.0f;
            sp_airspeed = (Input.GetAxis("Thrust") + 1.0f) * 40.0f;

        }

        // Check setpoint limits
        if (sp_airspeed > 70)
            sp_airspeed = 70;
        else if (sp_airspeed < 0)
            sp_airspeed = 0;

        if (sp_roll_angle > 45)
            sp_roll_angle = 45;
        else if (sp_roll_angle < -45)
            sp_roll_angle = -45;

        if (sp_pitch_angle > 25)
            sp_pitch_angle = 25;
        else if (sp_pitch_angle < -25)
            sp_pitch_angle = -25;

        // Sideslip is automatically corrected for!

        // Update controllers
        delta_t = ctr_Airspeed.update(sp_airspeed, uav.Va + 0.1 * randGen.NextDouble(), dt);
        delta_a = ctr_Roll.update(sp_roll_angle, Mathf.Rad2Deg * (uav.phi + 0.01 * randGen.NextDouble()), dt);
        delta_e = -ctr_Pitch.update(sp_pitch_angle, Mathf.Rad2Deg * (uav.theta + 0.01 * randGen.NextDouble()), dt);
        delta_r = ctr_Sideslip.update(sp_sideslip, Mathf.Rad2Deg * (uav.beta + 0.1 * randGen.NextDouble()), dt);

        return;
    }

    private void HILDataReceived(object sender, SerialDataReceivedEventArgs e)
    {

        SerialPort sp = (SerialPort) sender;
        string indata = sp.ReadExisting();

        if (indata.Equals("DATA"))
        {

            string data = "<" + uav.pd.ToString() + "," + uav.phi.ToString() + "," + uav.theta.ToString() + "," + uav.psi.ToString() + "," + 
                                uav.p.ToString() + "," + uav.q.ToString() + "," + uav.r.ToString() + "," +
                                uav.Va.ToString() + "," + uav.alpha.ToString() + "," + uav.beta.ToString() + ">";
            
            sp.WriteLine(data);

        }
        else
        {

            indata = indata.Replace("<", "").Replace(">", "");
            string[] cmds = indata.Split(',');

            delta_t = float.Parse(cmds[0]);
            delta_a = float.Parse(cmds[1]);
            delta_e = float.Parse(cmds[2]);
            delta_r = float.Parse(cmds[3]);

        }

    }

    private void modeAutonomous(double dt)
    {

        if (HILtest)
        {

        }
        else
        {

            if (state == "IDLE")
            {
                delta_t = 0; delta_a = 0; delta_e = 0; delta_r = 0;
            }
            else if (state == "TAKE OFF")
            {
                delta_t = 1.0;
            }
            else if (state == "CRUISE")
            {

                // Roll Angle
                if (Input.GetKey(KeyCode.A))
                {
                    sp_roll_angle -= 1;
                }
                else if (Input.GetKey(KeyCode.D))
                {
                    sp_roll_angle += 1;
                }

                // Update controllers
                sp_altitude = sldAltitude.value;
                sp_airspeed = sldAirspeed.value;

                sp_pitch_angle = ctr_Altitude.update(sp_altitude, uav.pd, dt);

                delta_t = ctr_Airspeed.update(sp_airspeed, uav.Va, dt);
                delta_a = ctr_Roll.update(sp_roll_angle, Mathf.Rad2Deg * uav.phi, dt);
                delta_e = -ctr_Pitch.update(sp_pitch_angle, Mathf.Rad2Deg * uav.theta, dt);
                delta_r = ctr_Sideslip.update(sp_sideslip, Mathf.Rad2Deg * uav.beta, dt);

            }
            else if (state == "LOITER")
            {

            }
            else if (state == "WAYPOINT")
            {
                Debug.Log("Waypoint");

                // Check if UAV has entered waypoint sphere
                double dist = System.Math.Sqrt(System.Math.Pow(uav.pe - waypoints[curWaypoint][0], 2) + System.Math.Pow(uav.pn - waypoints[curWaypoint][1], 2) + System.Math.Pow(uav.pd - waypoints[curWaypoint][2], 2));

                if (dist <= waypointTolerance)
                {
                    Debug.Log("Waypoint #" + (curWaypoint + 1) + " reached!");
                    waypointSpheres[curWaypoint].GetComponent<MeshRenderer>().material.SetColor("_Color", new Color(0, 1, 0, 0.5f)); // Set to "visited" colour
                    curWaypoint++;

                    if (curWaypoint == nWaypoints)
                    {
                        curWaypoint = 0;

                        for (int i = 0; i < nWaypoints; i++)
                            waypointSpheres[i].GetComponent<MeshRenderer>().material.SetColor("_Color", new Color(0, 0, 0, 0.5f));

                    }
                    else
                    {
                        waypointSpheres[curWaypoint].GetComponent<MeshRenderer>().material.SetColor("_Color", new Color(0, 1, 1, 0.5f));
                    }
                }

                // Update line connecting UAV to target waypoint
                uavToWPLine.SetPosition(0, transform.position);
                uavToWPLine.SetPosition(1, new Vector3((float)waypoints[curWaypoint][0], (float)waypoints[curWaypoint][2], (float)waypoints[curWaypoint][1]));

                // Calculate roll depending on angle (bearing) to waypoint (projected on to X-Y plane)
                double dx = waypoints[curWaypoint][0] - uav.pe;
                double dy = waypoints[curWaypoint][1] - uav.pn;
                double bearing = System.Math.Atan2(dx, dy) * Mathf.Rad2Deg - uav.psi * Mathf.Rad2Deg;

                sp_roll_angle = Mathf.Clamp((float)bearing, -45, 45);

                // Set altitude of waypoint
                sp_altitude = waypoints[curWaypoint][2];
                sp_pitch_angle = Mathf.Clamp((float) ctr_Altitude.update(sp_altitude, uav.pd, dt), -20, 20);

                // Set desired airspeed
                sp_airspeed = waypointAirspeed;

                // Sideslip we want to remain at 0°
                sp_sideslip = 0;

                // Get control inputs from controllers
                delta_t = ctr_Airspeed.update(sp_airspeed, uav.Va, dt);
                delta_a = ctr_Roll.update(sp_roll_angle, Mathf.Rad2Deg * uav.phi, dt);
                delta_e = ctr_Pitch.update(sp_pitch_angle, Mathf.Rad2Deg * uav.theta, dt);
                delta_r = ctr_Sideslip.update(sp_sideslip, Mathf.Rad2Deg * uav.beta, dt);

            }
            else if (state == "LAND")
            {

            }
            else
            {
                Debug.Log("INVALID STATE");
                drpState.value = 2;
                state = "CRUISE";
            }

        }

    }

    // Waypoint methods
    private void initWaypoints()
    {
        // Set up material and gradient of lines
        waypointLines = this.gameObject.AddComponent<LineRenderer>();
        waypointLines.material = new Material(Shader.Find("Sprites/Default"));
        waypointLines.startWidth = 0.25f; waypointLines.endWidth = 0.25f;
        Gradient waypointGradient = new Gradient();
        waypointGradient.SetKeys(new GradientColorKey[] { new GradientColorKey(Color.green, 0.0f), new GradientColorKey(Color.blue, 1.0f) }, new GradientAlphaKey[] { new GradientAlphaKey(0.5f, 0.0f), new GradientAlphaKey(0.5f, 1.0f) });
        waypointLines.colorGradient = waypointGradient;
        waypointLines.loop = true;
        waypointLines.enabled = false;

        // Create UAV to waypoint line
        uavToWPLine = UAVToWaypointLineRenderer.gameObject.AddComponent<LineRenderer>();
        uavToWPLine.material = new Material(Shader.Find("Sprites/Default"));
        uavToWPLine.startWidth = 0.05f; uavToWPLine.endWidth = 0.05f;
        Gradient UAVToWaypointGradient = new Gradient();
        UAVToWaypointGradient.SetKeys(new GradientColorKey[] { new GradientColorKey(Color.green, 0.0f), new GradientColorKey(Color.blue, 1.0f) }, new GradientAlphaKey[] { new GradientAlphaKey(0.5f, 0.0f), new GradientAlphaKey(0.5f, 1.0f) });
        uavToWPLine.colorGradient = UAVToWaypointGradient;
        uavToWPLine.enabled = false;

        // Initialise waypoint parameters
        curWaypoint = 0;
        nWaypoints = 0;
        waypointTolerance = 50;
        waypointAirspeed = 70;

        waypoints = new List<double[]>();


    }

    private void showWaypoints()
    {
        for (int i = 0; i < waypointSpheres.Count; i++)
            waypointSpheres[i].gameObject.SetActive(true);
        

        waypointLines.enabled = true;
        uavToWPLine.enabled = true;
    }

    private void hideWaypoints()
    {

        for (int i = 0; i < waypointSpheres.Count; i++)
            waypointSpheres[i].gameObject.SetActive(false);

        waypointLines.enabled = false;
        uavToWPLine.enabled   = false;
        
    }

    private void loadWaypoints()
    {
        
        string filename = UnityEditor.EditorUtility.OpenFilePanel("Load Waypoints", "", "");

        if (filename.Length > 0)
        {
            hideWaypoints();

            nWaypoints = 0;
            waypoints.Clear();

            double defaultAltitude = 150;

            try
            {

                System.IO.StreamReader file = new System.IO.StreamReader(filename);
                string line;
                
                while ((line = file.ReadLine()) != null)
                {

                    line = line.Replace(" ", "").ToUpper();

                    if (line.Contains("AIRSPEED="))
                    {
                        line = line.Replace("AIRSPEED=", "");
                        waypointAirspeed = System.Convert.ToDouble(line);
                    }
                    else if (line.Contains("ALTITUDE="))
                    {
                        line = line.Replace("ALTITUDE=", "");
                        defaultAltitude = System.Convert.ToDouble(line);
                    } else if (line.Contains("TOLERANCE="))
                    {
                        line = line.Replace("TOLERANCE=", "");
                        waypointTolerance = System.Convert.ToDouble(line);
                    }
                    else if (line.Contains("WAYPOINT="))
                    {
                        line = line.Replace("WAYPOINT=", "");
                        string[] line_split = line.Split(',');

                        double wpnorth = System.Convert.ToDouble(line_split[0]);
                        double wpeast = System.Convert.ToDouble(line_split[1]);
                        double wpalt = defaultAltitude;

                        if (line_split.Length == 3)
                            wpalt = System.Convert.ToDouble(line_split[2]);

                        waypoints.Add(new double[] { wpeast, wpnorth, wpalt }); // Waypoints are formatted X,Y,Z
                        nWaypoints++;

                    }

                }

                file.Close();

                // Delete old spheres
                for (int i = 0; i < waypointSpheres.Count; i++)
                    Destroy(waypointSpheres[i].gameObject);

                waypointSpheres.Clear();

                waypointLines.enabled = false;
                waypointLines.positionCount = nWaypoints;

                uavToWPLine.enabled = false;
                uavToWPLine.positionCount = 2;

                for (int i = 0; i < nWaypoints; i++)
                {

                    // Add spheres
                    waypointSpheres.Add(GameObject.CreatePrimitive(PrimitiveType.Sphere));
                    waypointSpheres[i].transform.localScale = new Vector3((float)waypointTolerance, (float)waypointTolerance, (float)waypointTolerance);

                    Vector3 wpPos = new Vector3((float)waypoints[i][0], (float)waypoints[i][2], (float)waypoints[i][1]); // Since order is: pe (x), pd (z), pn (y) in transform!
                    waypointSpheres[i].transform.position = wpPos;

                    Material wpSphMat = waypointSpheres[i].GetComponent<MeshRenderer>().material;
                    wpSphMat.SetFloat("_Mode", 3.0f);
                    wpSphMat.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.SrcAlpha);
                    wpSphMat.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.OneMinusSrcAlpha);
                    wpSphMat.SetInt("_ZWrite", 0);
                    wpSphMat.DisableKeyword("_ALPHATEST_ON");
                    wpSphMat.EnableKeyword("_ALPHABLEND_ON");
                    wpSphMat.DisableKeyword("_ALPHAPREMULTIPLY_ON");
                    wpSphMat.renderQueue = 3000;
                    wpSphMat.SetColor("_Color", new Color(0, 0, 0, 0.5f));
                    Instantiate(wpSphMat);

                    // Add connecting lines
                    waypointLines.SetPosition(i, wpPos);

                    waypointSpheres[i].gameObject.SetActive(false);

                }

            } catch (System.Exception e) {

                Debug.Log("Error occured while trying to load waypoints from file (" + e.ToString() + ").");
                return;

            }

            Debug.Log("Succesfully read waypoint file containing " + nWaypoints + " waypoints.");

        }

    }

    // GUI Handlers
    private void initGUI()
    {
        // Initialise GUI listeners
        btnLogData.onClick.AddListener(GUI_logData);
        btnLinearise.onClick.AddListener(delegate { GUI_linearise(); });
        btnAircraftParams.onClick.AddListener(GUI_aircraftparams);
        btnControlParams.onClick.AddListener(GUI_control_params);
        btnPause.onClick.AddListener(GUI_pause);
        btnLoadWaypoints.onClick.AddListener(loadWaypoints);
        drpMode.onValueChanged.AddListener(delegate { GUI_mode(); });
        drpState.onValueChanged.AddListener(delegate { GUI_state(); });
        sldAltitude.onValueChanged.AddListener(delegate { sp_altitude = sldAltitude.value; txtAltSP.text = sp_altitude + "m"; });
        sldAirspeed.onValueChanged.AddListener(delegate { sp_airspeed = sldAirspeed.value; txtAirspeedSP.text = sp_airspeed + "m/s"; });

        // GUI: Trim
        btnTrim.onClick.AddListener(GUI_trim);
        guiTrim_btnClose.onClick.AddListener(delegate { handleGUI_trim(0); });
        guiTrim_btnTrim.onClick.AddListener(delegate { handleGUI_trim(1); });
        guiTrim_north.onValueChanged.AddListener(delegate { handleGUI_trim(2); });
        guiTrim_east.onValueChanged.AddListener(delegate { handleGUI_trim(3); });
        guiTrim_alt.onValueChanged.AddListener(delegate { handleGUI_trim(4); });

        // GUI: Initial Conditions
        btnInitCond.onClick.AddListener(GUI_init_cond);
        guiInitCond_btnSet.onClick.AddListener(delegate { handleGUI_initcond(0); });
        guiInitCond_btnClose.onClick.AddListener(delegate { handleGUI_initcond(1); });

        // GUI: Wind
        btnWind.onClick.AddListener(GUI_wind);
        guiWind_btnClose.onClick.AddListener(delegate { handleGUI_wind(0); });
        guiWind_btnSet.onClick.AddListener(delegate { handleGUI_wind(1); });
        guiWind_sldNorth.onValueChanged.AddListener(delegate { guiWind_txtNorth.text = nf(guiWind_sldNorth.value, 1); });
        guiWind_sldEast.onValueChanged.AddListener(delegate { guiWind_txtEast.text = nf(guiWind_sldEast.value, 1); });
        guiWind_sldDown.onValueChanged.AddListener(delegate { guiWind_txtDown.text = nf(guiWind_sldDown.value, 1); });
        guiWind_sldTurbulence.onValueChanged.AddListener(delegate { guiWind_txtTurbulence.text = nf(guiWind_sldTurbulence.value, 1); });
        
    }

    private void GUI_linearise()
    {
        string destination = UnityEditor.EditorUtility.SaveFilePanel("Save Log File", "", "Linearised System.txt", "txt");

        if (destination.Length == 0)
        {
            UnityEditor.EditorUtility.DisplayDialog("Error", "Invalid filename or filepath selected.", "Ok");
            return;
        }

        uav.linearise(destination);
    }

    private void GUI_logData()
    {

        if (isRecording)
        {

            string destination = UnityEditor.EditorUtility.SaveFilePanel("Save Log File", "", "FlightData.csv", "csv");

            if (destination.Length == 0)
            {
                UnityEditor.EditorUtility.DisplayDialog("Error", "Invalid filename or filepath selected.", "Ok");
                return;
            }


            if (File.Exists(destination))
            {
                File.Delete(destination);
                Debug.Log("File already exists. Overwriting...");
            }

            StreamWriter sw = File.CreateText(destination);

            sw.WriteLine("t,delta_t,delta_a,delta_e,delta_r,Va,alpha,beta,sp_airspeed,sp_roll_angle,sp_pitch_angle,sp_altitude,sp_sideslip,pn,pe,pd,u,v,w,phi,theta,psi,p,q,r");

            for (int i = 0; i < numData; i++)
            {
                string s = data[i][0].ToString();

                for (int j = 1; j < data[i].GetLength(0); j++)
                    s += "," + data[i][j].ToString();

                sw.WriteLine(s);
            }


            sw.Close();

            btnLogData.GetComponentInChildren<Text>().text = "Log Flight Data";

            Debug.Log("Flight data saved!");


        }
        else
        {

            data = new List<double[]>();
            numData = 0; dataDT = 0;

            btnLogData.GetComponentInChildren<Text>().text = "Stop Logging";

            Debug.Log("Recording flight data...");

        }

        isRecording = !isRecording;

    }

    private void GUI_aircraftparams()
    {
        if (guiState == "RUNNING")
        {
            string paramfile = UnityEditor.EditorUtility.OpenFilePanel("Load Aircraft Parameters", "", "");

            if (paramfile.Length > 0)
            {
                int numParams = uav.loadAircraftParams(paramfile);
                Debug.Log("Loaded " + numParams + " aircraft parameters.");
            }

        }
    }

    private void GUI_control_params()
    {
        if (guiState == "CONTROL")
        {
            guiControlParams.SetActive(false);
            guiState = "RUNNING";
        }
        else if (guiState == "RUNNING")
        {
            guiControlParams.SetActive(true);
            guiState = "CONTROL";
        }
    }

    private void GUI_init_cond()
    {
        if (guiState == "RUNNING")
        {
            guiInitCond.SetActive(true);
            guiState = "INITCOND";
        }
        else if (guiState == "INITCOND")
        {
            guiInitCond.SetActive(false);
            guiState = "RUNNING";
        }
    }

    private void GUI_pause()
    {
        if (guiState == "RUNNING")
        {
            btnPause.GetComponentInChildren<Text>().text = "Continue";
            guiState = "PAUSE";
        }
        else if (guiState == "PAUSE")
        {
            btnPause.GetComponentInChildren<Text>().text = "Pause";
            guiState = "RUNNING";
        }
    }

    private void GUI_trim()
    {
        if (guiState == "RUNNING")
        {
            guiTrim.SetActive(true);
            guiState = "TRIM";
        }
        else if (guiState == "TRIM")
        {
            guiTrim.SetActive(false);
            guiState = "RUNNING";
        }

    }

    private void GUI_wind()
    {
        if (guiState == "RUNNING")
        {
            guiWind.SetActive(true);
            guiState = "WIND";
        }
        else if (guiState == "WIND")
        {
            guiWind.SetActive(false);
            guiState = "RUNNING";
        }
    }

    private void GUI_mode()
    {
        switch (drpMode.value)
        {
            case 0:
                mode = "AUTONOMOUS";

                hideWaypoints();

                drpState.enabled = true;
                sldAltitude.enabled = true;
                sldAirspeed.enabled = true;
                break;

            case 1:
                mode = "ASSISTED";

                hideWaypoints();

                drpState.enabled = false;
                sldAltitude.enabled = false;
                sldAirspeed.enabled = false;
                break;

            case 2:
                mode = "MANUAL";

                hideWaypoints();

                drpState.enabled = false;
                sldAltitude.enabled = false;
                sldAirspeed.enabled = false;
                break;
        }

        GUI_state();

    }

    private void GUI_state()
    {
        switch (drpState.value)
        {
            case 0:
                hideWaypoints();

                state = "IDLE";
                break;

            case 1:
                hideWaypoints();

                state = "TAKE OFF";
                break;

            case 2:
                hideWaypoints();

                state = "CRUISE";
                break;

            case 3:
                hideWaypoints();

                state = "LOITER";
                break;

            case 4:

                if (waypoints.Count > 0)
                {

                    curWaypoint = 0;
                    showWaypoints();

                    // Set first waypoint to "target" colour
                    waypointSpheres[0].GetComponent<MeshRenderer>().material.SetColor("_Color", new Color(0, 1, 1, 0.5f));

                    state = "WAYPOINT";

                } else
                {
                    drpState.value = 2;
                    state = "CRUISE";
                }

                break;

            case 5:
                hideWaypoints();

                state = "LAND";
                break;
        }
    }

    public void GUI_updateMeasurements()
    {
        // Update measurement info
        txtLat.text = "Latitude: " + nf(uav.pn, 1); txtLon.text = "Longitude: " + nf(uav.pe, 1); txtAlt.text = "Altitude: " + nf(uav.pd, 1) + "m";
        txtAirspeed.text = "Airspeed: " + nf(uav.Va, 1) + "m/s";
        txtAlpha.text = "A.o.A.: " + nf(Mathf.Rad2Deg * uav.alpha, 1) + "°"; txtBeta.text = "Sideslip: " + nf(Mathf.Rad2Deg * uav.beta, 1) + "°";
        txtRoll.text = "Roll: " + nf(Mathf.Rad2Deg * uav.phi, 1) + "°"; txtPitch.text = "Pitch: " + nf(Mathf.Rad2Deg * uav.theta, 1) + "°"; txtYaw.text = "Yaw: " + nf(Mathf.Rad2Deg * uav.psi, 1) + "°";
    }

    private void handleGUI_control_params()
    {
        // TO DO
    }

    private void handleGUI_initcond(int ID)
    {

        switch (ID)
        {
            case 0: // Set

                uav.resetState();
                uav.pn = System.Convert.ToDouble(guiInitCond_n.text);
                uav.pe = System.Convert.ToDouble(guiInitCond_e.text);
                uav.pd = System.Convert.ToDouble(guiInitCond_alt.text);
                uav.u = System.Convert.ToDouble(guiInitCond_u.text);
                uav.v = System.Convert.ToDouble(guiInitCond_v.text);
                uav.w = System.Convert.ToDouble(guiInitCond_w.text);
                uav.p = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_p.text);
                uav.q = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_q.text);
                uav.r = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_r.text);

                double phi_ = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_roll.text);
                double theta_ = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_pitch.text);
                double psi_ = Mathf.Deg2Rad * System.Convert.ToDouble(guiInitCond_yaw.text);

                // Convert to quaternions
                uav.e0 = uav.cos(psi_ / 2) * uav.cos(theta_ / 2) * uav.cos(phi_ / 2) + uav.sin(psi_ / 2) * uav.sin(theta_ / 2) * uav.sin(phi_ / 2);
                uav.e1 = uav.cos(psi_ / 2) * uav.cos(theta_ / 2) * uav.sin(phi_ / 2) - uav.sin(psi_ / 2) * uav.sin(theta_ / 2) * uav.cos(phi_ / 2);
                uav.e2 = uav.cos(psi_ / 2) * uav.sin(theta_ / 2) * uav.cos(phi_ / 2) + uav.sin(psi_ / 2) * uav.cos(theta_ / 2) * uav.sin(phi_ / 2);
                uav.e3 = uav.sin(psi_ / 2) * uav.cos(theta_ / 2) * uav.cos(phi_ / 2) - uav.cos(psi_ / 2) * uav.sin(theta_ / 2) * uav.sin(phi_ / 2);

                state = guiInitCond_mode.captionText.text.ToUpper();
                mode = guiInitCond_state.captionText.text.ToUpper();

                drpState.value = guiInitCond_state.value;
                drpMode.value = guiInitCond_mode.value;

                // Set control inputs to zero
                uav.delta_t = 0; uav.delta_a = 0; uav.delta_e = 0; uav.delta_r = 0;

                guiState = "RUNNING";
                guiInitCond.SetActive(false);

                break;

            case 1: // Close

                guiState = "RUNNING";
                guiInitCond.SetActive(false);

                break;
        }

    }

    private void handleGUI_trim(int ID)
    {

        switch (ID)
        {
            case 0:
                guiTrim.SetActive(false);
                guiState = "RUNNING";
                break;
            case 1:
                double north = guiTrim_north.value;
                double east = guiTrim_east.value;
                double altitude = guiTrim_alt.value;
                double airspeed = System.Convert.ToDouble(guiTrim_inpAirspeed.text);
                double fpangle = System.Convert.ToDouble(guiTrim_inpFPAngle.text);
                double radius = System.Convert.ToDouble(guiTrim_inpRadius.text);

                // Trim
                try
                {

                    double guess_alpha = 0; double guess_beta = 0; double guess_phi = 0;


                    double[] trimmed = uav.trim(guess_alpha, guess_beta, guess_phi, airspeed, radius, Mathf.Deg2Rad * fpangle);

                    // Set controls         
                    delta_t = trimmed[0]; delta_a = Mathf.Rad2Deg * trimmed[1]; delta_e = Mathf.Rad2Deg * trimmed[2]; delta_r = Mathf.Rad2Deg * trimmed[3];

                    // Set position
                    uav.pn = north; uav.pe = east; uav.pd = altitude;

                    // Display trimmed state
                    guiTrim_txtTrimmedControls.text = "Thrust=" + nf(uav.delta_t, 4) + ",Aileron=" + nf(uav.delta_a, 4) + ",Elevator=" + nf(uav.delta_e, 4) + "°,Rudder=" + nf(uav.delta_r, 4) + "°";


                }
                catch (System.Exception e)
                {

                    guiTrim_txtTrimmedControls.text = "Error calculating trim conditions (" + e.ToString() + ").";

                }

                break;
            case 2:

                guiTrim_txtNorth.text = guiTrim_north.value.ToString();
                break;
            case 3:

                guiTrim_txtEast.text = guiTrim_east.value.ToString();
                break;
            case 4:

                guiTrim_txtAlt.text = guiTrim_alt.value.ToString();
                break;
            default:

                Debug.Log("Trim GUI error.");
                break;
        }
    }
    
    private void handleGUI_wind(int ID) {

        switch (ID)
        {
            case 0:
                guiWind.SetActive(false);
                guiState = "RUNNING";
                break;

            case 1:
                wind_field_north = guiWind_sldNorth.value;
                wind_field_east = guiWind_sldEast.value;
                wind_field_down = guiWind_sldDown.value;
                wind_turbulence = guiWind_sldTurbulence.value;
                wind_enabled = guiWind_tgEnable.isOn;

                guiWind.SetActive(false);
                guiState = "RUNNING";
                break;
        }

    }

    public string nf(double val, int dp)
    {
        switch (dp)
        {
            case 0:
                return System.Convert.ToInt32(val).ToString();
            case 1:
                return val.ToString("F1");
            case 2:
                return val.ToString("F2");
            case 3:
                return val.ToString("F3");
            case 4:
                return val.ToString("F4");
            default:
                return val.ToString();
        }
    }

    public void printArray(double[,] arr)
    {

        string s = "[";

        for (int i = 0; i < arr.GetLength(0); i++)
        {

            s += "";

            for (int j = 0; j < arr.GetLength(1); j++)
            {
                s += " " + arr[i, j] + " ";
            }

            s += "; ... \n";

        }

        s += "];";

        Debug.Log(s);
                
    }

    private double randGaussian(double mean, double stdDev)
    {
        double u1 = 1 - randGen.NextDouble();
        double u2 = 1 - randGen.NextDouble();

        double randStdNormal = System.Math.Sqrt(-2 * System.Math.Log(u1)) * System.Math.Sin(2 * System.Math.PI * u2);

        return mean + stdDev * randStdNormal;
    }
        
}
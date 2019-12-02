% Generate UAV Sensor Measurements from Truth Data
%
% Use random noise and biases to generate sensor measurements from the
% truth data.  See Reference 1 for mathematical derivations.
%
% Inputs:  uavTruth structure (edit mfile for more information)
%
% Outputs: uavSensors structure (edit mfile for more information)
%
% References:
%   1  Barton, J. D., “Fundamentals of Small Unmanned Aircraft Flight,” 
%      Johns Hopkins APL Technical Digest, Volume 31, Number 2 (2012).
%      http://www.jhuapl.edu/techdigest/TD/td3102/
% 
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

% Inputs (uavTruth structure):
%     uavTruth = 
%                 time_s: [Kx1 double] Time, s
%                 east_m: [Kx1 double] East position, m
%                north_m: [Kx1 double] North position, m
%                h_msl_m: [Kx1 double] Height above mean-sea-level, m
%              v_ned_mps: [Kx3 double] North-East-Down (NED) velocity vector, m/s
%                yaw_deg: [Kx1 double] Yaw angle, deg, positive East-of-North
%              pitch_deg: [Kx1 double] Pitch angle, deg, positive above horizontal
%               roll_deg: [Kx1 double] Roll angle, deg, positive banking right
%                 wb_rps: [Kx3 double] Body attitude rate vector, radians/s
%         v_wind_ned_mps: [Kx3 double] NED velocity vector of wind, m/s
%                 temp_K: [Kx1 double] Air temperature, Kelvin
%          pressure_Npm2: [Kx1 double] Air pressure, Newtons/m^2
%          density_kgpm3: [Kx1 double] Air density, kg/m^3

% Outputs (uavSensors structure)
%     uavSensors = 
%                           time_s: [Kx1 double]  Time, s
%                       GPS_east_m: [Kx1 double]  GPS-measured east position, m
%                      GPS_north_m: [Kx1 double]  GPS-measured north position, m
%                      GPS_h_msl_m: [Kx1 double]  GPS-measured height above mean-sea-level, m
%                    GPS_v_ned_mps: [Kx3 double]  GPS-measured NED velocity vector, m/s
%                        GPS_valid: [Kx1 logical] GPS validity flag, true when GPS is valid
%               pitot_airspeed_mps: [Kx1 double]  Pitot-sensor-based airspeed, m/s
%                      gyro_wb_rps: [Kx3 double]  Gyro-based body attitude rate vector, radians/s
%                    accel_fb_mps2: [Kx3 double]  Accelerometer-based specific force vector (accerations), m/s^2
%         mag3D_unitVector_in_body: [Kx3 double]  3D-magnetometer measurement: unit vector in body coords. pointing to Mag. North
%                    mag2D_yaw_deg: [Kx1 double]  2D-magnetometer measurement: body yaw angle (from true north), deg
%                           sigmas: [struct]      Structure containing utilized sensor noise and bias standard deviations (sigmas)
%                           biases: [struct]      Structure containing utilized sensor biases
%                           params: [struct]      Structure containing utilized parameters (see below)
%     uavSensors.params =
%                         dt_GPS_s: [1x1 double] Time between GPS measurements, s
%              mag_declination_deg: [1x1 double] Magnetic declination angle, deg
%

% Make sure uavTruth exists
if ~exist('uavTruth')
    error(['This routine is meant to be run using the uavTruth structure. ' ...
          'Try running the main routine ''uav_state_estimation.m''.']) 
end

% Constants
gravity_mps2 = 9.81;

% Initialize uavSensors structure with time
uavSensors=[];
uavSensors.sigmas=[];
uavSensors.biases=[];
uavSensors.params=[];
uavSensors.time_s = uavTruth.time_s;

% Establish sensor noise parameters
% Note: The numbers provided do not reflect errors in any specific sensor.
%       In fact, I made them all up.
uavSensors.sigmas.gyro_noise_rps            = 0.005;     % Gyro angular rate noise and bias, rad/s
uavSensors.sigmas.gyro_bias_rps             = 0.01;     %
uavSensors.sigmas.accel_noise_mps2          = 0.05;      % Accelerometer noise and bias, m/s^2
uavSensors.sigmas.accel_bias_mps2           = 0.1;      %
uavSensors.sigmas.pitot_noise_mps           = 0.5;      % Pitot-tube-based airspeed noise and bias, m/s
uavSensors.sigmas.pitot_bias_mps            = 1;        %
uavSensors.sigmas.GPSpos_noise_m            = 2;        % GPS position noise and bias, m
uavSensors.sigmas.GPSpos_bias_m             = 0;        %
uavSensors.sigmas.GPSvel_noise_mps          = 1;        % GPS velocity noise and bias, m/s
uavSensors.sigmas.GPSvel_bias_mps           = 0;        %
uavSensors.sigmas.mag3D_unitVector_noise    = 0.02;     % 3D Magnetometer unit vector noise and bias
uavSensors.sigmas.mag3D_unitVector_bias     = 0.05;     %

% Establish GPS update rate
uavSensors.params.dt_GPS_s = 1.0;  % Number of seconds between GPS updates

% GPS Position and Velocity
% Method:
% - Randomly determine sensor biases
% - Add noise and biases to true position to form GPS position measurements
% - Loop through North-East-Down components to add noise and biases to 
%   true velocities to form GPS velocity measurements
uavSensors.biases.GPS_east_m    = uavSensors.sigmas.GPSpos_bias_m*randn;
uavSensors.biases.GPS_north_m   = uavSensors.sigmas.GPSpos_bias_m*randn;
uavSensors.biases.GPS_h_m       = uavSensors.sigmas.GPSpos_bias_m*randn;
uavSensors.biases.GPS_Vned_mps= uavSensors.sigmas.GPSvel_bias_mps*randn(1,3);
uavSensors.GPS_east_m = uavTruth.east_m ... 
                            + uavSensors.biases.GPS_east_m ...
                            + uavSensors.sigmas.GPSpos_noise_m*randn(size(uavTruth.time_s));
uavSensors.GPS_north_m = uavTruth.north_m ...
                            + uavSensors.biases.GPS_north_m ...
                            + uavSensors.sigmas.GPSpos_noise_m*randn(size(uavTruth.time_s));
uavSensors.GPS_h_msl_m = uavTruth.h_msl_m ... 
                            + uavSensors.biases.GPS_h_m ...
                            + uavSensors.sigmas.GPSpos_noise_m*randn(size(uavTruth.time_s));
for m=[1 2 3] % 1: North, 2: East, 3: Down                        
    uavSensors.GPS_v_ned_mps(:,m) = uavTruth.v_ned_mps(:,m) ...
                                + uavSensors.biases.GPS_Vned_mps(m) ...
                                + uavSensors.sigmas.GPSvel_noise_mps*randn(size(uavTruth.time_s));
end

% GPS is only valid at a rate of 1/dt_GPS_s
% - Uses a "nearest-neighbor" interpolation to determine GPS validity times
% - GPS_valid=1 if GPS is valid, 0 otherwise.  
% - GPS values are set to NaN if not valid.
kGPS = interp1(uavTruth.time_s,1:length(uavTruth.time_s),uavTruth.time_s(1):uavSensors.params.dt_GPS_s:uavTruth.time_s(end),'nearest');
uavSensors.GPS_valid = false(size(uavSensors.time_s)); % \  Initialze GPS_valid to a vector of "false" values.
uavSensors.GPS_valid(kGPS)=true;                       % /  Set GPS_valid to "true" for GPS update times.
uavSensors.GPS_east_m(~uavSensors.GPS_valid)      = NaN; % \ 
uavSensors.GPS_north_m(~uavSensors.GPS_valid)     = NaN; %  | GPS values are set to NaN
uavSensors.GPS_h_msl_m(~uavSensors.GPS_valid)     = NaN; %  |   if not valid
uavSensors.GPS_v_ned_mps(~uavSensors.GPS_valid,:) = NaN; % /

% Pitot-measured Airspeed
% Assuming the pitot tube is aligned with the UAV body-x direction (axial),
% the pitot airspeed is a measure of the wind-relative UAV axial speed.
% Thus, we first need to generate the wind-relative velocity vector
% in body coordinates, Vrel_b_mps.  Axial airspeed is the x-component
% of Vrel_b_mps.
% Note: We simply use a truth+bias+noise model here for airspeed.
%   Alternative, one could use Equation 6 in Reference 1 to derive airspeed
%   via the atmospheric data provided in uavTruth.
% Method:
% - Randomly determine sensor biases
% - Loop through time to build airspeed measurements
uavSensors.biases.pitot_airspeed_mps = uavSensors.sigmas.pitot_bias_mps*randn;
for kTime=1:length(uavTruth.time_s)
    
    % Extract needed variables for time index kTime
    yaw_rad        = pi/180*uavTruth.yaw_deg(kTime);
    pitch_rad      = pi/180*uavTruth.pitch_deg(kTime);
    roll_rad       = pi/180*uavTruth.roll_deg(kTime);
    v_ned_mps      = uavTruth.v_ned_mps(kTime,:)';
    v_wind_ned_mps = uavTruth.v_wind_ned_mps(kTime,:)';

    % Direction Cosine Matrix (DCM) from NED coordinates to body 
    % coordinates expressed using Euler angles.
    % See rotation_examples.m for more information about converting between
    % different rotation representations.  Here we use the method in
    % Section B of rotation_examples.m to convert from Euler angles to DCM.
    C_ned2b  = [cos(pitch_rad)*cos(yaw_rad)                                             cos(pitch_rad)*sin(yaw_rad)                                           -sin(pitch_rad); ...
                sin(roll_rad)*sin(pitch_rad)*cos(yaw_rad)-cos(roll_rad)*sin(yaw_rad)    sin(roll_rad)*sin(pitch_rad)*sin(yaw_rad)+cos(roll_rad)*cos(yaw_rad)   sin(roll_rad)*cos(pitch_rad); ...
                cos(roll_rad)*sin(pitch_rad)*cos(yaw_rad)+sin(roll_rad)*sin(yaw_rad)    cos(roll_rad)*sin(pitch_rad)*sin(yaw_rad)-sin(roll_rad)*cos(yaw_rad)   cos(roll_rad)*cos(pitch_rad)];
    
    % Generate the wind-relative velocity vector in body coordinates
    % (Converts NED wind-relative velocity vector into body coords)
    Vrel_b_mps   = C_ned2b*(v_ned_mps - v_wind_ned_mps);
    
    % Generate pitot-measured airspeed by adding noise to
    % true axial airspeed 
    % - Axial airspeed is the x-component of Vrel_b_mps: Vrel_b_mps(1)
    uavSensors.pitot_airspeed_mps(kTime,1) = Vrel_b_mps(1) ...
                            + uavSensors.biases.pitot_airspeed_mps ...
                            + uavSensors.sigmas.pitot_noise_mps*randn;
end

% IMU Gyro Angular Rate (wb) Measurements, body coordinates
% Method:
% - Randomly determine sensor biases
% - Loop through body x-y-z components to add noise and biases to 
%   true angular rates to form measurements (radians/second)
uavSensors.biases.gyro_wb_rps = uavSensors.sigmas.gyro_bias_rps*randn(1,3);
for m=[1 2 3] % 1: body-x, 2: body-y, 3: body-z                        
    uavSensors.gyro_wb_rps(:,m) = uavTruth.wb_rps(:,m) ...
                            + uavSensors.biases.gyro_wb_rps(m) ...
                            + uavSensors.sigmas.gyro_noise_rps*randn(size(uavTruth.time_s));
end

% IMU Accelerometer Specific Force (fb) Measurements, body coordinates.
% Accelerometers measure "Specific Force", which is actually the total 
% acceleration relative to free-fall.  Assuming the IMU is near the UAV
% center-of-gravity and neglecting earth rotation effects, the Specific Force
% can be mathematically represented in inertial NED coordinates (f_ned) as 
% a vector combination of NED translational acceleration [d(v_ned_mps)/dt] 
% and gravity, as in equation (a):
%
%                d /     \    [      0       ]
%  (a)  f_ned = --( v_ned ) - [      0       ]
%               dt \     /    [ gravity_mps2 ]
%
%
% But, the IMU is rigidly attached to the rotating body coordinate frame,
% so it actually measures f_b=C_ned2b*f_ned, as in equation (b):
%
%                      d /     \            [      0       ]
%  (b)  f_b = C_ned2b*--( v_ned ) - C_ned2b*[      0       ]
%                     dt \     /            [ gravity_mps2 ]
%
% Equation (b) is exactly how the accelerometer measurement is generated
% below.  But, the described state estimation methods make use of the
% equivalent formulation in equation (c), which is also Equation (10) in
% Reference 1:
%
%                               d /   \            [      0       ]
%  (c)  f_b = cross(w_b,v_b) + --( v_b ) - C_ned2b*[      0       ]
%                              dt \   /            [ gravity_mps2 ]
%
% Equation (c) is derived from equation (b) in the following manner:
%
%    First, replace v_ned with C_b2ned*v_b:
%
%                      d /           \            [      0       ]
%       f_b = C_ned2b*--( C_b2ned*v_b ) - C_ned2b*[      0       ]
%                     dt \           /            [ gravity_mps2 ]
%
%    Next, use the derivative chain rule:
%
%                     [ d /       \                 d /   \ ]           [      0       ]
%       f_b = C_ned2b*[--( C_b2ned )*v_b + C_b2ned*--( v_b )] - C_ned2b*[      0       ]
%                     [dt \       /                dt \   / ]           [ gravity_mps2 ]
%
%    Then, note that the time derivative of C_b2ned is C_b2ned*[~w_b], where
%    [~w_b] is the skew-symmetric matrix formed from w_b (the angular rate
%    vector of the body relative to NED, given in body coordinates):
%
%                     [                              d /   \ ]           [      0       ]
%       f_b = C_ned2b*[C_b2ned*[~w_b]*v_b + C_b2ned*--( v_b )] - C_ned2b*[      0       ]
%                     [                             dt \   / ]           [ gravity_mps2 ]
%
%       where:          [   0    -w_b(3)  w_b(2)]
%              [~w_b] = [ w_b(3)    0    -w_b(1)]
%                       [-w_b(2)  w_b(1)    0   ]
%
%    Noting that C_ned2b*C_b2ned is equal to the identity matrix yields:
%
%                           d /   \            [      0       ]
%       f_b = [~w_b]*v_b + --( v_b ) - C_ned2b*[      0       ]
%                          dt \   /            [ gravity_mps2 ]
%
%    Finally, noting that [~w_b]*v_b = cross(w_b,v_b) we result in equation
%    (c) from above.
%
% Using equation (b) above, form the accelerometer specific force vector
% (accel_fb_mps2) by combining differentiated NED velocities (vdot_ned_mps2)
% with the gravity vector, and rotating the result into body coordinates.
%
% Method:
% - Randomly determine sensor biases
% - Differentiate NED velocities (using gradient) to compute NED inertial 
%   accelerations: vdot_ned_mps2 
% - Loop through time to build accelerometer specific force measurements
uavSensors.biases.accel_fb_mps2 = uavSensors.sigmas.accel_bias_mps2*randn(1,3);
vdot_n_mps2 = gradient(uavTruth.v_ned_mps(:,1),uavTruth.time_s);
vdot_e_mps2 = gradient(uavTruth.v_ned_mps(:,2),uavTruth.time_s);
vdot_d_mps2 = gradient(uavTruth.v_ned_mps(:,3),uavTruth.time_s);
vdot_ned_mps2 = [vdot_n_mps2 vdot_e_mps2 vdot_d_mps2]; % [nx3 double]
for kTime=1:length(uavTruth.time_s)

    % Compute NED-to-body Direction Cosine Matrix for time index kTime.
    % See rotation_examples.m for more information about converting between
    % different rotation representations.  Here we use the method in
    % Section B of rotation_examples.m to convert from Euler angles to DCM.
    yaw_rad   = pi/180*uavTruth.yaw_deg(kTime);
    pitch_rad = pi/180*uavTruth.pitch_deg(kTime);
    roll_rad  = pi/180*uavTruth.roll_deg(kTime);
    C_ned2b  = [cos(pitch_rad)*cos(yaw_rad)                                             cos(pitch_rad)*sin(yaw_rad)                                           -sin(pitch_rad); ...
                sin(roll_rad)*sin(pitch_rad)*cos(yaw_rad)-cos(roll_rad)*sin(yaw_rad)    sin(roll_rad)*sin(pitch_rad)*sin(yaw_rad)+cos(roll_rad)*cos(yaw_rad)   sin(roll_rad)*cos(pitch_rad); ...
                cos(roll_rad)*sin(pitch_rad)*cos(yaw_rad)+sin(roll_rad)*sin(yaw_rad)    cos(roll_rad)*sin(pitch_rad)*sin(yaw_rad)-sin(roll_rad)*cos(yaw_rad)   cos(roll_rad)*cos(pitch_rad)];

    % Compute perfect specific force
    fb_mps2_perfect = C_ned2b*vdot_ned_mps2(kTime,:)' - C_ned2b*[0;0;gravity_mps2];
    
    % Add bias and noise
    uavSensors.accel_fb_mps2(kTime,:) = fb_mps2_perfect' ...
                            + uavSensors.biases.accel_fb_mps2 ...
                            + uavSensors.sigmas.accel_noise_mps2*randn(1,3);    
end
 
% Magnetometer measurement
% 3D magnetometers are common for UAVs.  They are generally fixed to the
% UAV body and return a unit vector (in body coordinates) pointing toward
% Magnetic North (mag3d_unitVector_in_body).  Note that Magnetic North is
% offset from True North by the magnetic declination angle (mag_declination_deg),
% which varies across the Earth and can be determined using available
% databases.  In addition, a simple 2D magnetometer-based yaw measurement using
% the x- and y-components of the 3D magnetometer and the declination
% angle is also computed (mag2d_yaw_deg).  A state estimator could use either
% the 2D direct yaw measurement or the 3D unit-vector measurement, although
% the 2D variant will lose accuracy while pitching and rolling.
% Method:
% - Randomly select a Magnetic Declination angle (using a standard dev. of 10 degrees)
% - Randomly determine sensor biases
% - Loop through time to build 3D magnetometer unit vector measurement
% - Use x,y components of 3D magnetometer measurement to generate
%   simple 2D-magnetometer-derived yaw measurement
uavSensors.params.mag_declination_deg = 10*randn;
uavSensors.biases.mag3D_unitVector = uavSensors.sigmas.mag3D_unitVector_bias*randn(1,3);
for kTime=1:length(uavTruth.time_s)

    % Compute NED-to-body Direction Cosine Matrix for time index kTime.
    % See rotation_examples.m for more information about converting between
    % different rotation representations.  Here we use the method in
    % Section B of rotation_examples.m to convert from Euler angles to DCM.
    yaw_rad   = pi/180*uavTruth.yaw_deg(kTime);
    pitch_rad = pi/180*uavTruth.pitch_deg(kTime);
    roll_rad  = pi/180*uavTruth.roll_deg(kTime);
    C_ned2b  = [cos(pitch_rad)*cos(yaw_rad)                                             cos(pitch_rad)*sin(yaw_rad)                                           -sin(pitch_rad); ...
                sin(roll_rad)*sin(pitch_rad)*cos(yaw_rad)-cos(roll_rad)*sin(yaw_rad)    sin(roll_rad)*sin(pitch_rad)*sin(yaw_rad)+cos(roll_rad)*cos(yaw_rad)   sin(roll_rad)*cos(pitch_rad); ...
                cos(roll_rad)*sin(pitch_rad)*cos(yaw_rad)+sin(roll_rad)*sin(yaw_rad)    cos(roll_rad)*sin(pitch_rad)*sin(yaw_rad)-sin(roll_rad)*cos(yaw_rad)   cos(roll_rad)*cos(pitch_rad)];

    % DCM from "magnetic-NED" coordinates to true NED coordinates.
    % (Negative rotation of the magnetic declination angle about the vertical.)
    % Using the terminology used in rotation_examples.m:
    %    C_mag2ned = Cz(-mag_declination_deg*pi/180)
    C_mag2ned = [ cos(-pi/180*uavSensors.params.mag_declination_deg)  sin(-pi/180*uavSensors.params.mag_declination_deg)   0; ...
                 -sin(-pi/180*uavSensors.params.mag_declination_deg)  cos(-pi/180*uavSensors.params.mag_declination_deg)   0; ...
                 0                                                    0                                                    1];

    % Compute biased and noisy unit vector toward magnetic north, in body coordinates
    % - Scale so that 3x1 M is of unit magnitude
    M = C_ned2b*C_mag2ned*[1;0;0] ...
              + uavSensors.biases.mag3D_unitVector' ...
              + uavSensors.sigmas.mag3D_unitVector_noise*randn(1,3)';
    M = M/norm(M);
    
    % Store 3D magnetometer vector in uavSensors structure
    uavSensors.mag3D_unitVector_in_body(kTime,:) = M';
    
    % Use x,y components of M to generate a simple 2D-magnetometer-derived yaw measurement
    % - Force yaw measurement between -180 and 180 degrees
    uavSensors.mag2D_yaw_deg(kTime,1) = 180/pi*atan2(-M(2),M(1)) + uavSensors.params.mag_declination_deg;
    uavSensors.mag2D_yaw_deg(kTime,1) = mod(uavSensors.mag2D_yaw_deg(kTime,1)+180,360)-180; % -180 <= yaw <=180 
    
    % Define derived "2D" magnetometer noise and bias standard deviations.
    % Conveniently, the resulting "2D" angular noise, in rad, is approximately 
    % equivalent to the error in the "3D" unit vector.  
    if kTime==1
        uavSensors.sigmas.mag2D_yaw_noise_rad = 1*uavSensors.sigmas.mag3D_unitVector_noise;
        uavSensors.sigmas.mag2D_yaw_bias_rad  = 1*uavSensors.sigmas.mag3D_unitVector_bias;
    end
end

% Clean up any temporary variables that were created above
clear gravity_mps2 kGPS C_mag2ned C_ned2b M Vrel_b_mps fb_mps2_perfect kTime m yaw_rad pitch_rad roll_rad v_ned_mps v_wind_ned_mps 
clear vdot_n_mps2 vdot_e_mps2 vdot_d_mps2 vdot_ned_mps2

% Small Fixed-Wing UAS State Estimation Examples
% 
% This routine provides example methods for performing state estimation
% (i.e. position and orientation) of a fixed-wing UAS (Unmanned Aircraft
% System) using GPS, gyros, accelerometers, and a magnetometer as sensors.
% This code is meant as a companion to the JHU/APL Technical Digest article
% "Fundamentals of Small Unmanned Aircraft Flight", by Jeff Barton (see 
% Reference 1).  UASs are also referred to as Unmanned Aerial Vehicles
% (UAVs) and drones.
% 
% The routine is segmented into four overall components:
%   1) Load UAV truth data from a file (creates uavTruth structure)
%   2) Generate noisy UAV sensor measurements (creates uavSensors structure)
%   3) Perform UAV state estimation using a selected method (creates uavEst structure)
%   4) Display UAV state estimation results (compares uavTruth, uavSensors, and uavEst)
% 
% Five state estimation methods are provided, based on algorithms
% presented in Reference 1.  The user can choose which method to use:
%   1) EKF-based AHRS using Euler angles and a 2D magnetometer
%   2) EKF-based AHRS using quaternions and a 3D magnetometer
%   3) Feedback AHRS using Euler angles and a 2D magnetometer
%   4) EKF-based INS using Euler angles and a 2D magnetometer
%   5) EKF-based INS using quaternions and a 3D magnetometer
% Definitions:
%   EKF: Extended Kalman Filter
%   AHRS: Attitude and Heading Reference System
%   INS: Inertial Navigation System
%   
% Usage:
%   Simply run this file "uav_state_estimation" from Matlab.  The routine
%   will query you for which method you wish to use.
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

% Notes:
%   - The code is heavily commented, but it is not necessarily meant as a
%     tutorial on advanced topics such as Extended Kalman Filters.  Some
%     prior experience is assumed.
%   - The code is not intended to represent highly efficient algorithms.
%   - There are many methods to perform state estimation, and the methods
%     provided here are meant for educational purposes.  These methods are
%     not necessarily the best methods to be used on any specific UAV.
%   - This code uses three common methods to represent rotations:
%     Euler angles, Direction Cosine Matrices, and quaternions.
%     See rotation_examples.m for more background and conversions between
%     the rotation methods.
% 

disp(' ')
disp('This routine provides example methods for performing state estimation')
disp('(i.e. position and orientation) of a fixed-wing UAS using GPS, gyros, ')
disp('accelerometers, and a magnetometer as sensors.')

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Add UAV_Data and state estimation folders to path.
% (Files needed for different state estimation methods are separated into
% different folders.)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
addpath UavData
addpath AHRS_EKF_Euler
addpath AHRS_EKF_Quaternion
addpath AHRS_Feedback
addpath INS_EKF_Euler
addpath INS_EKF_Quaternion

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Clean up some structures that will be used within this code
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
clear uavTruth      % UAV truth structure (read from a file)
clear uavSensors    % UAV sensors structure (synthetically derived sensor measurements)
clear uavEst        % Resulting UAV state estimates structure
clear ekf           % Structure used in EKF methods.
clear ahrsFeedback  % Structure used in AHRS Feedback method.
clear gyro_integration_method % Gyro integration method used in AHRS Feedback method

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Load UAV truth data from a file.
% - UAV "truth" example data was generated using a 6DOF simulation.
%
% Truth data is provided as the uavTruth structure:
% uavTruth = 
%             time_s: [Kx1 double] Time, s
%             east_m: [Kx1 double] East position, m
%            north_m: [Kx1 double] North position, m
%            h_msl_m: [Kx1 double] Height above mean-sea-level, m
%          v_ned_mps: [Kx3 double] North-East-Down (NED) velocity vector, m/s
%            yaw_deg: [Kx1 double] Yaw angle, deg, positive East-of-North
%          pitch_deg: [Kx1 double] Pitch angle, deg, positive above horizontal
%           roll_deg: [Kx1 double] Roll angle, deg, positive banking right
%             wb_rps: [Kx3 double] Body attitude rate vector, radians/s
%     v_wind_ned_mps: [Kx3 double] NED velocity vector of wind, m/s
%             temp_K: [Kx1 double] Air temperature, Kelvin
%      pressure_Npm2: [Kx1 double] Air pressure, Newtons/m^2
%      density_kgpm3: [Kx1 double] Air density, kg/m^3
%
% The UavData directory contains multiple truth data files.
% User can select which to use.
disp(' ')
disp('Available UAV Truth Data Files:')
dir_mat_files = dir('UavData\*.mat');
for nFile=1:length(dir_mat_files)
    fprintf('   %d: %s\n',nFile,dir_mat_files(nFile).name);
end
nFileChoice = input('Choose a UAV Truth data file (e.g. 1<Enter>): ');
try
    load(dir_mat_files(nFileChoice).name)
catch
    error('Selected UAV Truth data file (%d) is invalid.\n',nFileChoice);
end
clear dir_mat_files nFile

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Generate noisy UAV sensor measurements.
% - Add noise and biases to truth data to generate sensor measurements.
%
% Generated sensor measurements are in the form of a uavSensors structure:
% uavSensors = 
%                       time_s: [Kx1 double]  Time, s
%                   GPS_east_m: [Kx1 double]  GPS-measured east position, m
%                  GPS_north_m: [Kx1 double]  GPS-measured north position, m
%                  GPS_h_msl_m: [Kx1 double]  GPS-measured height above mean-sea-level, m
%                GPS_v_ned_mps: [Kx3 double]  GPS-measured NED velocity vector, m/s
%                    GPS_valid: [Kx1 logical] GPS validity flag, true when GPS is valid
%           pitot_airspeed_mps: [Kx1 double]  Pitot-sensor-based airspeed, m/s
%                  gyro_wb_rps: [Kx3 double]  Gyro-based body attitude rate vector, radians/s
%                accel_fb_mps2: [Kx3 double]  Accelerometer-based specific force vector (accerations), m/s^2
%     mag3D_unitVector_in_body: [Kx3 double]  3D-magnetometer measurement: unit vector in body coords. pointing to Mag. North
%                mag2D_yaw_deg: [Kx1 double]  2D-magnetometer measurement: body yaw angle (from true north), deg
%                       sigmas: [struct]      Structure containing utilized sensor noise and bias standard deviations (sigmas)
%                       biases: [struct]      Structure containing utilized sensor biases
%                       params: [struct]      Structure containing utilized parameters (see below)
% uavSensors.params =
%                     dt_GPS_s: [1x1 double] Time between GPS measurements, s
%          mag_declination_deg: [1x1 double] Magnetic declination angle, deg
generate_uav_sensors_structure

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Perform UAV state estimation.
% - Use a user-specified method to perform UAV state estimation.
%
% Generated state estimates are stored in the form of a uavEst structure:
% uavEst = 
%        time_s: [Kx1 double] Time, s
%      roll_deg: [Kx1 double] Estimated roll angle, deg
%     pitch_deg: [Kx1 double] Estimate pitch angle, deg
%       yaw_deg: [Kx1 double] Estimated yaw angle, deg
%       north_m: [Kx1 double] Estimated north position, m
%        east_m: [Kx1 double] Estimated east position, m
%       h_msl_m: [Kx1 double] Estimated height above mean-sea-level, m
%     v_ned_mps: [Kx3 double] Estimated NED velocity vector, m/s
%        states: [struct]     Structure containing xhat state indices
%          xhat: [Kxn double] Estimated states
%             P: [Kxn double] Estimated state variances
disp(' ')
disp('UAV State Estimation Methods:')
disp('  1: EKF-based AHRS using Euler angles and a 2D magnetometer')
disp('  2: EKF-based AHRS using quaternions and a 3D magnetometer')
disp('  3: Feedback AHRS using Euler angles and a 2D magnetometer')
disp('  4: EKF-based INS using Euler angles and a 2D magnetometer')
disp('  5: EKF-based INS using quaternions and a 3D magnetometer')
state_estimation_method = input('Choose a numeric method (e.g. 1<Enter>): ');
switch state_estimation_method
    case 1
        perform_state_estimation__ahrs_ekf_euler
    case 2
        perform_state_estimation__ahrs_ekf_quaternion
    case 3
        perform_state_estimation__ahrs_feedback
    case 4
        perform_state_estimation__ins_ekf_euler
    case 5
        perform_state_estimation__ins_ekf_quaternion
    otherwise
        error('Selected state_estimation_method (%d) is invalid.\n',state_estimation_method);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Display UAV state estimation results.
% - Compares uavEst with uavTruth and uavSensors
display_uav_estimate_results

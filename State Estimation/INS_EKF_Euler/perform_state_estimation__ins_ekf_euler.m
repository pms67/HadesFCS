% UAV state estimation example: INS using Euler angles
%
% This routine is an Euler-based implementation of the "Full GPS 
% Inertial Navigation System" state estimation method described in
% Reference 1.  Specifically, this method uses a 15-state Extended Kalman
% Filter to estimate the UAV position, velocity, and attitude states, 
% while simultaneously estimating gyro and accelerometer biases.
%
% This routine loops through time to perform state estimation using the UAV
% sensor measurements provided in the uavSensors structure.  (This routine 
% is meant to be executed via the uav_state_estimation.m script.)
%
% The discrete-time Extended Kalman Filter is implemented via the provided 
% perform_EKF() routine.
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

% NOTE: There are many ways to represent rotations.  See
%       rotation_examples.m for background and conversions between the
%       methods.  The methods used herein are:
%         - Euler Angles: [yaw; pitch; roll] of UAV, defining the 
%           transformation from the inertial North-East-Down frame to the 
%           UAV body x-y-z frame.  Rotation order: Z-Y-X, or yaw-about-z,
%           then pitch-about-y, then roll-about-x.
%         - Direction Cosine Matrix (DCM): 3x3 matrix representing the
%           Z-Y-X Euler Angle rotation described above. (Akin to the
%           default rotation order for ANGLE2DCM in the Matlab Aerospace 
%           Toolbox.)  Example:  C_ned2b = Cx(roll)*Cy(pitch)*Cz(yaw).
%

% Make sure uavSensors exists
if ~exist('uavSensors')
    error(['This routine is meant to be run after creating the uavSensors structure. ' ...
          'Try running the main routine ''uav_state_estimation.m''.']) 
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Constants
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
gravity_mps2 = 9.81;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Establish gaussian noise assumptions used in EKF state estimator
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% For convenience, base EKF gaussian noise assumptions on true noise
% parameters (where applicable), defined in "generate_uav_sensors_structure.m".
% Note: "sigma" refers to the standard deviation of a random process.

% EKF measurement noise estimates (used to form "R" matrix in EKF)
% Note: Position, velocity and magnetometer biases are not being
%   estimated in this EKF, yet the true measurements are biased.  In order
%   to accomodate the biases without violating the gaussian EKF measurement
%   assumption too much, we'll lump the noise standard deviations with scaled
%   (factor-of-5) bias standard deviations in forming the EKF measurement
%   noise:  ekf_sigma_meas_noise = sqrt(true_sigma_meas_noise^2 + (5*true_sigma_meas_bias)^2).
ekf = [];
ekf.sigmas.mag2D_meas_rad        = sqrt(uavSensors.sigmas.mag2D_yaw_noise_rad^2+(5*uavSensors.sigmas.mag2D_yaw_bias_rad)^2);
ekf.sigmas.pos_meas_m            = sqrt(uavSensors.sigmas.GPSpos_noise_m^2     +(5*uavSensors.sigmas.GPSpos_bias_m)^2        );
ekf.sigmas.vel_meas_mps          = sqrt(uavSensors.sigmas.GPSvel_noise_mps^2   +(5*uavSensors.sigmas.GPSvel_bias_mps)^2      );

% EKF initial bias uncertainty estimates (used to form initial "P" matrix in EKF)
ekf.sigmas.gyro_bias_rps         = uavSensors.sigmas.gyro_bias_rps;
ekf.sigmas.accel_bias_mps2       = uavSensors.sigmas.accel_bias_mps2;

% EKF process noise estimates (used to form "Q" matrix in EKF)
% Note: Assigning process noise values requires some engineering judgement.
%   In general, Q establishes how much we expect reality to deviate from our
%   simple model of vehicle motion.  
%   Process noise of sensor biases are near-zero (e.g. 1e-6) because we 
%   don't expect them to vary very quickly.
ekf.sigmas.attitude_process_noise_rad = 0.002;% Euler angle process noise, rad
ekf.sigmas.pos_process_noise_m = 0;           % Position process noise, m
ekf.sigmas.vel_process_noise_mps = 2;         % Velocity process noise, m/s
ekf.sigmas.gyroBias_process_noise_rps = 1e-6; % Gyro bias process noise, rad/s
ekf.sigmas.accelBias_process_noise_mps2=1e-6; % Accel bias process noise, m/s^2


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialize EKF state estimate (xhat) and covariance (P)
%   State estimates, xhat:
%     - Use GPS and magnetometer to provide an initial state estimate.
%     - This method uses Euler angles to represent the attitude of
%       the UAV body frame relative to the fixed North-East-Down frame. 
%       (The NED-to-body Euler angles are often denoted yaw, pitch,
%       and roll.)
%     - In this method, xhat is a column vector with 15 elements (15x1)
%
%   Covariance, P:
%     - P is a square symmetric matrix, where the nth diagonal element is the
%       variance of the nth state.  The off-diagonal terms represent the
%       covariance between different states.
%     - In this method, P is 15x15.
%     - Although we are initializing the attitude, position and velocity
%       states, we will start the filter with initial uncertainties are 
%       "large" compared to the expected magnitudes of the states.
%     - The EKF designer must use some a priori information to specify
%       the initial uncertainties in the gyro and accelerometer biases
%       (via ekf.sigmas.gyro_bias_rps & ekf.sigmas.accel_bias_mps2).
%     - Using initial uncertainty assumptions (sigmaM for the Mth state),
%       the initial covariance P is a matrix with the square of the
%       uncertainties along the diagonal and zeros in the off-diagonals:
%                  [ sigma1^2    0      ...      0     ]
%             P  = [   0      sigma2^2  ...      0     ]
%                  [  ...       ...     ...     ...    ]
%                  [   0         0      ...   sigmaN^2 ]
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Define EKF state indices (helpful for post-processing)
ekf.states.phi  = 1; % Roll, Pitch, Yaw Euler angles, rad
ekf.states.theta= 2; % 
ekf.states.psi  = 3; % 
ekf.states.Pn   = 4; % Position, North/East/Altitude, meters
ekf.states.Pe   = 5; % 
ekf.states.Alt  = 6; % 
ekf.states.Vn   = 7; % Velocity, North/East/Down, m/s
ekf.states.Ve   = 8; % 
ekf.states.Vd   = 9; % 
ekf.states.bwx  =10; % Gyro biases, rad/s
ekf.states.bwy  =11; % 
ekf.states.bwz  =12; % 
ekf.states.bax  =13; % Accelerometer biases, m/s^2
ekf.states.bay  =14; % 
ekf.states.baz  =15; % 

% Use magnetometer (solely) for initial orientation estimate.  
phi = 0;                                   % Initial roll estimate, rad
theta=0;                                   % Initial pitch estimate, rad
psi = uavSensors.mag2D_yaw_deg(1)*pi/180;  % Initial yaw estimate, rad
euler_init = [ phi theta psi ];
       
% Use GPS for initial position and velocity estimates.      
pos_init = [ uavSensors.GPS_north_m(1) uavSensors.GPS_east_m(1) uavSensors.GPS_h_msl_m(1) ];
vel_init = uavSensors.GPS_v_ned_mps(1,:);

% Build initial state estimates
ekf.xhat = [ ...
        euler_init ...                              %   1-3: init Euler angle (Roll, Pitch, Yaw) estimates
        pos_init ...                                %   4-6: init North-East-Alt position, m
        vel_init ...                                %   7-9: init NED velocity, m/s
        [0 0 0] ...                                 % 10-12: init XYZ gyro bias estimates, rad/s
        [0 0 0] ...                                 % 13-15: init XYZ accel bias estimates, m/s^2
    ]';
clear psi theta phi euler_init pos_init vel_init

% Start with "large" uncertainties and build initial covariance (state uncertainty)
large_angle_uncertainty_rad = 30*pi/180;
large_pos_uncertainty_m = 100;
large_vel_uncertainty_mps = 10;
ekf.P = diag([...
        [1 1 1]*large_angle_uncertainty_rad ...     % init Euler angle (NED-to-body) uncertainties, rad
        [1 1 1]*large_pos_uncertainty_m ...         % init North-East-Alt position uncertainties, m
        [1 1 1]*large_vel_uncertainty_mps ...       % init NED velocity uncertainties, m/s
        [1 1 1]*ekf.sigmas.gyro_bias_rps ...        % init XYZ gyro bias uncertainties, rad/s
        [1 1 1]*ekf.sigmas.accel_bias_mps2 ...      % init XYZ accel bias uncertainties, m/s^2
    ].^2);
% Make sure no elements of P diagonal are zero! Arbitrarily use a
% minimum of (1e-3)^2.  (For example, if gyroBias was set to zero when
% generating the sensor measurement, we still want some initial P.)
ekf.P = max(ekf.P,(1e-3)^2*eye(size(ekf.P)));    
clear large_angle_uncertainty_rad large_pos_uncertainty_m large_vel_uncertainty_mps
    
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Loop through time to iteratively build the state estimation data    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
for kTime=1:length(uavSensors.time_s)
   
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Extract variables at time kTime from uavSensors structure
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    time_s                  = uavSensors.time_s(kTime);
    GPS_east_m              = uavSensors.GPS_east_m(kTime);
    GPS_north_m             = uavSensors.GPS_north_m(kTime);
    GPS_h_msl_m             = uavSensors.GPS_h_msl_m(kTime);
    GPS_v_ned_mps           = uavSensors.GPS_v_ned_mps(kTime,:)';            % 3x1 vector
    GPS_valid               = uavSensors.GPS_valid(kTime);
    pitot_airspeed_mps      = uavSensors.pitot_airspeed_mps(kTime);
    gyro_wb_rps             = uavSensors.gyro_wb_rps(kTime,:)';              % 3x1 vector
    accel_fb_mps2           = uavSensors.accel_fb_mps2(kTime,:)';            % 3x1 vector
    mag3D_unitVector_in_body= uavSensors.mag3D_unitVector_in_body(kTime,:)'; % 3x1 vector
    mag2D_yaw_deg           = uavSensors.mag2D_yaw_deg(kTime);

    % Define the change-in-time between time steps.  If this is the first
    % time through, use the change in time between kTime=2 and kTime=1.  
    % Otherwise, use the time between steps kTime and kTime-1.
    if kTime==1
        dt_s = uavSensors.time_s(2) - uavSensors.time_s(1);
    else
        dt_s = time_s - uavSensors.time_s(kTime-1);
    end

       
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Prepare inputs to Extended Kalman Filter (EKF) routine, perform_ekf().
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % The Extended Kalman Filter is an iterative method for providing an 
    % estimate (xhat) of a true non-linear state vector (x) based on measurements (z).
    % It is assumed that the dynamics of x and the measurement z can be
    % represented as a non-linear continuous-time model:
    %
    %    Non-Linear Continuous-Time Model
    %    --------------------------------
    %    xdot(t) = f(x(t),u(t)) + w(t),           E(w(t)*w(t)')=Q(t)
    %    z(t)    = h(x(t),u(t)) + v(t),           E(v(t)*v(t)')=R(t)
    %
    % where xdot is the time-derivative of x; f() and h() are non-linear
    % functions mapping the state vector x, and possibly other inputs
    % u, into the state derivatives (xdot) and measurement model, 
    % respectively; and w & v are Gaussian processes representing unmodeled 
    % state dynamics process noise and measurement noise, respectively.  
    % Q & R are the covariances of w and v.  In addition to estimating the
    % state vector x, the EKF maintains an error covariance matrix, P,
    % which is a representation of its uncertainty in the estimate of x.
    %
    % Note that the Extended Kalman Filter is based on a linearization of
    % the above non-linear system, and thus requires the linearized
    % dynamics and measurement matrices, F & H, as in:
    %
    %    Linearized Continuous-Time Model
    %    --------------------------------
    %    xdot(t) = F(t)*x(t) + Fu(t)*u(t) + w(t),  E(w(t)*w(t)')=Q(t)
    %    z(t)    = H(t)*x(t) + Hu(t)*u(t) + v(t),  E(v(t)*v(t)')=R(t)

    % Determine continuous time state dynamics process noise covariance, Q. 
    % (Here, Q is a diagonal 15x15 matrix)
    % The parameters used were defined above.
    ekf.Q = diag([...
            [1 1 1]*ekf.sigmas.attitude_process_noise_rad ...   % Euler angle process noise, rad
            [1 1 1]*ekf.sigmas.pos_process_noise_m ...          % North-East-Alt position process noise, m
            [1 1 1]*ekf.sigmas.vel_process_noise_mps ...        % NED velocity process noise, m/s
            [1 1 1]*ekf.sigmas.gyroBias_process_noise_rps ...   % XYZ gyro bias process noise, rad/s
            [1 1 1]*ekf.sigmas.accelBias_process_noise_mps2 ... % XYZ accel bias process noise, m/s^2
        ].^2);
    % Make sure no elements of Q diagonal are zero! Arbitrarily use a
    % minimum of (1e-3)^2.  (For example, if gyroBias was set to zero when
    % generating the sensor measurement, we still want some Q process
    % noise.)
    ekf.Q = max(ekf.Q,(1e-3)^2*eye(size(ekf.Q)));    

    % Assign functions to return dynamics model and measurement model.
    %   [xdot F]=compute_xdot_and_F(xhat,u1,u2,...)
    %     xdot: estimated state derivatives with respect to time, xdot=dx/dt
    %     F:    linearized dynamics model, where F is a matrix mapping xhat to xdot
    %     xhat: EKF state vector estimate
    %     uk:   Additional inputs necessary to compute: xdot, F, zhat, and H (k=1,2,...)
    %   [zhat H]=compute_zhat_and_H(xhat,u1,u2,...)
    %     zhat: re-creation of measurement z, based on current state estimate
    %     H:    linearized measurement model, where H is a matrix mapping xhat to zhat
    %     xhat: EKF state vector estimate
    %     uk:   Additional inputs necessary to compute: xdot, F, zhat, and H (k=1,2,...)
    ekf.compute_xdot_and_F = @compute_xdot_and_F__ins_ekf_euler;
    ekf.compute_zhat_and_H = @compute_zhat_and_H__ins_ekf_euler;
    ekf.u1 = gyro_wb_rps;                           % Additional input needed to compute xdot, F, zhat, or H
    ekf.u2 = accel_fb_mps2;                         % Additional input needed to compute xdot, F, zhat, or H
    ekf.u3 = gravity_mps2;                          % Additional input needed to compute xdot, F, zhat, or H

    % If GPS is available, assign measurement, z, and the sampled
    % measurement uncertainty matrix, R(k).
    % Note: For convenience, we only apply magnetometer measurements whenever
    %   a GPS measurement is available.  In practice, the magnetometer and GPS
    %   measurements can and should be applied independently.
    if GPS_valid
        
        % z: measurement vector
        %
        % Note: Make sure the yaw measurement is not "wrapped" (by 2pi)
        %       relative to the yaw state, ekf.xhat(3).
        yaw_measurement_rad = mag2D_yaw_deg*pi/180;
        while yaw_measurement_rad>ekf.xhat(3)+pi, yaw_measurement_rad=yaw_measurement_rad-2*pi; end
        while yaw_measurement_rad<ekf.xhat(3)-pi, yaw_measurement_rad=yaw_measurement_rad+2*pi; end
        ekf.z = [...
                yaw_measurement_rad; ...                    % Magnetometer-based yaw measurement, rad
                [GPS_north_m; GPS_east_m; GPS_h_msl_m]; ... % North-East-Alt measurements, m [3x1]
                GPS_v_ned_mps ...                           % NED velocity measurements, m/s [3x1]
            ];
        
        % R: sampled measurement uncertainty, R(k)
        %
        % Note: The 2D magnetometer as used in this method can be very
        % inaccurate during turns, and the EKF will try to attribute that
        % inaccuracy to the accelerometer bias states!  So, we'll scale the 
        % magnetometer uncertainty by a factor of 10 when either the 
        % previously estimated roll exceeds 5 deg or the gyro-measured roll
        % rate exceeds 3 deg/s.
        if kTime>1 && (abs(uavEst.roll_deg(kTime-1))>5 || abs(gyro_wb_rps(1)*180/pi)>3)
            SF_magUnc=10; % Magnetometer uncertainty Scale Factor
        else
            SF_magUnc=1;  % Magnetometer uncertainty Scale Factor
        end        
        ekf.R = diag([...
                [1]*SF_magUnc*ekf.sigmas.mag2D_meas_rad ...  % Magnetometer-based yaw measurement noise, rad
                [1 1 1]*ekf.sigmas.pos_meas_m ...            % North-East-Alt position measurement noise, m
                [1 1 1]*ekf.sigmas.vel_meas_mps ...          % NED velocity measurement noise, m/s
            ].^2);
        % Make sure no elements of R diagonal are zero! Arbitrarily use a
        % minimum of (1e-3)^2.
        ekf.R = max(ekf.R,(1e-3)^2*eye(size(ekf.R)));    
        
    else
        ekf.z = []; % Empty means no valid measurement
        ekf.R = [];
    end

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Perform EKF
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    [ekf.xhat ekf.P] = perform_ekf( ...
                ekf.compute_xdot_and_F, ... % Handle to function which returns xdot (nx1 state derivatives) and F (nxn linearized dynamics model)
                ekf.xhat, ...               % Current full state estimate (nx1)
                ekf.P, ...                  % Current state covariance matrix (nxn)
                ekf.Q, ...                  % Continuous time state process noise matrix (nxn)
                ekf.compute_zhat_and_H, ... % Handle to function which returns zhat (mx1 non-linear meas estimate) and H (mxm linearized meas model)
                ekf.z, ...                  % Current measurement vector (mx1)
                ekf.R, ...                  % Sampled measurement error covariance matrix (mxm)
                dt_s, ...                   % Time step interval, seconds: t(k+1) = t(k) + dt
                ekf.u1, ...                 % Additional input for compute_xdot_and_F() and compute_zhat_and_H().
                ekf.u2, ...                 % Additional input for compute_xdot_and_F() and compute_zhat_and_H().
                ekf.u3 ...                  % Additional input for compute_xdot_and_F() and compute_zhat_and_H().
                );

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Compile uavEst, structure of state estimates
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    % Append current state estimates to uavEst structure components
    if kTime==1        
        % Allocate memory for vectors and arrays
        uavEst=[];
        uavEst.time_s    = 0*uavTruth.time_s;
        uavEst.roll_deg  = 0*uavTruth.time_s;
        uavEst.pitch_deg = 0*uavTruth.time_s;
        uavEst.yaw_deg   = 0*uavTruth.time_s;
        uavEst.north_m   = 0*uavTruth.time_s;
        uavEst.east_m    = 0*uavTruth.time_s;
        uavEst.h_msl_m   = 0*uavTruth.time_s;
        uavEst.v_ned_mps = 0*uavTruth.time_s*[1 1 1];
    end
    uavEst.time_s(kTime,:)    = uavSensors.time_s(kTime);
    uavEst.roll_deg(kTime,:)  = ekf.xhat(1)*180/pi; % Roll estimate, deg
    uavEst.pitch_deg(kTime,:) = ekf.xhat(2)*180/pi; % Pitch estimate, deg
    uavEst.yaw_deg(kTime,:)   = ekf.xhat(3)*180/pi; % Yaw estimate, deg
    uavEst.north_m(kTime,:)   = ekf.xhat(4);        % North position estimate, m
    uavEst.east_m(kTime,:)    = ekf.xhat(5);        % East position estimate, m
    uavEst.h_msl_m(kTime,:)   = ekf.xhat(6);        % Altitude estimate, above MeanSeaLevel, m
    uavEst.v_ned_mps(kTime,:) = ekf.xhat(7:9);      % NED Velocity estimate, m/s

    % Save a state vector (called xhat) consisting of the estimated EKF
    % states (including biases).
    % Also save the corresponding covariance diagonals (P).
    if kTime==1
        % Pass state indices from ekf to uavEst
        uavEst.states = ekf.states;
        
        % Allocate memory for arrays
        uavEst.xhat = zeros(length(uavTruth.time_s),length(ekf.xhat));
        uavEst.P    = zeros(length(uavTruth.time_s),length(ekf.xhat));
    end
    uavEst.xhat(kTime,:) = ekf.xhat';
    uavEst.P(kTime,:) = diag(ekf.P);
            
    % Note progress with dots
    if mod(kTime,ceil(length(uavTruth.time_s)/40))==0
        fprintf('.');
    end
        
end % end kTime loop
fprintf('\n')

% Clean up variables
clear time_s kTime dt_s GPS_east_m GPS_north_m GPS_h_msl_m GPS_v_ned_mps GPS_valid baro_h_msl_m pitot_airspeed_mps gyro_wb_rps accel_fb_mps2 gravity_mps2 mag3D_unitVector_in_body mag2D_yaw_deg
clear yaw_measurement_rad
clear SF_magUnc


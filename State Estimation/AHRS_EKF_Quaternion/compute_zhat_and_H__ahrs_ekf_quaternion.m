function [zhat H] = compute_zhat_and_H__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg)
% compute_zhat_and_H__ahrs_ekf_quaternion: @compute_zhat_and_H function for quaternion-based AHRS
%
% Function recreates measurement vector, zhat, and linearized 
% measurement matrix, H.  This function is used within: 
%    perform_state_estimation__ahrs_ekf_quaternion.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [zhat H] = compute_zhat_and_H__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg)
%
% Inputs:
%   x:             7x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   vb_ref_mps:    3x1 reference velocity vector in body coords, m/s
%   g_mps2:        Magnitude of gravity, m/s2
%   mag_declination_deg: Angle between true north and magnetic north, deg
%
% Outputs:
%   zhat:   6x1 recreation of measurement vector using x,
%              where: zhat = h(x,...) = [fx; fy; fz; magVecX; magVecY; magVecZ]
%   H:      6x7 Linerized state dynamics matrix, H = d(zhat)/dx
%           (H is the Jacobian of the vector zhat with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix H can sometimes be difficult.  The
%   expression for H utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of H
%   by calling the function "compute_zhat_and_H__ahrs_ekf_quaternion" without any arguments.
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

% References:
%   1  Barton, J. D., “Fundamentals of Small Unmanned Aircraft Flight,” 
%      Johns Hopkins APL Technical Digest, Volume 31, Number 2 (2012).
%      http://www.jhuapl.edu/techdigest/TD/td3102/

% If compute_zhat_and_H is called without any arguments, use the Symbolic Toolbox
% to compute and display the linearized measurement matrix, H.  This is
% provided merely as a helper to the interested user.
if nargin==0
    % Derive H using the Symbolic Toolbox
    derive_H
    return
end

% Extract state variables from state vector
%  x = [q0; q1; q2; q3; bwx; bwy; bwz];
n=1;
q0=x(n);  n=n+1; % Attitude quaternion
q1=x(n);  n=n+1; % 
q2=x(n);  n=n+1; % 
q3=x(n);  n=n+1; % 
bwx=x(n); n=n+1; % Gyro biases, rad/s
bwy=x(n); n=n+1; % 
bwz=x(n); n=n+1; % 

% Angular rate measurement from gyros
wx=gyro_wb_rps(1); % rad/s
wy=gyro_wb_rps(2);
wz=gyro_wb_rps(3);

% Reference velocity vector, in body xyz coordinates, m/s
Vx=vb_ref_mps(1); % m/s
Vy=vb_ref_mps(2); % 
Vz=vb_ref_mps(3); % 

% Direction Cosine Matrix (DCM) from NED coordinates to body cooridinates
% expressed using quaternions, using the current state estimate.
% (See rotation_examples.m for more information on quaternions and
% conversions between rotation methods.  Here we use the method in 
% Section E to convert from a quaternion to a DCM.)
C_ned2b  = [ 1-2*(q2^2+q3^2)    2*(q1*q2+q3*q0)     2*(q1*q3-q2*q0); ...
             2*(q1*q2-q3*q0)    1-2*(q1^2+q3^2)     2*(q2*q3+q1*q0); ...
             2*(q1*q3+q2*q0)    2*(q2*q3-q1*q0)     1-2*(q1^2+q2^2)];

% DCM from "magnetic-NED" coordinates to true NED coordinates.
% (Negative rotation of the magnetic declination angle about the vertical.)
% Using the terminology used in rotation_examples.m:
%    C_mag2ned = Cz(-mag_declination_deg*pi/180)
C_mag2ned = [  cos(-mag_declination_deg*pi/180)   sin(-mag_declination_deg*pi/180)   0; ...
              -sin(-mag_declination_deg*pi/180)   cos(-mag_declination_deg*pi/180)   0; ...
                       0                                  0                          1];

% Compute re-creation of 3D magnetometer unit vector, in body coordinates,
% from the state vector.  The 3D magnetometer unit vector output is a
% vector in body coordinates that points toward Magnetic North.
% In the computation of mag3D_unitVector_in_body:
%    [1;0;0]:                   Unit vector pointing toward MagNorth in the 
%                               "magnetic-NED" coordinate frame
%    C_mag2ned*[1;0;0]:         Unit vector pointing toward MagNorth in the 
%                               NED coordinate frame
%    C_ned2b*C_mag2ned*[1;0;0]: Unit vector pointing toward MagNorth in the 
%                               UAV body coordinate frame
mag3D_unitVector_in_body = C_ned2b*C_mag2ned*[1;0;0];

% Compute re-creation of accelerometer specific force measurement, 
% in body coordinates, akin to Equations 11 & 21 in Reference 1.
%
%                                                [      0       ]
%    f_b_steady_state = cross(w_b,v_b) - C_ned2b*[      0       ]
%                                                [ gravity_mps2 ]
%
% Note: We are using a "steady-state turn" assumption, where the 
%   translational acceleration in the body-frame [d(v_b)/dt] is neglected.  
%   See generate_uav_sensors_structure.m for a derivation of specific force.
% Note: We are using our gyro bias estimates to de-bias the raw gyro
%   measurements, [wx; wy; wz].
accel_fb_mps2 = cross([wx-bwx; wy-bwy; wz-bwz], [Vx;Vy;Vz]) - C_ned2b*[0;0;g_mps2];
fx = accel_fb_mps2(1);
fy = accel_fb_mps2(2);
fz = accel_fb_mps2(3);

% Compute re-creation of non-linear measurement vector from the state vector.
%   zhat = h(x) = [ ...
%                  [fx; fy; fz]; ...  % Accelerometer specific force measurement
%                  [unit vector pointing to MagNorth in body coords]; ...
%                 ];
% Akin to Equation 21 in Reference 1, except that the 3D magnetometer unit
% vector measurement is used instead of a direct yaw measurement.
zhat = [ ...
        [fx; fy; fz]; ...             % [Accel. specific force measurement]
        mag3D_unitVector_in_body;     % [3D magnetometer unit vector]
       ];

% Compute linerized state dynamics matrix, H = d(zhat)/dx.
% H was derived a priori using the Symbolic Toolbox, as in the subfunction
% "derive_H".
H = [ ...
    [                                                                     2*g_mps2*q2,                                                                  (-2)*g_mps2*q3,                                                                     2*g_mps2*q0,                                                                    (-2)*g_mps2*q1,   0, -Vz,  Vy]
    [                                                                  (-2)*g_mps2*q1,                                                                  (-2)*g_mps2*q0,                                                                  (-2)*g_mps2*q3,                                                                    (-2)*g_mps2*q2,  Vz,   0, -Vx]
    [                                                                               0,                                                                     4*g_mps2*q1,                                                                     4*g_mps2*q2,                                                                                 0, -Vy,  Vx,   0]
    [                                          2*q3*sin((pi*mag_declination_deg)/180),                                          2*q2*sin((pi*mag_declination_deg)/180), 2*q1*sin((pi*mag_declination_deg)/180) - 4*q2*cos((pi*mag_declination_deg)/180),   2*q0*sin((pi*mag_declination_deg)/180) - 4*q3*cos((pi*mag_declination_deg)/180),   0,   0,   0]
    [                                       (-2)*q3*cos((pi*mag_declination_deg)/180), 2*q2*cos((pi*mag_declination_deg)/180) - 4*q1*sin((pi*mag_declination_deg)/180),                                          2*q1*cos((pi*mag_declination_deg)/180), - 2*q0*cos((pi*mag_declination_deg)/180) - 4*q3*sin((pi*mag_declination_deg)/180),   0,   0,   0]
    [ 2*q2*cos((pi*mag_declination_deg)/180) - 2*q1*sin((pi*mag_declination_deg)/180), 2*q3*cos((pi*mag_declination_deg)/180) - 2*q0*sin((pi*mag_declination_deg)/180), 2*q0*cos((pi*mag_declination_deg)/180) + 2*q3*sin((pi*mag_declination_deg)/180),   2*q1*cos((pi*mag_declination_deg)/180) + 2*q2*sin((pi*mag_declination_deg)/180),   0,   0,   0]
    ];

% Helper function to show how to use the Symbolic Toolbox to derive H,
% the linearized measurement matrix.  This sub-function is accessed when
% this file is called without any arguments.  This routine prints the 
% resulting H, using symbolics, to the Matlab console.
function derive_H

% Test whether user has symbolic toolbox
try
    sym('testSymbolic');
catch
    error(['An error occurred creating a symbolic expression.  ' ...
           'Calling this routine without any arguments to symbolically derive ' ...
           'the matrix H is only supported if you have the Symbolics Toolbox.  ' ...
           'This capability is not necessary to run "uav_state_estimation".']);
end
          
% Symbolically define state vector
q0     = sym('q0');
q1     = sym('q1');
q2     = sym('q2');
q3     = sym('q3');
bwx    = sym('bwx');
bwy    = sym('bwy');
bwz    = sym('bwz');
x = [q0; q1; q2; q3; bwx; bwy; bwz];

% Symbolically define gyro measurement
syms wx wy wz 
gyro_wb_rps=[wx; wy; wz];

% Symbolically definine reference velocity vector (body coords)
syms Vx Vy Vz
vb_ref_mps=[Vx; Vy; Vz];

% Symbolically define gravity in m/s^2
syms g_mps2

% Symbolically define magnetic declination
syms mag_declination_deg

% Call compute_zhat_and_H using symbolic inputs to make a
% symbolic zhat
zhat = compute_zhat_and_H__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg);

% Differentiate zhat symbolically (Same as: H = jacobian(zhat,x);)
for n=1:length(x)
    H(:,n) = diff(zhat,x(n)); % Derivative of zhat wrt nth state variable
end
   
% Display H
fprintf('\nLinearized measurement matrix, H\n')
H

assignin('base','H',H);

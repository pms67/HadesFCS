function [zhat H] = compute_zhat_and_H__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg)
% compute_zhat_and_H__ins_ekf_quaternion: @compute_zhat_and_H function for Quaternion-based INS
%
% Function recreates measurement vector, zhat, and linearized 
% measurement matrix, H.  This function is used within: 
%    perform_state_estimation__ins_ekf_quaternion.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [zhat H] = compute_zhat_and_H__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg)
%
%   NOTE: gyro_wb_rps, accel_fb_mps2 and g_mps2 are not used in this routine,
%         but they are used in the sister routine
%         compute_xdot_and_F__ins_ekf_quaternion()
%
% Inputs:
%   x:             16x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   accel_fb_mps2: 3x1 Accelerometer measurements, m/s2
%   g_mps2:        Magnitude of gravity, m/s2
%   mag_declination_deg: Angle between true north and magnetic north, deg
%
% Outputs:
%   zhat:   9x1 recreation of measurement vector using x,
%              where: zhat = h(x,...) = [magVecX; magVecY; magVecZ; Pn; Pe; Alt; Vn; Ve; Vd];
%   H:      9x16 Linerized state dynamics matrix, H = d(zhat)/dx
%           (H is the Jacobian of the vector zhat with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix H can sometimes be difficult.  The
%   expression for H utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of H
%   by calling the function "compute_zhat_and_H__ins_ekf_quaternion" without any arguments.
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
%  x = [q0; q1; q2; q3; Pn; Pe; Alt; Vn; Ve; Vd; bwx; bwy; bwz; bax; bay; baz];
n=1;
q0=x(n);  n=n+1; % Attitude quaternion
q1=x(n);  n=n+1; % 
q2=x(n);  n=n+1; % 
q3=x(n);  n=n+1; % 
Pn=x(n);  n=n+1; % Position, North/East/Altitude, meters
Pe=x(n);  n=n+1; % 
Alt=x(n); n=n+1; % 
Vn=x(n);  n=n+1; % Velocity, North/East/Down, m/s
Ve=x(n);  n=n+1; % 
Vd=x(n);  n=n+1; % 
bwx=x(n); n=n+1; % Gyro biases, rad/s
bwy=x(n); n=n+1; % 
bwz=x(n); n=n+1; % 
bax=x(n); n=n+1; % Accelerometer biases, m/s^2
bay=x(n); n=n+1; % 
baz=x(n); n=n+1; % 

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

% Compute re-creation of non-linear measurement vector from the state vector.
%   zhat = h(x) = [ ...
%                  [unit vector pointing to MagNorth in body coords]; ...
%                  [North Pos., East Pos., Altitude]; ...
%                  [North Vel., East Vel., Down Velocity]; ...
%                 ];
% Akin to Equation 25 in Reference 1, except that the 3D magnetometer unit
% vector measurement is used instead of a direct yaw measurement.
zhat = [ ...
        mag3D_unitVector_in_body;     % [3D magnetometer unit vector]
        [Pn; Pe; Alt]; ...            % [Pn; Pe; Alt]
        [Vn; Ve; Vd]; ...             % [Vn; Ve; Vd]
       ];


% Compute linerized state dynamics matrix, H = d(zhat)/dx.
% H was derived a priori using the Symbolic Toolbox, as in the subfunction
% "derive_H".
H = [ ...
    [                                          2*q3*sin((pi*mag_declination_deg)/180),                                          2*q2*sin((pi*mag_declination_deg)/180), 2*q1*sin((pi*mag_declination_deg)/180) - 4*q2*cos((pi*mag_declination_deg)/180),   2*q0*sin((pi*mag_declination_deg)/180) - 4*q3*cos((pi*mag_declination_deg)/180), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                       (-2)*q3*cos((pi*mag_declination_deg)/180), 2*q2*cos((pi*mag_declination_deg)/180) - 4*q1*sin((pi*mag_declination_deg)/180),                                          2*q1*cos((pi*mag_declination_deg)/180), - 2*q0*cos((pi*mag_declination_deg)/180) - 4*q3*sin((pi*mag_declination_deg)/180), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [ 2*q2*cos((pi*mag_declination_deg)/180) - 2*q1*sin((pi*mag_declination_deg)/180), 2*q3*cos((pi*mag_declination_deg)/180) - 2*q0*sin((pi*mag_declination_deg)/180), 2*q0*cos((pi*mag_declination_deg)/180) + 2*q3*sin((pi*mag_declination_deg)/180),   2*q1*cos((pi*mag_declination_deg)/180) + 2*q2*sin((pi*mag_declination_deg)/180), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]
    [                                                                               0,                                                                               0,                                                                               0,                                                                                 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]
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
Pn     = sym('Pn');
Pe     = sym('Pe');
Alt    = sym('Alt');
Vn     = sym('Vn');
Ve     = sym('Ve');
Vd     = sym('Vd');
bwx    = sym('bwx');
bwy    = sym('bwy');
bwz    = sym('bwz');
bax    = sym('bax');
bay    = sym('bay');
baz    = sym('baz');
x = [q0; q1; q2; q3; Pn; Pe; Alt; Vn; Ve; Vd; bwx; bwy; bwz; bax; bay; baz];

% Symbolically define gyro measurement
syms wx wy wz 
gyro_wb_rps=[wx; wy; wz];

% Symbolically define accelerometer measurement
syms fx fy fz
accel_fb_mps2=[fx; fy; fz];

% Symbolically define gravity in m/s^2
syms g_mps2

% Symbolically define magnetic declination
syms mag_declination_deg

% Call compute_zhat_and_H using symbolic inputs to make a
% symbolic zhat
zhat = compute_zhat_and_H__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg);

% Differentiate zhat symbolically (Same as: H = jacobian(zhat,x);)
for n=1:length(x)
    H(:,n) = diff(zhat,x(n)); % Derivative of zhat wrt nth state variable
end
   
% Display H
fprintf('\nLinearized measurement matrix, H\n')
H

assignin('base','H',H);

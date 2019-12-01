function [xdot F] = compute_xdot_and_F__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg)
% compute_xdot_and_F__ins_ekf_quaternion: @compute_xdot_and_F function for Quaternion-based INS
%
% Function generates state derivatives vector, xdot, and linearized 
% state dynamics matrix, F.  This function is used within: 
%    perform_state_estimation__ins_ekf_quaternion.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [xdot F] = compute_xdot_and_F__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg)
%
%   NOTE: mag_declination_deg is not used in this routine,
%         but it is used in the sister routine
%         compute_zhat_and_H__ins_ekf_quaternion()
%
% Inputs:
%   x:             16x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   accel_fb_mps2: 3x1 Accelerometer measurements, m/s2
%   g_mps2:        Magnitude of gravity, m/s2
%   mag_declination_deg: Angle between true north and magnetic north, deg
%
% Outputs:
%   xdot:   16x1 state derivatives vector,  xdot = dx/dt
%   F:      16x16 Linaerized state dynamics matrix, F = d(xdot)/dx 
%           (F is the Jacobian of the vector xdot with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix F can sometimes be difficult.  The
%   expression for F utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of F
%   by calling the function "compute_xdot_and_F__ins_ekf_quaternion" without any arguments.
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

% References:
%   1  Barton, J. D., “Fundamentals of Small Unmanned Aircraft Flight,” 
%      Johns Hopkins APL Technical Digest, Volume 31, Number 2 (2012).
%      http://www.jhuapl.edu/techdigest/TD/td3102/

% If compute_xdot_and_F is called without any arguments, use the Symbolic Toolbox
% to compute and display the linearized state dynamics matrix, F.  This is
% provided merely as a helper to the interested user.
if nargin==0
    % Derive F using the Symbolic Toolbox
    derive_F
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

% Angular rate measurement from gyros
wx=gyro_wb_rps(1); % rad/s
wy=gyro_wb_rps(2);
wz=gyro_wb_rps(3);

% Specific force measurement from accelerometers
fx=accel_fb_mps2(1); % m/s^2
fy=accel_fb_mps2(2);
fz=accel_fb_mps2(3);

% 4x3 conversion from body attitute rates to quaternion rates
%
% Body rates [wx;wy;wz] are represented as quaternion rates via:
%                          [  0 -wx -wy -wz ][q0]
%   d(quaternion)/dt = 0.5*[ wx   0  wz -wy ][q1]
%                          [ wy -wz   0  wx ][q2]
%                          [ wz  wy -wx   0 ][q3]
%
% The above matrix math can be manipulated to form:
%   d(quaternion)/dt = C_bodyrate2qdot*[wx; wy; wz]
C_bodyrate2qdot  = .5*[-q1  -q2  -q3; ...
                        q0  -q3   q2; ...
                        q3   q0  -q1; ...
                       -q2   q1   q0];

% Direction Cosine Matrix (DCM) from body cooridinates to NED coordinates
% expressed using quaternions.  
% Method: Follow algorithm in Section E of rotation_examples.m to form
%   C_ned2b from a quaternion, then transpose C_ned2b to 
%   aquire C_b2ned: C_b2ned = C_ned2b'.
C_ned2b  = [ 1-2*(q2^2+q3^2)    2*(q1*q2+q3*q0)     2*(q1*q3-q2*q0); ...
             2*(q1*q2-q3*q0)    1-2*(q1^2+q3^2)     2*(q2*q3+q1*q0); ...
             2*(q1*q3+q2*q0)    2*(q2*q3-q1*q0)     1-2*(q1^2+q2^2)];
C_b2ned=transpose(C_ned2b);

% Compute state dynamics vector, xdot = dx/dt
% Akin to Equation 24 in Reference 1.
xdot = [ ...
        C_bodyrate2qdot*([wx;wy;wz]-[bwx;bwy;bwz]); ...      % Derivative of [q0; q1; q2; q3]
        [Vn; Ve; -Vd]; ...                                   % Derivative of [Pn; Pe; Alt]
        C_b2ned*([fx;fy;fz]-[bax;bay;baz])+[0;0;g_mps2]; ... % Derivative of [Vn; Ve; Vd]
        [0;0;0]; ...                                         % Derivative of [bwx; bwy; bwz]
        [0;0;0]; ...                                         % Derivative of [bax; bay; baz]
       ];

% Compute linearized state dynamics, F = d(xdot)/dx
% F can easily be aquired by differentiating "xdot" using the Symbolic
% Toolbox.  See the sub-function "derive_F" below for example.
F = [ ...
    [                                 0,                                        bwx/2 - wx/2,                                        bwy/2 - wy/2,                                        bwz/2 - wz/2, 0, 0, 0, 0, 0,  0,  q1/2,  q2/2,  q3/2,                   0,                   0,                   0]
    [                      wx/2 - bwx/2,                                                   0,                                        wz/2 - bwz/2,                                        bwy/2 - wy/2, 0, 0, 0, 0, 0,  0, -q0/2,  q3/2, -q2/2,                   0,                   0,                   0]
    [                      wy/2 - bwy/2,                                        bwz/2 - wz/2,                                                   0,                                        wx/2 - bwx/2, 0, 0, 0, 0, 0,  0, -q3/2, -q0/2,  q1/2,                   0,                   0,                   0]
    [                      wz/2 - bwz/2,                                        wy/2 - bwy/2,                                        bwx/2 - wx/2,                                                   0, 0, 0, 0, 0, 0,  0,  q2/2, -q1/2, -q0/2,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 1, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 1,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0, -1,     0,     0,     0,                   0,                   0,                   0]
    [ 2*q3*(bay - fy) - 2*q2*(baz - fz),                 - 2*q2*(bay - fy) - 2*q3*(baz - fz), 4*q2*(bax - fx) - 2*q1*(bay - fy) - 2*q0*(baz - fz), 2*q0*(bay - fy) + 4*q3*(bax - fx) - 2*q1*(baz - fz), 0, 0, 0, 0, 0,  0,     0,     0,     0, 2*q2^2 + 2*q3^2 - 1,   2*q0*q3 - 2*q1*q2, - 2*q0*q2 - 2*q1*q3]
    [ 2*q1*(baz - fz) - 2*q3*(bax - fx), 4*q1*(bay - fy) - 2*q2*(bax - fx) + 2*q0*(baz - fz),                 - 2*q1*(bax - fx) - 2*q3*(baz - fz), 4*q3*(bay - fy) - 2*q0*(bax - fx) - 2*q2*(baz - fz), 0, 0, 0, 0, 0,  0,     0,     0,     0, - 2*q0*q3 - 2*q1*q2, 2*q1^2 + 2*q3^2 - 1,   2*q0*q1 - 2*q2*q3]
    [ 2*q2*(bax - fx) - 2*q1*(bay - fy), 4*q1*(baz - fz) - 2*q3*(bax - fx) - 2*q0*(bay - fy), 2*q0*(bax - fx) - 2*q3*(bay - fy) + 4*q2*(baz - fz),                 - 2*q1*(bax - fx) - 2*q2*(bay - fy), 0, 0, 0, 0, 0,  0,     0,     0,     0,   2*q0*q2 - 2*q1*q3, - 2*q0*q1 - 2*q2*q3, 2*q1^2 + 2*q2^2 - 1]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    [                                 0,                                                   0,                                                   0,                                                   0, 0, 0, 0, 0, 0,  0,     0,     0,     0,                   0,                   0,                   0]
    ];    

return



% Helper function to show how to use the Symbolic Toolbox to derive F,
% the linearized state dynamics matrix.  This sub-function is accessed when
% this file is called without any arguments.  This routine prints the 
% resulting F, using symbolics, to the Matlab console.
function derive_F

% Test whether user has symbolic toolbox
try
    sym('testSymbolic');
catch
    error(['An error occurred creating a symbolic expression.  ' ...
           'Calling this routine without any arguments to symbolically derive ' ...
           'the matrix F is only supported if you have the Symbolics Toolbox.  ' ...
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

% Call compute_xdot_and_F using symbolic inputs to make a
% symbolic xdot
xdot = compute_xdot_and_F__ins_ekf_quaternion(x,gyro_wb_rps,accel_fb_mps2,g_mps2,mag_declination_deg);

% Differentiate xdot symbolically (Same as: F = jacobian(xdot,x);)
for n=1:length(x)
    F(:,n) = diff(xdot,x(n)); % Derivative of xdot with regard to nth state variable
end
   
% Display F
fprintf('\nLinearized state dynamics matrix, F\n')
F

assignin('base','F',F);

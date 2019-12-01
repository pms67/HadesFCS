function [xdot F] = compute_xdot_and_F__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg)
% compute_xdot_and_F__ahrs_ekf_quaternion: @compute_xdot_and_F function for quaternion-based AHRS
%
% Function generates state derivatives vector, xdot, and linearized 
% state dynamics matrix, F.  This function is used within: 
%    perform_state_estimation__ahrs_ekf_quaternion.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [xdot F] = compute_xdot_and_F__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg)
%
%   NOTE: vb_ref_mps, g_mps2 & mag_declination_deg are not used in this routine,
%         but they are used in the sister routine
%         compute_zhat_and_H__ahrs_ekf_quaternion()
%
% Inputs:
%   x:             7x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   vb_ref_mps:    3x1 reference velocity vector in body coords, m/s
%   g_mps2:        Magnitude of gravity, m/s2
%   mag_declination_deg: Angle between true north and magnetic north, deg
%
% Outputs:
%   xdot:   7x1 state derivatives vector,  xdot = dx/dt
%   F:      7x7 Linaerized state dynamics matrix, F = d(xdot)/dx 
%           (F is the Jacobian of the vector xdot with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix F can sometimes be difficult.  The
%   expression for F utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of F
%   by calling the function "compute_xdot_and_F__ahrs_ekf_quaternion" without any arguments.
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

% Compute state dynamics vector, xdot = dx/dt
% Akin to Equation 20 in Reference 1.
xdot = [ ...
        C_bodyrate2qdot*([wx;wy;wz]-[bwx;bwy;bwz]); ...    % Derivative of [q0; q1; q2; q3]
        [0;0;0]; ...                                       % Derivative of [bwx; bwy; bwz]
       ];

% Compute linearized state dynamics, F = d(xdot)/dx
% F can easily be aquired by differentiating "xdot" using the Symbolic
% Toolbox.  See the sub-function "derive_F" below for example.
F = [ ...
    [            0, bwx/2 - wx/2, bwy/2 - wy/2, bwz/2 - wz/2,  q1/2,  q2/2,  q3/2]
    [ wx/2 - bwx/2,            0, wz/2 - bwz/2, bwy/2 - wy/2, -q0/2,  q3/2, -q2/2]
    [ wy/2 - bwy/2, bwz/2 - wz/2,            0, wx/2 - bwx/2, -q3/2, -q0/2,  q1/2]
    [ wz/2 - bwz/2, wy/2 - bwy/2, bwx/2 - wx/2,            0,  q2/2, -q1/2, -q0/2]
    [            0,            0,            0,            0,     0,     0,     0]
    [            0,            0,            0,            0,     0,     0,     0]
    [            0,            0,            0,            0,     0,     0,     0]
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

% Call compute_xdot_and_F using symbolic inputs to make a
% symbolic xdot
xdot = compute_xdot_and_F__ahrs_ekf_quaternion(x,gyro_wb_rps,vb_ref_mps,g_mps2,mag_declination_deg);

% Differentiate xdot symbolically (Same as: F = jacobian(xdot,x);)
for n=1:length(x)
    F(:,n) = diff(xdot,x(n)); % Derivative of xdot with regard to nth state variable
end
   
% Display F
fprintf('\nLinearized state dynamics matrix, F\n')
F

assignin('base','F',F);

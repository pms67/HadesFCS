function [zhat H] = compute_zhat_and_H__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2)
% compute_zhat_and_H__ahrs_ekf_euler: @compute_zhat_and_H function for Euler-based AHRS
%
% Function recreates measurement vector, zhat, and linearized 
% measurement matrix, H.  This function is used within: 
%    perform_state_estimation__ahrs_ekf_euler.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [zhat H] = compute_zhat_and_H__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2)
%
% Inputs:
%   x:             6x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   vb_ref_mps:    3x1 reference velocity vector in body coords, m/s
%   g_mps2:        Magnitude of gravity, m/s2
%
% Outputs:
%   zhat:   4x1 recreation of measurement vector using x,
%              where: zhat = h(x,...) = [fx; fy; fz; psi]=[accel_fb_mps2; psi];
%   H:      4x6 Linerized state dynamics matrix, H = d(zhat)/dx
%           (H is the Jacobian of the vector zhat with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix H can sometimes be difficult.  The
%   expression for H utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of H
%   by calling the function "compute_zhat_and_H__ahrs_ekf_euler" without any arguments.
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
%  x = [phi; theta; psi; bwx; bwy; bwz];
n=1;
phi=x(n);n=n+1;  % Roll, Pitch, Yaw Euler angles, rad
theta=x(n);n=n+1;% 
psi=x(n); n=n+1; % 
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

% Direction Cosine Matrix (DCM) from NED coordinates to body 
% coordinates expressed using Euler angles.
% (See Sections A & B in rotation_examples.m for C_ned2b derivation.)
C_ned2b  = [cos(theta)*cos(psi)                               cos(theta)*sin(psi)                              -sin(theta); ...
            sin(phi)*sin(theta)*cos(psi)-cos(phi)*sin(psi)    sin(phi)*sin(theta)*sin(psi)+cos(phi)*cos(psi)   sin(phi)*cos(theta); ...
            cos(phi)*sin(theta)*cos(psi)+sin(phi)*sin(psi)    cos(phi)*sin(theta)*sin(psi)-sin(phi)*cos(psi)   cos(phi)*cos(theta)];    

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
%                  [yaw]; ...
%                 ];
% Akin to Equation 21 in Reference 1.
zhat = [ ...
        [fx; fy; fz]; ...             % [Accel. specific force measurement]
        [psi]; ...                    % [2D magnetometer yaw measurement]
       ];

% Compute linerized state dynamics matrix, H = d(zhat)/dx.
% H was derived a priori using the Symbolic Toolbox, as in the subfunction
% "derive_H".
H = [ ...
    [                           0,          g_mps2*cos(theta), 0,   0, -Vz,  Vy]
    [ -g_mps2*cos(phi)*cos(theta), g_mps2*sin(phi)*sin(theta), 0,  Vz,   0, -Vx]
    [  g_mps2*cos(theta)*sin(phi), g_mps2*cos(phi)*sin(theta), 0, -Vy,  Vx,   0]
    [                           0,                          0, 1,   0,   0,   0]
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
phi    = sym('phi');
theta  = sym('theta');
psi    = sym('psi');
bwx    = sym('bwx');
bwy    = sym('bwy');
bwz    = sym('bwz');
x = [phi; theta; psi; bwx; bwy; bwz];

% Symbolically define gyro measurement
syms wx wy wz 
gyro_wb_rps=[wx; wy; wz];

% Symbolically definine reference velocity vector (body coords)
syms Vx Vy Vz
vb_ref_mps=[Vx; Vy; Vz];

% Symbolically define gravity in m/s^2
syms g_mps2

% Call compute_xdot_and_F using symbolic inputs to make a
% symbolic xdot
zhat = compute_zhat_and_H__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2);

% Differentiate zhat symbolically (Same as: H = jacobian(zhat,x);)
for n=1:length(x)
    H(:,n) = diff(zhat,x(n)); % Derivative of zhat wrt nth state variable
end
   
% Display H
fprintf('\nLinearized measurement matrix, H\n')
H

assignin('base','H',H);

function [xdot F] = compute_xdot_and_F__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2)
% compute_xdot_and_F__ahrs_ekf_euler: @compute_xdot_and_F function for Euler-based AHRS
%
% Function generates state derivatives vector, xdot, and linearized 
% state dynamics matrix, F.  This function is used within: 
%    perform_state_estimation__ahrs_ekf_euler.m. 
% Specifically, this function is used as an input to perform_ekf().
%
% Usage:
%   [xdot F] = compute_xdot_and_F__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2)
%
%   NOTE: vb_ref_mps & g_mps2 are not used in this routine,
%         but they are used in the sister routine
%         compute_zhat_and_H__ahrs_ekf_euler()
%
% Inputs:
%   x:             6x1 State vector (see function body for details)
%   gyro_wb_rps:   3x1 Gyro measurements, rad/s
%   vb_ref_mps:    3x1 reference velocity vector in body coords, m/s
%   g_mps2:        Magnitude of gravity, m/s2
%
% Outputs:
%   xdot:   6x1 state derivatives vector,  xdot = dx/dt
%   F:      6x6 Linaerized state dynamics matrix, F = d(xdot)/dx 
%           (F is the Jacobian of the vector xdot with respect to the vector x)
%
% Note:
%   Deriving the linearized matrix F can sometimes be difficult.  The
%   expression for F utilized in this method was derived a priori using the 
%   Symbolic Toolbox.  The user can recreate the symbolic derivation of F
%   by calling the function "compute_xdot_and_F__ins_ekf_euler" without any arguments.
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
%  x = [phi; theta; psi; bwx; bwy; bwz];
n=1;
phi=x(n); n=n+1; % Roll, Pitch, Yaw Euler angles, rad
theta=x(n);n=n+1;% 
psi=x(n); n=n+1; % 
bwx=x(n); n=n+1; % Gyro biases, rad/s
bwy=x(n); n=n+1; % 
bwz=x(n); n=n+1; % 

% Angular rate measurement from gyros
wx=gyro_wb_rps(1); % rad/s
wy=gyro_wb_rps(2);
wz=gyro_wb_rps(3);

% 3x3 conversion from body attitute rates to euler rates
%
% To derive, start with body rates ([wx;wy;wz]) from euler rates:
%   [wx]   [d(phi)/dt]           [     0     ]                     [    0    ]
%   [wy] = [    0    ] + Cx(phi)*[d(theta)/dt] + Cx(phi)*Cy(theta)*[    0    ]
%   [wz]   [    0    ]           [     0     ]                     [d(psi)/dt]
%
%          [      -sin(theta)          0         1 ][ d(phi)/dt ]
%        = [ cos(theta)*sin(phi)   cos(phi)      0 ][d(theta)/dt]
%          [ cos(theta)*cos(phi)  -sin(phi)      0 ][ d(psi)/dt ]
%
% Then, inverting the above yields:
%   [ d(phi)/dt ]   [1      sin(phi)*tan(theta)    cos(phi)*tan(theta)][wx]
%   [d(theta)/dt] = [0           cos(phi)                -sin(phi)    ][wy]
%   [ d(psi)/dt ]   [0      sin(phi)*sec(theta)    cos(phi)*sec(theta)][wz]
%
% Forming:  d(eulers)/dt = C_bodyrate2eulerdot*[wx; wy; wz]
C_bodyrate2eulerdot  = [1      sin(phi)*tan(theta)    cos(phi)*tan(theta); ...
                        0           cos(phi)                -sin(phi)    ; ...
                        0      sin(phi)*sec(theta)    cos(phi)*sec(theta)];

% Compute state dynamics vector, xdot = dx/dt
% Akin to Equation 20 in Reference 1.
xdot = [ ...
        C_bodyrate2eulerdot*([wx;wy;wz]-[bwx;bwy;bwz]); ...% Derivative of [roll; pitch; yaw]
        [0;0;0]; ...                                       % Derivative of [bwx; bwy; bwz]
       ];

% Compute linearized state dynamics, F = d(xdot)/dx
% F can easily be aquired by differentiating "xdot" using the Symbolic
% Toolbox.  See the sub-function "derive_F" below for example.
F = [ ...
    [     sin(phi)*tan(theta)*(bwz - wz) - cos(phi)*tan(theta)*(bwy - wy),               - cos(phi)*(bwz - wz)*(tan(theta)^2 + 1) - sin(phi)*(bwy - wy)*(tan(theta)^2 + 1), 0, -1, -sin(phi)*tan(theta), -cos(phi)*tan(theta)]
    [                           cos(phi)*(bwz - wz) + sin(phi)*(bwy - wy),                                                                                               0, 0,  0,            -cos(phi),             sin(phi)]
    [ (sin(phi)*(bwz - wz))/cos(theta) - (cos(phi)*(bwy - wy))/cos(theta), - (cos(phi)*sin(theta)*(bwz - wz))/cos(theta)^2 - (sin(phi)*sin(theta)*(bwy - wy))/cos(theta)^2, 0,  0, -sin(phi)/cos(theta), -cos(phi)/cos(theta)]
    [                                                                   0,                                                                                               0, 0,  0,                    0,                    0]
    [                                                                   0,                                                                                               0, 0,  0,                    0,                    0]
    [                                                                   0,                                                                                               0, 0,  0,                    0,                    0]
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
xdot = compute_xdot_and_F__ahrs_ekf_euler(x,gyro_wb_rps,vb_ref_mps,g_mps2);

% Differentiate xdot symbolically (Same as: F = jacobian(xdot,x);)
for n=1:length(x)
    F(:,n) = diff(xdot,x(n)); % Derivative of xdot with regard to nth state variable
end
   
% Display F
fprintf('\nLinearized state dynamics matrix, F\n')
F

assignin('base','F',F);

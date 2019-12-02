% UAV Rotation Mathematics Examples
%
% The purpose of this file is present the various rotation methods utilized
% within the SUAS state estimation code (see uav_state_estimation.m), and
% show the conversions between the rotation representations.
%
% There are many ways to represent rotations.  The methods used in the SUAS
% state estimation code are:
%
%     - Euler Angles: [yaw; pitch; roll] of UAV, defining the 
%       transformation from the inertial North-East-Down frame to the 
%       UAV body x-y-z frame.  Rotation order: Z-Y-X, or yaw-about-z,
%       then pitch-about-y, then roll-about-x.
%
%     - Direction Cosine Matrix (DCM): 3x3 matrix representing the
%       Z-Y-X Euler Angle rotation described above. (Akin to the
%       default rotation order for ANGLE2DCM in the Matlab Aerospace 
%       Toolbox.)  Example:  C_ned2b = Cx(roll)*Cy(pitch)*Cz(yaw).
%
%     - Quaternions: q=q0+i*q1+j*q2+k*q3, representing the Z-Y-X Euler
%       Angle rotation described above. (Akin to the
%       default rotation order for ANGLE2QUAT in the Matlab Aerospace 
%       Toolbox.)
%
% The examples herein are specifically for rotations between the
% North-East-Down coordinate frame and the UAV body x-y-z
% (forward-right-down) coordinate frame.  Rotations between other
% coordinate frames can be accomplished in similar manners.
%
% The rotation examples provided in the script are given in different
% sections:
%   Section A: Converting from Euler angles to a DCM
%   Section B: Converting from Euler angles to a DCM (2nd method)
%   Section C: Converting from DCM to Euler angles
%   Section D: Converting from Euler angles to a quaternion
%   Section E: Converting from a quaternion to a DCM
%   Section F: Converting from a quaternion to Euler angles
%   Section G: Convertion from DCM to a quaternion
%
% A table of the included conversion methods is below.  The letters A-G
% refer to the sections in this mfile.  (XXXXX implies no conversion is
% necessary.)
%
%                     From    From    From
%                     Euler   DCM     Quaternion
%                   .-----------------------.
%          To Euler | XXXXX |   C   |   E   |
%                   |-------+-------+-------|
%            To DCM |  A,B  | XXXXX |   G   |
%                   |-------+-------+-------|
%     To Quaternion |   D   |   F   | XXXXX |
%                   '-----------------------'
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

clc
disp('The purpose of rotation_examples.m is present the various rotation methods utilized')
disp('within the SUAS state estimation code (see uav_state_estimation.m), and')
disp('show the conversions between the rotation representations.')
disp('Edit rotation_examples.m for the details of each conversion method.')
disp(' ')

%% Start with some Euler angles

    % Yaw, Pitch, Roll Euler angles (in degrees) representing the
    % transformation from the NED frame to the body x-y-z frame.
    yaw_deg   = -82.7;
    pitch_deg = 62.9; 
    roll_deg  = -7.04;

    fprintf(['Initial Euler Angles:\n' ...
             '  [yaw pitch roll] = [%f %f %f] degrees\n\n'], ...
             yaw_deg, pitch_deg, roll_deg);

%% Section A: Converting from Euler angles to a DCM

    % First, convert Euler angles from degrees to radians
    phi   = roll_deg*pi/180;    % \  Greek letters [phi theta psi] are 
    theta = pitch_deg*pi/180;   %  ) often used to represent
    psi   = yaw_deg*pi/180;     % /  [roll pitch yaw]

    % Direction Cosine Matrix (DCM) from NED cooridinates to body coordinates
    % expressed using Euler angles.
    % Note: This is the equivalent to the default 'ZYX' transformation used
    %   in ANGLE2DCM in the Matlab Aerospace Toolbox: 
    %          C_ned2b = angle2dcm(psi,theta,phi);
    
    %   1st Euler rotation: Rotation of yaw(psi) about z axis
    Cz_psi   = [cos(psi)    sin(psi)    0 ; ...
               -sin(psi)    cos(psi)    0 ; ...
                0           0           1 ];
            
    %   2nd Euler rotation: Rotation of pitch(theta) about y axis
    Cy_theta = [cos(theta)  0          -sin(theta); ...
                0           1           0         ; ...
                sin(theta)  0           cos(theta)];
            
    %   3rd Euler rotation: Rotation of roll(phi) about x axis
    Cx_phi   = [1           0           0        ; ...
                0           cos(phi)    sin(phi) ; ...
                0          -sin(phi)    cos(phi) ];

    % Combining the three rotations above yields C_ned2b, the DCM
    % expressing the transformation from the NED frame to the body frame.
    % Note: Rotation order is important!  Cz_psi is on the right because it
    %   is the first rotaion, followed by rotations about y, then x.
    C_ned2b = Cx_phi*Cy_theta*Cz_psi;        

    fprintf(['Section A: Converting from Euler angles to a DCM yields:\n' ...
             '  C_ned2b =\n']);
    disp(C_ned2b)

%% Section B: Converting from Euler angles to a DCM (2nd method)

    % The NED-to-body Direction Cosine Matrix (C_ned2b) can also be 
    % computed directly by writing out the matrix manipulations expressed
    % in C_ned2b = Cx_phi*Cy_theta*Cz_psi.
    % Note: This is the equivalent to the default 'ZYX' transformation used
    %   in ANGLE2DCM in the Matlab Aerospace: 
    %          C_ned2b = angle2dcm(psi,theta,phi);
    C_ned2b  = [cos(theta)*cos(psi)                               cos(theta)*sin(psi)                             -sin(theta)         ; ...
                sin(phi)*sin(theta)*cos(psi)-cos(phi)*sin(psi)    sin(phi)*sin(theta)*sin(psi)+cos(phi)*cos(psi)   sin(phi)*cos(theta); ...
                cos(phi)*sin(theta)*cos(psi)+sin(phi)*sin(psi)    cos(phi)*sin(theta)*sin(psi)-sin(phi)*cos(psi)   cos(phi)*cos(theta)];
    
    fprintf(['Section B: Alternate method for converting from Euler angles to a DCM yields:\n' ...
             '  C_ned2b =\n']);
    disp(C_ned2b)
    
%% Section C: Converting from DCM to Euler angles

    % To extract roll, pitch, and yaw estimates from DCM, recall the DCM
    % formulation in Section B:
    %            [ cos(theta)cos(psi)   cos(theta)sin(psi)       -sin(theta)    ]
    %  C_ned2b = [        . . .              . . .           sin(phi)cos(theta) ]
    %            [        . . .              . . .           cos(phi)cos(theta) ]
    %
    % Note: the lower left of C_ned2b is not relevant to
    %   the nominal Euler angle extraction.  It will be used for the 
    %   pitch = +/-90 degree case, however (see below).
    % Note: This is the equivalent to the default 'ZYX' transformation used
    %   in ANGLE2DCM in the Matlab Aerospace: 
    %          C_ned2b = angle2dcm(psi,theta,phi);
    
    % Extract phi:
    %
    %    C_ned2b(2,3)   sin(phi)*cos(theta)   sin(phi)
    %    ------------ = ------------------- = -------- = tan(phi)
    %    C_ned2b(3,3)   cos(phi)*cos(theta)   cos(phi)
    %
    phi   = atan2(C_ned2b(2,3),C_ned2b(3,3));
    
    % Extract theta
    %
    %    C_ned2b(1,3) = -sin(theta)
    %
    theta = asin( -C_ned2b(1,3) );
    
    % Extract psi
    %
    %    C_ned2b(1,2)   cos(theta)*sin(psi)   sin(psi)
    %    ------------ = ------------------- = -------- = tan(psi)
    %    C_ned2b(1,1)   cos(theta)*cos(psi)   cos(psi)
    %
    psi   = atan2(C_ned2b(1,2),C_ned2b(1,1)); % -pi <= psi <= pi
    
    % The above conversions work as long as cos(theta) isn't zero.
    % (dividing by zero is bad.)  "cos(theta)=0" corresponds to the case 
    % when pitch is +/-90 degrees.  We can test for this case by checking
    % if the magnitude of the "C_ned2b(1,3)=-sin(theta)" case is "near"
    % unity.  (Recall that sin(theta) = +/-1 when cos(theta)=0.) We'll use a 
    % buffer of 1e-8 to account for computational precision.
    if abs(C_ned2b(1,3)) > 1 - 1e-8
        % Pitch=+/-90deg case.  Underdetermined, so assume phi(roll) is zero,
        % and solve for theta(pitch) and psi(yaw) as follows:
        %
        %  Solution for theta, assuming phi=0:
        %
        %    C_ned2b(1,3)      -sin(theta)       -sin(theta)
        %    ------------ = ------------------ = ------------ = -tan(theta)
        %    C_ned2b(3,3)   cos(phi)cos(theta)   1*cos(theta)
        %
        %  Solution for psi, assuming phi=0:
        %
        %    Using:
        %                [              . . .                                                 . . .                          . . . ]
        %     C_ned2b  = [sin(phi)*sin(theta)*cos(psi)-cos(phi)*sin(psi)    sin(phi)*sin(theta)*sin(psi)+cos(phi)*cos(psi)   . . . ]
        %                [              . . .                                                 . . .                          . . . ]
        %
        %    C_ned2b(2,1)   sin(phi)*sin(theta)*cos(psi)-cos(phi)*sin(psi)   0-1*sin(psi)
        %    ------------ = ---------------------------------------------- = ------------ = -tan(psi) 
        %    C_ned2b(2,2)   sin(phi)*sin(theta)*sin(psi)+cos(phi)*cos(psi)   0+1*cos(psi)
        %
        phi   = 0;
        theta = atan2( -C_ned2b(1,3), C_ned2b(3,3) );
        psi   = atan2( -C_ned2b(2,1), C_ned2b(2,2) );
    end

    % Then, convert from radians to degrees
    yaw_deg   = psi*180/pi;
    pitch_deg = theta*180/pi;
    roll_deg  = phi*180/pi;
    
    fprintf(['Section C: Converting from a DCM to Euler angles yields:\n' ...
             '  [yaw pitch roll] = [%f %f %f] degrees\n\n'], ...
             yaw_deg, pitch_deg, roll_deg);
    
%% Section D: Converting from Euler angles to a quaternion

    % Quaternions are 4-element entities (q=q0+i*q1+j*q2+k*q3) representing 
    % a transformation between coordinate frames.  The 4 elements of the
    % quaternion combine to represent a 3-dimensional vector and a rotation
    % angle about that vector:  
    %
    %    q = [   q0           q1            q2            q3      ]
    %
    %    q = [cos(ang/2) ux*sin(ang/2) uy*sin(ang/2) uz*sin(ang/2)]
    %
    %    where the transformation is defined as a rotation of "ang" degrees
    %    about the unit vector [ux; uy; uz].  (Note: norm(q)=1.)
    %
    % Any transformation between two frames can be represented by a single 
    % rotation about some vector.  Thus, a quaternion is a compact 
    % representation of a rotation. (Note that q and -q represent the same
    % rotation!)
    %
    % Note: As shown here, this is the equivalent to the default 
    %   'ZYX' transformation used in ANGLE2QUAT in the Matlab Aerospace: 
    %          q_ned2b = angle2quat(psi,theta,phi);
    
    % First, convert Euler angles from degrees to radians
    phi   = roll_deg*pi/180;    % \  Greek letters [phi theta psi] are 
    theta = pitch_deg*pi/180;   %  ) often used to represent
    psi   = yaw_deg*pi/180;     % /  [roll pitch yaw]

    % Convert from Euler angles to a quaternion (q_ned2b is a 1x4 vector)
    q_ned2b = [ cos(psi/2)*cos(theta/2)*cos(phi/2) + sin(psi/2)*sin(theta/2)*sin(phi/2), ...
                cos(psi/2)*cos(theta/2)*sin(phi/2) - sin(psi/2)*sin(theta/2)*cos(phi/2), ...
                cos(psi/2)*sin(theta/2)*cos(phi/2) + sin(psi/2)*cos(theta/2)*sin(phi/2), ...
                sin(psi/2)*cos(theta/2)*cos(phi/2) - cos(psi/2)*sin(theta/2)*sin(phi/2)];

    fprintf(['Section D: Converting from a Euler angles to a quaternion yields:\n' ...
             '  q_ned2b = [%f %f %f %f]\n\n'],q_ned2b);
    
%% Section E: Converting from a quaternion to a DCM

    % Convert from the 4-element quaternion to the 3x3 DCM.
    % Note: This is the equivalent to QUAT2DCM in the Matlab Aerospace: 
    %          q_ned2b = dcm2quat(C_ned2b);    
    q0=q_ned2b(1); % Quaternion scalar 
    q1=q_ned2b(2); % \ 
    q2=q_ned2b(3); %  ) Quaternion vector 
    q3=q_ned2b(4); % /
    C_ned2b  = [ 1-2*(q2^2+q3^2)    2*(q1*q2+q3*q0)     2*(q1*q3-q2*q0); ...
                 2*(q1*q2-q3*q0)    1-2*(q1^2+q3^2)     2*(q2*q3+q1*q0); ...
                 2*(q1*q3+q2*q0)    2*(q2*q3-q1*q0)     1-2*(q1^2+q2^2)];

    fprintf(['Section E: Converting from a quaternion to a DCM yields:\n' ...
             '  C_ned2b =\n']);
    disp(C_ned2b)

%% Section F: Convertion from DCM to a quaternion

    % There are less computationally intensive methods for converting a
    % DCM to a quaternion, but here we will merely combine the
    % conversions in Sections C and D.
    % Note: The result is equivalent to DCM2QUAT in the Matlab Aerospace: 
    %          q_ned2b = dcm2quat(C_ned2b);    

    % Use method in Section C to convert from DCM to Euler angles, using
    % the 'ZYX' rotation order
    phi   = atan2(C_ned2b(2,3),C_ned2b(3,3));
    theta = asin( -C_ned2b(1,3) );
    psi   = atan2(C_ned2b(1,2),C_ned2b(1,1)); % -pi <= psi <= pi    
    if abs(C_ned2b(1,3)) > 1 - 1e-8
        % Pitch=+/-90deg case.  Underdetermined, so assume phi(roll) is zero,
        % and solve for theta(pitch) and psi(yaw) as follows:
        phi   = 0;
        theta = atan2( -C_ned2b(1,3), C_ned2b(3,3) );
        psi   = atan2( -C_ned2b(2,1), C_ned2b(2,2) );
    end
    
    % Use the method in Section D to convert from Euler angles to a
    % quaternion, using the 'ZYX' rotation order
    q_ned2b = [ cos(psi/2)*cos(theta/2)*cos(phi/2) + sin(psi/2)*sin(theta/2)*sin(phi/2), ...
                cos(psi/2)*cos(theta/2)*sin(phi/2) - sin(psi/2)*sin(theta/2)*cos(phi/2), ...
                cos(psi/2)*sin(theta/2)*cos(phi/2) + sin(psi/2)*cos(theta/2)*sin(phi/2), ...
                sin(psi/2)*cos(theta/2)*cos(phi/2) - cos(psi/2)*sin(theta/2)*sin(phi/2)];

    fprintf(['Section F: Converting from a DCM to a quaternion yields:\n' ...
             '  q_ned2b = [%f %f %f %f]\n\n'],q_ned2b);    

%% Section G: Converting from a quaternion to Euler angles
    
    % There are less computationally intensive methods for converting a
    % quaternion to Euler angles, but here we will merely combine the
    % conversions in Sections E and C.
    % Note: The result is equivalent to QUAT2ANGLE, with the default 'ZYX'
    %   rotation order, in the Matlab Aerospace: 
    %          [psi theta phi] = quat2angle(q_ned2b);    
    
    % Use method in Section E to convert from quaternion to DCM
    q0=q_ned2b(1); % Quaternion scalar 
    q1=q_ned2b(2); % \ 
    q2=q_ned2b(3); %  ) Quaternion vector 
    q3=q_ned2b(4); % /
    C_ned2b  = [ 1-2*(q2^2+q3^2)    2*(q1*q2+q3*q0)     2*(q1*q3-q2*q0); ...
                 2*(q1*q2-q3*q0)    1-2*(q1^2+q3^2)     2*(q2*q3+q1*q0); ...
                 2*(q1*q3+q2*q0)    2*(q2*q3-q1*q0)     1-2*(q1^2+q2^2)];
             
    % Use method in Section C to convert from DCM to Euler angles, using
    % the 'ZYX' rotation order
    phi   = atan2(C_ned2b(2,3),C_ned2b(3,3));
    theta = asin( -C_ned2b(1,3) );
    psi   = atan2(C_ned2b(1,2),C_ned2b(1,1)); % -pi <= psi <= pi    
    if abs(C_ned2b(1,3)) > 1 - 1e-8
        % Pitch=+/-90deg case.  Underdetermined, so assume phi(roll) is zero,
        % and solve for theta(pitch) and psi(yaw) as follows:
        phi   = 0;
        theta = atan2( -C_ned2b(1,3), C_ned2b(3,3) );
        psi   = atan2( -C_ned2b(2,1), C_ned2b(2,2) );
    end
    
    % Then, convert from radians to degrees
    yaw_deg   = psi*180/pi;
    pitch_deg = theta*180/pi;
    roll_deg  = phi*180/pi;
    
    fprintf(['Section G: Converting from a quaternion to Euler angles yields:\n' ...
             '  [yaw pitch roll] = [%f %f %f] degrees\n\n'], ...
             yaw_deg, pitch_deg, roll_deg);


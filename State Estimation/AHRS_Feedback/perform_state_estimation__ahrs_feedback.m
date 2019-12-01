% UAV state estimation example: AHRS using feedback controller
%
% This routine is an implementation of the feedback controller variant of 
% the "GPS with IMU-based AHRS" state estimation method described in
% Figure 9 of Reference 1.  Specifically, this method uses a 
% proportional-integral (PI) controller to estimate gyro biases based on a 
% body-frame attitude error vector (eb).  eb is derived by noting that if 
% the output attitude estimate were correct and other error sources were 
% negligible, then the estimated North and Down directions would align with 
% those observed from a yaw reference (e.g. from a magnetometer) and a 
% body-frame gravity vector reference (derived by centripetally correcting 
% the accelerometer measurement).  More detailed information of the
% feedback controller state estimation method can be found in 
% References 2 and 3.
%
% This routine loops through time to perform state estimation using the UAV
% sensor measurements provided in the uavSensors structure.  (This routine 
% is meant to be executed via the uav_state_estimation.m script.)
%
% References:
%   1  Barton, J. D., “Fundamentals of Small Unmanned Aircraft Flight,” 
%      Johns Hopkins APL Technical Digest, Volume 31, Number 2 (2012).
%      http://www.jhuapl.edu/techdigest/TD/td3102/
%   2  Premerlani, W., and Bizard, P., “Direction Cosine Matrix IMU:
%      Theory,” http://gentlenav.googlecode.com/files/DCMDraft2.pdf
%   3  Euston, M., Coote, P., Mahony, R., Kim, J., and Hamel, T., 
%      "A Complementary Filter for Attitude Estimation of a Fixed-Wing UAV,"
%      in International Conf. on Intelligent Robots and Systems, 
%      Nice, France, pp. 340–345 (2008).
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
%         - Quaternions: q=q0+i*q1+j*q2+k*q3, representing the Z-Y-X Euler
%           Angle rotation described above. (Akin to the
%           default rotation order for ANGLE2QUAT in the Matlab Aerospace 
%           Toolbox.)
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
% Initialize Feedback Controller, orientation and gyro bias estimates
%   State estimates
%     - Use magnetometer to provide an initial state estimate.
%     - This method uses Euler angles to represent the attitude of
%       the UAV body frame relative to the fixed North-East-Down frame. 
%       (The NED-to-body Euler angles are often denoted yaw, pitch,
%       and roll.)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    
ahrsFeedback.yaw_est_deg   = uavSensors.mag2D_yaw_deg(1);% Initial Yaw estimates, degrees 
ahrsFeedback.pitch_est_deg = 0;                          % Initial Pitch estimates, degrees 
ahrsFeedback.roll_est_deg  = 0;                          % Initial Roll estimates, degrees 
ahrsFeedback.gyro_bias_rps = [0;0;0];                    % Init XYZ gyro bias estimates, rad/s

% Initialize feedback controller integral component.  "e_b" is the
% attitude error vector in body coordinates.  Thus, "e_b_int" is the
% integral of "e_b" (i.e. the accumulation of the e_b vector over
% time).
ahrsFeedback.e_b_int = [0; 0; 0]; % Integral of error vector, body coordinates


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
    % Translational state estimation (position and velocity)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    if(GPS_valid)
        % When GPS is valid, use GPS position and velocity estimates
        east_est_m = GPS_east_m;
        north_est_m= GPS_north_m;
        h_msl_est_m= GPS_h_msl_m;
        v_ned_est_mps = GPS_v_ned_mps;
    else
        % When GPS isn't valid, assume constant velocity and propagate 
        % position estimates.
        % (Alternatively, a UAV with an AHRS might just keep the position 
        % estimate constant between GPS measurements.)
        north_est_m = north_est_m + dt_s*v_ned_est_mps(1);
        east_est_m  = east_est_m  + dt_s*v_ned_est_mps(2);
        h_msl_est_m = h_msl_est_m + dt_s*(-v_ned_est_mps(3)); % <- Note: 3rd element of v_ned is down velocity 
    end

    % Generate a "reference" velocity vector in body coordinates.  This is
    % used in accounting for centripetal acceleration effects (see Ref 1).
    %
    % Note: We can't directly use v_ned_est_mps because we don't yet know
    % the UAV orientation (hence how to rotate NED velocity into body
    % coordinates.
    %
    % Often, the y- and z- components of the reference velocity are
    % neglected, and only the x-component (i.e. the axial component) of the
    % velocity vector is accounted for.  The axial velocity can be roughly
    % estimated using either the pitot tube airspeed measurement or the
    % horizontal inertial velocity estimate from GPS.  We'll use the pitot.
    vb_ref_mps = [pitot_airspeed_mps; 0; 0];  

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Rotational state estimation (yaw, pitch and roll)
    %
    % Estimate Direction Cosine Matrix (DCM) from NED coordinates to body 
    % coordinates expressed by multiplying individual Euler rotation
    % matrices.
    %   C_ned2b = Cx(roll)*Cy(pitch)*Cz(yaw);  (Eqn. 23a in Ref 1)
    %
    % The described method to convert from Euler angles to a DCM follows
    % the method shown in Section A of rotation_examples.m.
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%    

    % 1st Euler rotation: Rotation of yaw estimate about z axis
    yaw_est_rad = pi/180*ahrsFeedback.yaw_est_deg;
    Cz_yaw_est    = [cos(yaw_est_rad)    sin(yaw_est_rad)    0                 ; ...
                    -sin(yaw_est_rad)    cos(yaw_est_rad)    0                 ; ...
                     0                   0                   1                 ];
                 
    % 2nd Euler rotation: Rotation of pitch estimate about y axis
    pitch_est_rad = pi/180*ahrsFeedback.pitch_est_deg;
    Cy_pitch_est  = [cos(pitch_est_rad)  0                  -sin(pitch_est_rad); ...
                     0                   1                   0                 ; ...
                     sin(pitch_est_rad)  0                   cos(pitch_est_rad)];
                 
    % 3rd Euler rotation: Rotation of roll estimate about x axis
    roll_est_rad = pi/180*ahrsFeedback.roll_est_deg;    
    Cx_roll_est   = [1                   0                   0                 ; ...
                     0                   cos(roll_est_rad)   sin(roll_est_rad) ; ...
                     0                  -sin(roll_est_rad)   cos(roll_est_rad) ];
    
    % Estimate DCM from NED coordinates to body coordinates using estimated
    % Euler angles
    C_ned2b_est = Cx_roll_est*Cy_pitch_est*Cz_yaw_est;

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Generate a gravity vector reference (i.e. an estimate of the gravity
    % vector, in body coordinates, based on a centripetally-corrected
    % accelerometer measurement).  (See Equation 22 in Ref. 1)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    gb_ref_mps2 = cross(gyro_wb_rps,vb_ref_mps) - accel_fb_mps2;      
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Generate a yaw reference.  This could be from a magnetometer if it is
    % available.  If not, it could be from a crab-corrected GPS course
    % angle, or simply just a raw GPS course angle.  Here, we use a 2D
    % magnetometer.  (The code could be modified to utlize the 3D
    % magnetometer estimate as well.)
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    yaw_ref_rad = pi/180*mag2D_yaw_deg;
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Generate an attitude error vector, e_b, in body coordinates.   
    % e_b represents the orientation error between the UAV's estimated
    % orientation (expressed via C_ned2b_est) and a reference orientation 
    % described by gravity and yaw "references".  The gravity vector
    % reference is acquired from a centripetally-corrected accelerometer 
    % measurement. The yaw reference can be acquired from a magnetometer or
    % other methods. (See Figure 10 in Reference 1 for details.)
    %
    % e_b is formed from a vector sum of e_b_g (attitude error vector
    % relative to gravity reference) and e_b_psi (attitude error vector
    % relative to yaw reference).
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % Form attitude error vector relative to gravity reference.
    % e_b_g is the rotation vector expressed in body coordinates between the
    % centripetally-corrected gravity direction (Down) estimated from the
    % accelerometers and the AHRS-estimated Down direction.
    % Thus, e_b_g inherently captures the pitch and roll attitude errors.
    % See Eqn. 23c in Reference 1.    
    %
    %            /  gb_ref  \       /            [ 0 ] \
    %   e_b_g = (  --------  )  x  ( C_ned2b_est*[ 0 ]  )
    %            \ |gb_ref| /       \            [ 1 ] /
    %
    %           \_____  _____/     \_________  _________/
    %                 \/                     \/
    %          Reference "Down"        AHRS-estimated
    %          direction formed       "Down" direction
    %         from centripetally     in body coordinates
    %      corrected accelerometers,
    %        in body coordinates
    %
    % Note that the cross product of two vectors yields a vector
    % perpendicular to both with a magnitude proportional to the sine of
    % the angle between them.  Thus, e_b_g is a measure of the difference, 
    % or "error", between the two "Down" direction vectors above.
    e_b_g = cross(gb_ref_mps2/norm(gb_ref_mps2), C_ned2b_est*[0;0;1]);

    % Form attitude error vector relative to yaw reference.
    % e_b_psi is the rotation vector expressed in body coordinates between
    % the observed North direction (via the yaw reference) and the
    % AHRS-estimated North direction (via the current attitude estimate).
    % See Eqn. 23b in Reference 1.
    %
    %   e_b_psi =
    %                                             
    %     /                                       [ 1 ]\     /            [ 1 ]\ 
    %    ( Cx(roll_est)*Cy(pitch_est)*Cz(yaw_ref)*[ 0 ] ) x ( C_ned2b_est*[ 0 ] )
    %     \                                       [ 0 ]/     \            [ 0 ]/
    %           
    %    \_____________________  _______________________/   \_________  ________/
    %                          \/                                     \/
    %        Observed "North" direction using the           AHRS-estimated "North"
    %         yaw reference, in body coordinates       direction, in body coordinates
    %
    % Note that the cross product of two vectors yields a vector
    % perpendicular to both with a magnitude proportional to the sine of
    % the angle between them.  Thus, e_b_psi is a measure of the difference, 
    % or "error", between the two "North" direction vectors above.
    Cz_yaw_ref    = [cos(yaw_ref_rad)    sin(yaw_ref_rad)    0 ; ...
                    -sin(yaw_ref_rad)    cos(yaw_ref_rad)    0 ; ...
                     0                   0                   1 ];
    e_b_psi=cross(Cx_roll_est*Cy_pitch_est*Cz_yaw_ref*[1;0;0], C_ned2b_est*[1;0;0]);
    
    % Form the total attitude error vector by combining e_b_g and e_b_psi.
    % The combined error vector, e_b, expresses the angular error and
    % rotation axis between the observed (or reference) NED coordinate
    % frame the and AHRS-estimated NED coordinate frame.
    % See Eqn. 23d in Reference 1.
    e_b = e_b_g + e_b_psi;
    
    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Proportional-Intetral (PI) Control
    %
    % The feedback controller is designed to minimuze the error vector (e_b)
    % between the estimated orientation and the "reference" orientation,
    % defined by an acceleromter-estimated "Down" direction and a
    % magnetometer-estimated "North" direction.  Effectively, the
    % controller estimates and removes the gyro biases that would cause the
    % integrated gyro measurements to diverge from the reference attitudes.
    % See Figure 9 in Reference 1 for details.
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
    % Use simple euler integration to accumulate e_b_int, the integral of 
    % the attitude error vector e_b. (Recall that e_b and e_b_int are
    % body-coordinate vectors.)
    ahrsFeedback.e_b_int =ahrsFeedback.e_b_int + e_b*dt_s;
    
    % Perform Proportional-Integral control using gains K_p and K_i.  The
    % selection of gains is based on gyro, accelerometer and
    % magnetometer bandwidth characteristics.  These can be selected using
    % a control systems analysis, but are more often selected through
    % trial-and-error tuning.  In practice, separate PI gains for e_b_g and 
    % e_b_psi might be used to account for the differing bandwidth 
    % characteristics of the accelerometers and the magnetometer.  
    % For simplicity, that nuance is neglected here.  
    % The resulting gyro bias vector estimate is the negative of the 
    % control signal (K_p*e_b + K_i*e_b_int).  See Figure 9 in Ref 1 for details.
    K_p = 0.3;      % Proportional gain
    K_i = 0.01;     % Integral gain
    ahrsFeedback.gyro_bias_rps = -(K_p*e_b + K_i*ahrsFeedback.e_b_int);
    
    % Use the estimated gyro biases to de-bias the raw gyro measurement.
    gyro_wb_rps_debiased = gyro_wb_rps - ahrsFeedback.gyro_bias_rps;

    
    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    % Integrate the de-biased gyro measurement to generate the attitude
    % estimate.
    %
    % There are three primary methods of integrating the debiased gyro
    % measurements: Euler-based, DCM-based, and quaternion-based.  The
    % Euler-based, as described in Reference 1, is arguably the most intuitive,
    % but degrades notably as the pitch angle approaches +/- 90 degrees.
    % Thus, the DCM or quaternion-based methods are generally used.  The
    % code below nominally uses the Euler-based method to match the
    % description in Ref 1, but implementations of the DCM-based and the 
    % quaternion-based methods are provided for the interested user.
    gyro_integration_method = 'EULER';  % Options: 'EULER', 'DCM' or 'QUATERNION'
    switch gyro_integration_method
        case 'EULER'
            
            % 3x3 conversion from body attitute rates to euler rates, as
            % per Equation 8 in Reference 1.
            % (See EKF method compute_xdot_and_F__ins_ekf_euler.m for derivation.)
            %   d(eulers)/dt = C_bodyrate2eulerdot*[wx; wy; wz]
            C_bodyrate2eulerdot  = [1      sin(roll_est_rad)*tan(pitch_est_rad)    cos(roll_est_rad)*tan(pitch_est_rad); ...
                                    0      cos(roll_est_rad)                      -sin(roll_est_rad)                   ; ...
                                    0      sin(roll_est_rad)*sec(pitch_est_rad)    cos(roll_est_rad)*sec(pitch_est_rad)];

            % Generate estimate of Euler angle rates by converting the debiased
            % body-axis gyro measurements into Euler angle rates.
            eulerDotHat_dps = 180/pi*C_bodyrate2eulerdot*gyro_wb_rps_debiased;

            % Generate the Euler angle attitude estimates by integrating
            % the estimated Euler angle rates.  (Using simple Euler
            % integration.)
            ahrsFeedback.yaw_est_deg   = ahrsFeedback.yaw_est_deg   + dt_s*eulerDotHat_dps(3);
            ahrsFeedback.pitch_est_deg = ahrsFeedback.pitch_est_deg + dt_s*eulerDotHat_dps(2);
            ahrsFeedback.roll_est_deg  = ahrsFeedback.roll_est_deg  + dt_s*eulerDotHat_dps(1);
            
        case 'DCM'
            
            % Form Skew-Symmetric Matrix version of body rates (OMEGA_ned2b_b)
            % Here, OMEGA_ned2b_b is the skew symmetric matrix expressing the
            % rotational rate of the body coordinate frame relative to the
            % inertial NED coordinate frame, expressed in the body 
            % coordinate frame.
            wx=gyro_wb_rps_debiased(1);
            wy=gyro_wb_rps_debiased(2);
            wz=gyro_wb_rps_debiased(3);
            OMEGA_ned2b_b = [  0 -wz  wy; ...
                              wz   0 -wx; ...
                             -wy  wx   0];
            
            % Form derivative of DCM: 
            %   Cdot_ned2b = d(C_ned2b)/dt = -OMEGA_ned2b_b*C_ned2b
            Cdot_ned2b = -OMEGA_ned2b_b*C_ned2b_est;
            
            % Update the DCM estimate by integrating the estimated
            % DCM rate.  (Using simple Euler integration.)
            C_ned2b_est = C_ned2b_est + Cdot_ned2b*dt_s;
            
            % Orthonormalize the DCM.
            % DCMs must be orthonormal matrices.  Orthonormal means all 3 
            % columns are mutually perpendicular (orthogonal) and have unit 
            % magnitude. For example, given 3x1 column vectors u,v and w, a 
            % 3x3 matrix C=[u v w] is orthonormal if and only if:
            %   dot(u,u)=dot(v,v)=dot(w,w)=1;   <= Unit magnitude vectors
            %   dot(u,v)=dot(v,w)=dot(u,w)=0;   <= Mutually perpendicular
            %   u=cross(v,w); \
            %   v=cross(w,u);  ) Another way of saying mutually perpendicular
            %   w=cross(u,v); /
            % But, after Euler integration of Cdot_ned2b, the resulting
            % C_ned2b_est will likely fail the orthonormal requirement.
            % Using the method in Reference 2, orthonormalize C_ned2b_est.
            u = C_ned2b_est(:,1);    % Column 1, a 3x1 vector
            v = C_ned2b_est(:,2);    % Column 2, a 3x1 vector
            dot_error = dot(u,v);    % Were it orthogonal, "dot_error" would be zero.
            u = C_ned2b_est(:,1) - dot_error/2*v;  % \ Distribute dot_error  
            v = C_ned2b_est(:,2) - dot_error/2*u;  % /  between u and v
            w = cross(u,v);  % 3rd Column must be orthogonal to first 2
            u = u/norm(u);   % \  Make u, v & w have unit magnitude.
            v = v/norm(v);   %  ) (Note: Reference 2 uses a modified
            w = w/norm(w);   % /   method which avoids norm() calls.)
            C_ned2b_est = [u v w];
            
            % Extract euler angles from DCM
            ahrsFeedback.roll_est_deg  = 180/pi * atan2(C_ned2b_est(2,3),C_ned2b_est(3,3));
            ahrsFeedback.pitch_est_deg = 180/pi * asin( -C_ned2b_est(1,3) );
            ahrsFeedback.yaw_est_deg   = 180/pi * atan2(C_ned2b_est(1,2),C_ned2b_est(1,1)); % -180 <= yaw <= 180
            
        case 'QUATERNION'
            
            % Extract current quaternion from euler angles using the method
            % shown in Section D of rotation_examples.m.
            q = [cos(yaw_est_rad/2) * cos(pitch_est_rad/2) * cos(roll_est_rad/2) + sin(yaw_est_rad/2) * sin(pitch_est_rad/2) * sin(roll_est_rad/2); ...
                 cos(yaw_est_rad/2) * cos(pitch_est_rad/2) * sin(roll_est_rad/2) - sin(yaw_est_rad/2) * sin(pitch_est_rad/2) * cos(roll_est_rad/2); ...
                 cos(yaw_est_rad/2) * sin(pitch_est_rad/2) * cos(roll_est_rad/2) + sin(yaw_est_rad/2) * cos(pitch_est_rad/2) * sin(roll_est_rad/2); ...
                 sin(yaw_est_rad/2) * cos(pitch_est_rad/2) * cos(roll_est_rad/2) - cos(yaw_est_rad/2) * sin(pitch_est_rad/2) * sin(roll_est_rad/2)];
            q0=q(1); % Quaternion scalar
            q1=q(2); % \ 
            q2=q(3); %  ) Quaternion vector 
            q3=q(4); % /
            
            % 4x3 conversion from body attitute rates to quaternion rates
            % (See EKF method compute_xdot_and_F__ins_ekf_quaternion.m for derivation.)
            %   d(quaternion)/dt = C_bodyrate2qdot*[wx; wy; wz]
            C_bodyrate2qdot  = .5*[-q1  -q2  -q3; ...
                                    q0  -q3   q2; ...
                                    q3   q0  -q1; ...
                                   -q2   q1   q0];

            % Generate estimate of quaternion rate by converting the debiased
            % body-axis gyro measurements into quaternion rates.  
            qdot = C_bodyrate2qdot*gyro_wb_rps_debiased;

            % Update the quaternion estimate by integrating the estimated
            % quaternion rate.  (Using simple Euler integration.)
            q = q + dt_s*qdot;

            % Re-scale quaternion
            %   By definition the 4-element quaternion must have unity magnitude.  So, 
            %   after the EKF update we need to make sure it still does.
            %   Force: [q0;q1;q2;q3] = [q0;q1;q2;q3]/sqrt(q0^2+q1^2+q2^2+q3^2)
            q=q/norm(q);
            
            % Convert quaternion to yaw, pitch, and roll estimates, as described in
            % Section G of rotation_examples.m.
            %   1) Convert quaternion [q0;q1;q2;q3] into the Direction Cosine Matrix
            %      from North-East-Down (NED) coordinate frame to the body coordinate
            %      frame: C_ned2b (DCM uses Z-Y-X rotation order)
            %   2) Extract roll, pitch, and yaw estimates from DCM:
            %                    [ cos(pitch)cos(yaw)   cos(pitch)sin(yaw)       -sin(pitch)    ]
            %          C_ned2b = [        . . .              . . .          sin(roll)cos(pitch) ]
            %                    [        . . .              . . .          cos(roll)cos(pitch) ]
            %                    Note: the lower left of C_ned2b is not relevant to
            %                    this euler angle extraction.
            q0=q(1); % Quaternion scalar
            q1=q(2); % \ 
            q2=q(3); %  ) Quaternion vector 
            q3=q(4); % /
            C_ned2b_est  = [ 1-2*(q2^2+q3^2)    2*(q1*q2+q3*q0)     2*(q1*q3-q2*q0); ...
                             2*(q1*q2-q3*q0)    1-2*(q1^2+q3^2)     2*(q2*q3+q1*q0); ...
                             2*(q1*q3+q2*q0)    2*(q2*q3-q1*q0)     1-2*(q1^2+q2^2)];
            ahrsFeedback.roll_est_deg  = 180/pi * atan2(C_ned2b_est(2,3),C_ned2b_est(3,3));
            ahrsFeedback.pitch_est_deg = 180/pi * asin( -C_ned2b_est(1,3) );
            ahrsFeedback.yaw_est_deg   = 180/pi * atan2(C_ned2b_est(1,2),C_ned2b_est(1,1)); % -180 <= yaw <= 180
            if abs(C_ned2b_est(1,3)) > 1 - 1e-8
                % Pitch=+/-90deg case.  Underdetermined, so assume roll is zero,
                % and solve for pitch and yaw as follows:
                ahrsFeedback.roll_est_deg   = 0;
                ahrsFeedback.pitch_est_deg  = 180/pi*atan2( -C_ned2b(1,3), C_ned2b(3,3) );
                ahrsFeedback.yaw_est_deg    = 180/pi*atan2( -C_ned2b(2,1), C_ned2b(2,2) );
            end
            
        otherwise
            error('Error in gyro_integration_method selection: %s',gyro_integration_method);
    end

    
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
    uavEst.roll_deg(kTime,:)  = ahrsFeedback.roll_est_deg;  % Roll estimate, deg
    uavEst.pitch_deg(kTime,:) = ahrsFeedback.pitch_est_deg; % Pitch estimate, deg
    uavEst.yaw_deg(kTime,:)   = ahrsFeedback.yaw_est_deg;   % Yaw estimate, deg
    uavEst.north_m(kTime,:)   = north_est_m;        % North position estimate, m (from propagated measurement)
    uavEst.east_m(kTime,:)    = east_est_m;         % East position estimate, m (from propagated measurement)
    uavEst.h_msl_m(kTime,:)   = h_msl_est_m;        % Altitude estimate, above MeanSeaLevel, m (from propagated measurement)
    uavEst.v_ned_mps(kTime,:) = v_ned_est_mps;      % NED Velocity estimate, m/s (from propagated measurement)

    % In a fashion similar to the EKF methods, we'll save a state
    % vector (called xhat) consisting of the estimated attitude and 
    % biases.  But, the Feedback method doesn't estimate uncertainties in
    % the state estimates like the EKF does, so set uavEst.P = NaN for the
    % roll, pitch, and yaw estimates, and use the gyro_bias_rps standard 
    % deviation from uavSensors for the gyro bias uncertainties.
    if kTime==1
        %Specify the indices of xhat.
        uavEst.states.phi  = 1; % Roll, Pitch, Yaw Euler angles, rad
        uavEst.states.theta= 2; % 
        uavEst.states.psi  = 3; % 
        uavEst.states.bwx  = 4; % Gyro biases, rad/s
        uavEst.states.bwy  = 5; % 
        uavEst.states.bwz  = 6; %
        
        % Allocate memory for arrays
        uavEst.xhat = zeros(length(uavTruth.time_s),6);
        uavEst.P    = zeros(length(uavTruth.time_s),6);
    end
    uavEst.xhat(kTime,:) = [ ...
        ahrsFeedback.roll_est_deg*pi/180 ...  % Roll, Pitch, Yaw Euler angles, rad
        ahrsFeedback.pitch_est_deg*pi/180 ... %
        ahrsFeedback.yaw_est_deg*pi/180 ...   %
        ahrsFeedback.gyro_bias_rps(1) ...     % Gyro biases, rad/s
        ahrsFeedback.gyro_bias_rps(2) ...     %
        ahrsFeedback.gyro_bias_rps(3) ...     %
        ];
    uavEst.P(kTime,:) = [inf inf inf [1 1 1]*uavSensors.sigmas.gyro_bias_rps];
        
    % Note progress with dots
    if mod(kTime,ceil(length(uavTruth.time_s)/40))==0
        fprintf('.');
    end
        
end % end kTime loop
fprintf('\n')

% Clean up variables
clear time_s kTime dt_s GPS_east_m GPS_north_m GPS_h_msl_m GPS_v_ned_mps GPS_valid baro_h_msl_m pitot_airspeed_mps gyro_wb_rps accel_fb_mps2 gravity_mps2 mag3D_unitVector_in_body mag2D_yaw_deg
clear east_est_m north_est_m h_msl_est_m v_ned_est_mps vb_ref_mps
clear yaw_est_rad pitch_est_rad roll_est_rad Cx_roll_est Cy_pitch_est Cz_yaw_est C_ned2b_est
clear gb_ref_mps2 yaw_ref_rad Cz_yaw_ref gyro_wb_rps_debiased
clear K_p K_i e_b e_b_g e_b_psi
clear C_bodyrate2eulerdot eulerDotHat_dps
clear wx wy wz OMEGA_ned2b_b Cdot_ned2b u v w dot_error
clear q q0 q1 q2 q3 C_bodyrate2qdot qdot 

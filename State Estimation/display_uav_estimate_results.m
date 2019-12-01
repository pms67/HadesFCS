% Display results of UAV state estimation.
%
% This routine uses the structures uavTruth, uavSensors, and uavEst to plot
% and compare true, measured and estimated position and attitude states.
% Here, attitude states are represented as the Euler angles yaw, pitch and
% roll.
%
% This routine is meant to be executed via the uav_state_estimation.m 
% script.
%
% Copyright © 2012 The Johns Hopkins University / Applied Physics Laboratory LLC.  All Rights Reserved.

% SUAS Code Version: 1.0, October 16, 2012  
% Author: Jeff Barton at JHU/APL, jeffrey.barton@jhuapl.edu
% Website: http://www.jhuapl.edu/ott/Technologies/Copyright/SuasCode.asp

% Make sure uavTruth, uavSensors & uavEst exists
if ~exist('uavTruth') || ~exist('uavSensors') || ~exist('uavEst')
    error(['This routine is meant to be run after creating the uavEst structure. ' ...
          'Try running the main routine ''uav_state_estimation.m''.']) 
end

% Set "display_errors_figure" to 1 to display errors figure.
% (This is off by default.')
display_errors_figure = 0;

% Open figure
figure(1)
clf;

% Establish Line and Marker styles
truthLineType    = 'b-';
sensedLineType   = 'gd';
estimateLineType = 'r.';
markerSize       = 6;
linewidth        = 2;

% Plot East/North Position, meters (True, GPS Measured, & Estimated)
subplot(4,2,[1 3])
plot(uavSensors.GPS_east_m, uavSensors.GPS_north_m, sensedLineType, ...
     uavEst.east_m,         uavEst.north_m,         estimateLineType, ...
     uavTruth.east_m,       uavTruth.north_m,       truthLineType, ...
     'markersize',markerSize, 'linewidth', linewidth);
hold on
plot(uavTruth.east_m(1),uavTruth.north_m(1),'co', ...
     'markersize',1.5*markerSize, 'linewidth', 2*linewidth);
hold off
axis equal
grid on
xlabel('East, m'); ylabel('North, m')
title('UAV Position')
legend('GPS Meas.','Est. Pos.','True Pos.','Start','location','best')

% Plot Altitude above Mean Sea Level, m (True, GPS Measured, & Estimated)
subplot(4,2,5)
plot(uavSensors.time_s, uavSensors.GPS_h_msl_m, sensedLineType, ...
     uavEst.time_s,     uavEst.h_msl_m,         estimateLineType, ...
     uavTruth.time_s,   uavTruth.h_msl_m,       truthLineType, ...
     'markersize',markerSize, 'linewidth', linewidth);
grid on
xlabel('Time, s'); ylabel('Altitude, above MeanSeaLevel, m')
legend('GPS Meas.','Est. Alt','True Alt','location','best')

% Plot Speed, m/s (True, GPS Measured, & Estimated)
subplot(4,2,7)
mag = @(v)(sqrt(sum(v.^2,2))); % magnitude of each row
plot(uavSensors.time_s, mag(uavSensors.GPS_v_ned_mps), sensedLineType, ...
     uavEst.time_s,     mag(uavEst.v_ned_mps),         estimateLineType, ...
     uavTruth.time_s,   mag(uavTruth.v_ned_mps),       truthLineType, ...
     'markersize',markerSize, 'linewidth', linewidth);
grid on
xlabel('Time, s'); ylabel('Inertial Speed, m/s')
legend('GPS Meas.','Est. Speed','True Speed','location','best')

% Plot Roll Angle, deg (True & Estimated)
subplot(3,2,2); 
plot(uavEst.time_s,     uavEst.roll_deg,    estimateLineType, ...
     uavTruth.time_s,   uavTruth.roll_deg,  truthLineType, ...    
     'markersize',markerSize, 'linewidth', linewidth);
grid on
xlabel('Time, s'); ylabel('Roll Angle, deg')
title('UAV Attitude: Roll')
legend('Est. Roll','True Roll','location','best')

% Plot Pitch Angle, deg (True & Estimated)
subplot(3,2,4); 
plot(uavEst.time_s,     uavEst.pitch_deg,    estimateLineType, ...
     uavTruth.time_s,   uavTruth.pitch_deg,  truthLineType, ...     
     'markersize',markerSize, 'linewidth', linewidth);
grid on
xlabel('Time, s'); ylabel('Pitch Angle, deg')
title('UAV Attitude: Pitch')
legend('Est. Pitch','True Pitch','location','best')

% Plot Yaw Angle, deg (True, Magnetometer-Measured, & Estimated)
% (Make sure angles are wrapped between -180 and 180 degrees)
subplot(3,2,6);
wrap = @(angleDeg)(mod(angleDeg+180,360)-180); % Wrap from -180deg to 180deg.
plot(uavSensors.time_s, wrap(uavSensors.mag2D_yaw_deg), sensedLineType, ...
     uavEst.time_s,     wrap(uavEst.yaw_deg),           estimateLineType, ...
     uavTruth.time_s,   wrap(uavTruth.yaw_deg),         truthLineType, ...     
     'markersize',markerSize, 'linewidth', linewidth);
grid on
xlabel('Time, s'); ylabel('Yaw Angle, deg')
title('UAV Attitude: Yaw')
legend('Magnetometer','Est. Yaw','True Yaw','location','best')

% Label figure
figure(gcf) % Bring figure to front
set(gcf,'name',sprintf('UAV State Estimation, Method %d',state_estimation_method))
disp(' ')
disp('Figure 1 shows state estimates.')

% Bonus!
% Display state errors (Set display_errors_figure above.)
if(display_errors_figure)
    figure(2); 
    clf
    
    % Plot Attitude Errors
    % If xhat state estimates are available, use them.  Otherwise, use
    % uavEst data.
    if isfield(uavEst.states,'q0')
        % q0, q1, q2, q3 states were estimted, so use estimates
        estimates = uavEst.xhat(:,[uavEst.states.q0 uavEst.states.q1 uavEst.states.q2 uavEst.states.q3]);
        uncertainties = sqrt(uavEst.P(:,[uavEst.states.q0 uavEst.states.q1 uavEst.states.q2 uavEst.states.q3]));        
        
        % Use method in Section D of rotation_examples.m to convert
        % truth from Euler angles to a quaternion
        phi   = uavTruth.roll_deg*pi/180;    % \  Greek letters [phi theta psi] are 
        theta = uavTruth.pitch_deg*pi/180;   %  ) often used to represent
        psi   = uavTruth.yaw_deg*pi/180;     % /  [roll pitch yaw]
        truths  = [ cos(psi/2).*cos(theta/2).*cos(phi/2) + sin(psi/2).*sin(theta/2).*sin(phi/2), ...
                    cos(psi/2).*cos(theta/2).*sin(phi/2) - sin(psi/2).*sin(theta/2).*cos(phi/2), ...
                    cos(psi/2).*sin(theta/2).*cos(phi/2) + sin(psi/2).*cos(theta/2).*sin(phi/2), ...
                    sin(psi/2).*cos(theta/2).*cos(phi/2) - cos(psi/2).*sin(theta/2).*sin(phi/2)];
        clear phi theta psi
        plot_names = {'q0 Error','q1 Error','q2 Error','q3 Error'};
        pos1 = [4 5 1]; % position of top axis
    elseif isfield(uavEst.states,'phi')
        % phi, theta, psi states were estimted, so use estimates
        estimates = 180/pi*uavEst.xhat(:,[uavEst.states.phi uavEst.states.theta uavEst.states.psi]);
        uncertainties = 180/pi*sqrt(uavEst.P(:,[uavEst.states.phi uavEst.states.theta uavEst.states.psi]));
        truths = [uavTruth.roll_deg uavTruth.pitch_deg uavTruth.yaw_deg];
        plot_names = {'Roll Error, deg','Pitch Error, deg','Yaw Error, deg'};
        pos1 = [3 5 1]; % position of top axis
    else
        % No attitude states estimated, so just use uavEst
        % states, with no uncertainties.
        estimates = [uavEst.roll_deg uavEst.pitch_deg uavEst.yaw_deg];
        uncertainties = ones(size(uavTruth.time_s))*[nan nan nan];
        truths = [uavTruth.roll_deg uavTruth.pitch_deg uavTruth.yaw_deg];
        plot_names = {'Roll Error, deg','Pitch Error, deg','Yaw Error, deg'};
        pos1 = [3 5 1]; % position of top axis
    end
    for n=1:length(plot_names)
        subplot(pos1(1),pos1(2),pos1(3)+pos1(2)*(n-1))
        plot(uavTruth.time_s,uncertainties(:,n)*[-1 1],'b', ...
             uavTruth.time_s,truths(:,n)-estimates(:,n),'r', ...
             'linewidth',linewidth);
        grid on;
        xlabel('Time, s'); ylabel(plot_names{n})
        % Force y scaling, but don't use first 5 seconds of uncertainties
        % (Attitude uncertainties started arbitrarily large.)
        ylim([-1.05 1.05]*max(abs([truths(:,n)-estimates(:,n); uncertainties(uavTruth.time_s>5,n)])))
        if n==1, title('Attitude'); end
    end
    
    % Plot Position Errors
    % If xhat state estimates are available, use them.  Otherwise, use
    % uavEst data.
    if isfield(uavEst.states,'Pn')
        % Pn, Pe, Alt states were estimted, so use estimates
        estimates = uavEst.xhat(:,[uavEst.states.Pn uavEst.states.Pe uavEst.states.Alt]);
        uncertainties = sqrt(uavEst.P(:,[uavEst.states.Pn uavEst.states.Pe uavEst.states.Alt]));        
    else
        % Pn, Pe, Alt states weren't estimated, so just use predicted
        % states, with no uncertainties.
        estimates = [uavEst.north_m uavEst.east_m uavEst.h_msl_m];
        uncertainties = ones(size(uavTruth.time_s))*[nan nan nan];
    end
    plot_names = {'Pn Error, m','Pe Error, m','Alt Error, m'};
    truths = [uavTruth.north_m uavTruth.east_m uavTruth.h_msl_m];
    pos1 = [3 5 2]; % position of top axis
    for n=1:length(plot_names)
        subplot(pos1(1),pos1(2),pos1(3)+pos1(2)*(n-1))
        plot(uavTruth.time_s,uncertainties(:,n)*[-1 1],'b', ...
             uavTruth.time_s,truths(:,n)-estimates(:,n),'r', ...
             'linewidth',linewidth);
        grid on;
        xlabel('Time, s'); ylabel(plot_names{n})
        ylim([-1.05 1.05]*max(abs([truths(:,n)-estimates(:,n); uncertainties(:,n)]))) % Force y scaling
        if n==1, title('Position'); end
    end

    % Plot Velocity Errors
    % If xhat state estimates are available, use them.  Otherwise, use
    % uavEst data.
    if isfield(uavEst.states,'Vn')
        % Vn, Ve, Vd states were estimted, so use estimates
        estimates = uavEst.xhat(:,[uavEst.states.Vn uavEst.states.Ve uavEst.states.Vd]);
        uncertainties = sqrt(uavEst.P(:,[uavEst.states.Vn uavEst.states.Ve uavEst.states.Vd]));        
    else
        % Vn, Ve, Vd states weren't estimated, so just use predicted
        % states, with no uncertainties.
        estimates = [uavEst.v_ned_mps]; % kx3
        uncertainties = ones(size(uavTruth.time_s))*[nan nan nan];
    end
    plot_names = {'Vn Error, m/s','Ve Error, m/s','Vd Error, m/s'};
    truths = [uavTruth.v_ned_mps]; % kx3
    pos1 = [3 5 3]; % position of top axis
    for n=1:length(plot_names)
        subplot(pos1(1),pos1(2),pos1(3)+pos1(2)*(n-1))
        plot(uavTruth.time_s,uncertainties(:,n)*[-1 1],'b', ...
             uavTruth.time_s,truths(:,n)-estimates(:,n),'r', ...
             'linewidth',linewidth);
        grid on;
        xlabel('Time, s'); ylabel(plot_names{n})
        ylim([-1.05 1.05]*max(abs([truths(:,n)-estimates(:,n); uncertainties(:,n)]))) % Force y scaling
        if n==1, title('Velocity'); end
    end
    
    % Plot Gyro Bias Errors
    % If xhat state estimates are available, use them.  Otherwise, use
    % uavEst data.
    if isfield(uavEst.states,'bwx')
        % bwx, bwy, bwz states were estimted, so use estimates
        estimates = uavEst.xhat(:,[uavEst.states.bwx uavEst.states.bwy uavEst.states.bwz]);
        uncertainties = sqrt(uavEst.P(:,[uavEst.states.bwx uavEst.states.bwy uavEst.states.bwz]));        
    else
        % bwx, bwy, bwz states weren't estimated, so just use zeros,
        % with uavSensors uncertainties.
        estimates = ones(size(uavTruth.time_s))*[0 0 0]; % kx3
        uncertainties = ones(size(uavTruth.time_s))*[uavSensors.sigmas.gyro_bias_rps*[1 1 1]];
    end
    plot_names = {'Gyro-x Bias Error, rad/s','Gyro-y Bias Error, rad/s','Gyro-z Bias Error, rad/s'};
    truths = ones(size(uavTruth.time_s))*[uavSensors.biases.gyro_wb_rps]; % kx3
    pos1 = [3 5 4]; % position of top axis
    for n=1:length(plot_names)
        subplot(pos1(1),pos1(2),pos1(3)+pos1(2)*(n-1))
        plot(uavTruth.time_s,uncertainties(:,n)*[-1 1],'b', ...
             uavTruth.time_s,truths(:,n)-estimates(:,n),'r', ...
             'linewidth',linewidth);
        grid on;
        xlabel('Time, s'); ylabel(plot_names{n})
        ylim([-1.05 1.05]*max(abs([truths(:,n)-estimates(:,n); uncertainties(:,n)]))) % Force y scaling
        if n==1, title('Gyro Biases'); end
    end
    
    % Plot Accelerometer Bias Errors
    % If xhat state estimates are available, use them.  Otherwise, use
    % uavEst data.
    if isfield(uavEst.states,'bax')
        % bax, bay, baz states were estimted, so use estimates
        estimates = uavEst.xhat(:,[uavEst.states.bax uavEst.states.bay uavEst.states.baz]);
        uncertainties = sqrt(uavEst.P(:,[uavEst.states.bax uavEst.states.bay uavEst.states.baz]));        
    else
        % bax, bay, baz states weren't estimated, so just use zeros,
        % with uavSensors uncertainties.
        estimates = ones(size(uavTruth.time_s))*[0 0 0]; % kx3
        uncertainties = ones(size(uavTruth.time_s))*[uavSensors.sigmas.accel_bias_mps2*[1 1 1]];
    end
    plot_names = {'Accel-x Bias Error, m/s2','Accel-y Bias Error, m/s2','Accel-z Bias Error, m/s2'};
    truths = ones(size(uavTruth.time_s))*[uavSensors.biases.accel_fb_mps2]; % kx3
    pos1 = [3 5 5]; % position of top axis
    for n=1:length(plot_names)
        subplot(pos1(1),pos1(2),pos1(3)+pos1(2)*(n-1))
        plot(uavTruth.time_s,uncertainties(:,n)*[-1 1],'b', ...
             uavTruth.time_s,truths(:,n)-estimates(:,n),'r', ...
             'linewidth',linewidth);
        grid on;
        xlabel('Time, s'); ylabel(plot_names{n})
        ylim([-1.05 1.05]*max(abs([truths(:,n)-estimates(:,n); uncertainties(:,n)]))) % Force y scaling
        if n==1, title('Accel Biases'); end
    end
        
    % Label figure
    figure(gcf) % Bring figure to front
    set(gcf,'name',sprintf('UAV State Estimation, Method %d: Errors',state_estimation_method))
    disp('Figure 2 shows state estimate errors.')
else
    disp('(To also display state estimate errors, set "display_errors_figure" in display_uav_estimate_results.m.)')
end

% Clean up variables from workspace
clear truthLineType sensedLineType estimateLineType markerSize linewidth
clear wrap mag
clear estimates uncertainties truths plot_names pos1 n
clear display_errors_figure
T = 0.1;
g = 9.81;

% Initialise covariance matrix
cAtt0 = 10.0;
P = diag(([1 1] * cAtt0) .^ 2);

% Process noise matrix
nProcAtt  = 0.005;
Q = diag(([1 1] * nProcAtt) .^ 2);

% Measurement noise matrix
nMeasAcc = 0.500;
R = diag(([1 1 1] * nMeasAcc) .^ 2);

% States (phi, theta)
x = zeros(2, 1);

% Arrays for plotting
t = uavSensors.time_s;
phihat   = zeros(1, length(t));
thetahat = zeros(1, length(t));

% Low-pass filter constants
lpfGyr = 0.4;
lpfAcc = 0.4;
lpfVa  = 0.2;

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;
Va = 0;

errRoll  = 0.0;
errPitch = 0.0;

for n = 1 : length(uavSensors.time_s)
    
    % Get measurements and low-pass filter them
    p = lpfGyr * p + (1 - lpfGyr) * uavSensors.gyro_wb_rps(n, 1);
    q = lpfGyr * q + (1 - lpfGyr) * uavSensors.gyro_wb_rps(n, 2);
    r = lpfGyr * r + (1 - lpfGyr) * uavSensors.gyro_wb_rps(n, 3);
    
    ax = lpfAcc * ax + (1 - lpfAcc) * uavSensors.accel_fb_mps2(n, 1);
    ay = lpfAcc * ay + (1 - lpfAcc) * uavSensors.accel_fb_mps2(n, 2);
    az = lpfAcc * az + (1 - lpfAcc) * uavSensors.accel_fb_mps2(n, 3);
       
    Va = lpfVa * Va + (1 - lpfVa) * uavSensors.pitot_airspeed_mps(n);
        
    for it = 1 : 10
    
        % Extract states
        phi = x(1); theta = x(2);
        
        % Compute trig terms
        sph = sin(phi); cph = cos(phi);
        tth = tan(theta);

        % State transition function, xdot = f(x, u)
        f = [p + sph * tth * q + cph * tth * r; ...
                       cph * q -       sph * r];

        % Update state estimate
        x = x + (T / 10.0) * f;
        
        % Update error covariance matrix
        P = P + T * (A * P + P * A' + Q);
        
        % Re-extract states
        phi = x(1); theta = x(2);
        
        % Compute trig terms
        sph = sin(phi);   cph = cos(phi);
        sth = sin(theta); cth = cos(theta); tth = sth / cth;
    
        % Compute Jacobian of f, A(x, u)
        A = [ tth * (cph * q - sph * r), (sph * q + cph * r) / (cth * cth); ...
             -sph * q - cph * r, 0 ];
        
    end
                
    % Compute accelerometer output estimates z(x, u)
    z = [ sth * (q * Va + g); ...
          cth * (r * Va - g * sph) - p * Va * sth; ...
         -cth * (q * Va + g * cph) ];                    
      
    % Jacobian of z, C(x, u)    
    C = [ 0, cth * (q * Va + g); ...
         -g * cph * cth, sth * (-r * Va + g * sph) - p * Va * cth; ...
          g * sph * cth, (q * Va + g * cph) * sth];
         
    % Kalman gain
    K = P * C' / (C * P * C' + R);
    
    % Update error covariance matrix
    P = (eye(length(x)) - K * C) * P;
        
    % Update state estimate using measurements (accelerometer and
    % magnetometer)
    x = x + K * ([ax; ay; az] - z);
                   
    % Store state estimates
    phihat(n)   = x(1) * 180 / pi;
    thetahat(n) = x(2) * 180 / pi;
    
    % Sum errors
    errRoll  = errRoll  + abs(phihat(n) - uavTruth.roll_deg(n));
    errPitch = errPitch + abs(thetahat(n) - uavTruth.pitch_deg(n));
    
end

errRoll  = errRoll  / length(t);
errPitch = errPitch / length(t);

subplot(2, 1, 1)
plot(t, uavTruth.roll_deg, t, phihat)
title('Roll Angle')
legend('True', 'Estimate')

subplot(2, 1, 2)
plot(t, uavTruth.pitch_deg, t, thetahat)
title('Pitch Angle')
legend('True', 'Estimate')
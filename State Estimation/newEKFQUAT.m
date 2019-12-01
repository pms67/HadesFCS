T = 0.1;
g = 9.81;
magDecRad = uavSensors.params.mag_declination_deg * pi / 180.0;

% Initialise covariance matrix
cAtt0 = 0.001;
cBias0 = 0.0001;
P = diag([[1 1 1 1] * cAtt0, [1 1 1] * cBias0] .^ 2);

% Process noise matrix
nProcAtt  = 0.00005;
nProcBias = 0.000001;
Q = diag([[1 1 1 1] * nProcAtt, [1 1 1] * nProcBias] .^ 2);

% Measurement noise matrix
nMeasAcc = 0.05;
nMeasMag = 0.02;
R = diag([[1 1 1] * nMeasAcc, [1 1 1] * nMeasMag] .^ 2);

% States (q0, q1, q2, q3, bp, bq, br)
x = zeros(7, 1);
x(1) = 1.0;

% Arrays for plotting
t = uavSensors.time_s;
phihat   = zeros(1, length(t));
thetahat = zeros(1, length(t));
psihat   = zeros(1, length(t));
bphat    = zeros(1, length(t));
bqhat    = zeros(1, length(t));
brhat    = zeros(1, length(t));

% Low-pass filter constants
lpfGyr = 0.7;
lpfAcc = 0.9;
lpfMag = 0.4;
lpfVa  = 0.7;

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;
mx = 0; my = 0; mz = 0;
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
    
    mx = lpfMag * mx + (1 - lpfMag) * uavSensors.mag3D_unitVector_in_body(n, 1);
    my = lpfMag * my + (1 - lpfMag) * uavSensors.mag3D_unitVector_in_body(n, 2);
    mz = lpfMag * mz + (1 - lpfMag) * uavSensors.mag3D_unitVector_in_body(n, 3);
    
    mnorm = sqrt(mx * mx + my * my + mz * mz);
    mx = mx / mnorm;
    my = my / mnorm;
    mz = mz / mnorm;
    
    Va = lpfVa * Va + (1 - lpfVa) * uavSensors.pitot_airspeed_mps(n);
        
    for it = 1 : 10
    
        % Extract states
        q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
        bp = x(5); bq = x(6); br = x(7);

        % State transition function, xdot = f(x, u)
        C_body2qdot = 0.5 * [-q1, -q2, -q3; ...
                              q0, -q3,  q2; ...
                              q3,  q0, -q1; ...
                             -q2,  q1,  q0];
                         
        f = [ C_body2qdot * [p - bp; q - bq; r - br]; 0; 0; 0];

        % Update state estimate
        x = x + (T / 10.0) * f;
        
    end
    
    % Normalise quaternion
    qNorm = sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) / qNorm;
    x(2) = x(2) / qNorm;
    x(3) = x(3) / qNorm;
    x(4) = x(4) / qNorm;
    
    % Re-extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
    bp = x(5); bq = x(6); br = x(7);   
    
    % Compute Jacobian of f, A(x, u)
    A = [0, -0.5 * (p - bp), -0.5 * (q - bq), -0.5 * (r - br), 0.5 * q1, 0.5 * q2, 0.5 * q3; ...
         0.5 * (p - bp), 0, 0.5 * (r - br), -0.5 * (q - bq), -0.5 * q0, 0.5 * q3, -0.5 * q2; ...
         0.5 * (q - bq), -0.5 * (r - br), 0, 0.5 * (p - bp), -0.5 * q3, -0.5 * q0, 0.5 * q1; ...
         0.5 * (r - br), 0.5 * (q - bq), -0.5 * (p - bp), 0, 0.5 * q2, -0.5 * q1, -0.5 * q0; ...
         0, 0, 0, 0, 0, 0, 0; ...
         0, 0, 0, 0, 0, 0, 0; ...
         0, 0, 0, 0, 0, 0, 0];
         
    % Update error covariance matrix
    P = P + T * (A * P + P * A' + Q);
    
    % Compute magnetic field unit vector estimate in body coordinates from
    % quaternion estimates
    smag = sin(magDecRad);
    cmag = cos(magDecRad);
    
    magUnitEstimate = [ smag * (2 * q0 * q3 + 2 * q1 * q2) - cmag * (2 * q2 * q2 + 2 * q3 * q3 - 1); ...
                       -cmag * (2 * q0 * q3 - 2 * q1 * q2) - smag * (2 * q1 * q1 + 2 * q3 * q3 - 1); ...
                        cmag * (2 * q0 * q2 + 2 * q1 * q3) - smag * (2 * q0 * q1 - 2 * q2 * q3)];
    
    % Compute accelerometer output estimates
    % Note: assuming here that u = Va, v = 0, w = 0
    % Would be good to set u = Va * cos(theta), v = 0, w = Va * sin(theta)
    % But need expressions for cos(theta) as quaternions...
    
    accEstimate = [-2 * g * (q1 * q3 - q2 * q0); ...
                    Va * (r - br) - 2 * g * (q2 * q3 + q1 * q0); ...
                   -Va * (q - bq) - g * (1 - 2 * (q1 * q1 + q2 * q2))];
                    
    % Output function z(x, u) = [axhat, ayhat, azhat, mxhat, myhat, mzhat]
    z = [accEstimate; ...
         magUnitEstimate];
      
    % Jacobian of z, C(x, u)    
    C = [ 2 * g * q2, -2 * g * q3,  2 * g * q0, -2 * g * q1, 0, 0, 0; ...
         -2 * g * q1, -2 * g * q0, -2 * g * q3, -2 * g * q2, 0, 0, -Va; ...
          0, 4 * g * q1, 4 * g * q2, 0, 0, Va, 0; ...
          2 * q3 * smag, 2 * q2 * smag, 2 * q1 * smag - 4 * q2 * cmag, 2 * q0 * smag - 4 * q3 * cmag, 0, 0, 0; ...
         -2 * q3 * cmag, 2 * q2 * cmag - 4 * q1 * smag, 2 * q1 * cmag, -2 * q0 * cmag - 4 * q3 * smag, 0, 0, 0; ...
          2 * q2 * cmag - 2 * q1 * smag, 2 * q3 * cmag - 2 * q0 * smag, 2 * q0 * cmag + 2 * q3 * smag, 2 * q1 * cmag + 2 * q2 * smag, 0, 0, 0];
 
    % Kalman gain
    K = P * C' / (C * P * C' + R);
    
    % Update error covariance matrix
    P = (eye(length(x)) - K * C) * P;
        
    % Update state estimate using measurements (accelerometer and
    % magnetometer)
    x = x + K * ([ax; ay; az; mx; my; mz] - z);
    
    % Normalise quaternion
    qNorm = sqrt(x(1) * x(1) + x(2) * x(2) + x(3) * x(3) + x(4) * x(4));
    x(1) = x(1) / qNorm;
    x(2) = x(2) / qNorm;
    x(3) = x(3) / qNorm;
    x(4) = x(4) / qNorm;
             
    % Re-extract states
    q0 = x(1); q1 = x(2); q2 = x(3); q3 = x(4);
      
    % Store state estimates
    phihat(n)   = atan2(2 * (q0 * q1 + q2 * q3), q0 * q0 + q3 * q3 - q1 * q1 - q2 * q2) * 180 / pi;
    thetahat(n) = asin( 2 * (q0 * q2 - q1 * q3)) * 180 / pi;
    psihat(n)   = atan2(2 * (q0 * q3 + q1 * q2), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 180 / pi;
    bphat(n)    = x(5) * 180 / pi;
    bqhat(n)    = x(6) * 180 / pi;
    brhat(n)    = x(7) * 180 / pi;
    
    % Sum errors
    errRoll  = errRoll  + abs(phihat(n) - uavTruth.roll_deg(n));
    errPitch = errPitch + abs(thetahat(n) - uavTruth.pitch_deg(n));
    
end

errRoll  = errRoll  / length(t);
errPitch = errPitch / length(t);

subplot(3, 1, 1)
plot(t, uavTruth.roll_deg, t, phihat)
title('Roll Angle')
legend('True', 'Estimate')

subplot(3, 1, 2)
plot(t, uavTruth.pitch_deg, t, thetahat)
title('Pitch Angle')
legend('True', 'Estimate')

subplot(3, 1, 3)
plot(t, uavTruth.yaw_deg, t, psihat)
title('Yaw Angle')
legend('True', 'Estimate')
T = 0.1;
g = 9.81;
magDeclinationRad = uavSensors.params.mag_declination_deg * pi / 180.0;

% Initialise covariance matrix
cAtt0 = 0.1;
cYaw0 = 1.0;
cBias0 = 0.01;
P = diag([[1 1] * cAtt0, cYaw0, [1 1 1] * cBias0] .^ 2);

% Process noise matrix
nProcAtt  = 0.01;
nProcBias = 0.00001;
Q = diag([[1 1 1] * nProcAtt, [1 1 1] * nProcBias] .^ 2);


% Measurement noise matrix
nMeasAcc = 0.05;
nMeasMag = 22.0;
R = diag([nMeasAcc, nMeasAcc, nMeasAcc, nMeasMag] .^ 2);

% States (phi, theta, psi, bp, bq, br)
x = zeros(6, 1);

% Arrays for plotting
t = uavSensors.time_s;
phihat   = zeros(1, length(t));
thetahat = zeros(1, length(t));
psihat   = zeros(1, length(t));
psihatmag = zeros(1, length(t));
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
        phi = x(1); theta = x(2);
        bp  = x(4); bq    = x(5); br  = x(6);

        % Compute common trig terms
        sph = sin(phi);   cph = cos(phi);
        cth = cos(theta); tth = tan(theta);

        % State transition function, xdot = f(x, u)
        f = [(p - bp) + sph * tth * (q - bq) + cph * tth * (r - br); ...
                              cth * (q - bq) -       sph * (r - br); ...
                        sph / cth * (q - bq) + cph / cth * (r - br); ...
             0; 0; 0];

        % Update state estimate
        x = x + (T / 10.0) * f;
        
    end
    
    % Re-extract states
    phi = x(1); theta = x(2); psi = x(3);
    bp  = x(4); bq    = x(5); br  = x(6);
   
    % Re-compute common trig terms
    sph = sin(phi);   cph = cos(phi);
    sth = sin(theta); cth = cos(theta); tth = tan(theta);
    
    % Compute Jacobian of f, A(x, u)
    A = [ tth * (cph * (q - bq) - sph * (r - br)), (sph * (q - bq) + cph * (r - br)) / (cth * cth), 0, -1, -sph * tth, -cph * tth; ...
         -sph * (q - bq) - cph * (r - br), 0, 0, 0, -cth, sph; ...
         (cph * (q - bq) - sph * (r - br)) / cth, tth / cth * (sph * (q - bq) + cph * (r - br)), 0, 0, -sph / cth, -cph / cth; ...
          0, 0, 0, 0, 0, 0; ...
          0, 0, 0, 0, 0, 0; ...
          0, 0, 0, 0, 0, 0];
     
    % Update error covariance matrix
    P = P + T * (A * P + P * A' + Q);
    
    % Output function z(x, u) = [ax, ay, az, psiMag]
    z = [ sth * (Va * (q - bq) + g); ...
          Va * cth * (r - br) - Va * sth * (p - bp) - g * sph * cth; ...
         -Va * cth * (q - bq) - g * cph * cth; ...
          psi];
      
    % Jacobian of z, C(x, u)
    C = [0, cth * (Va * (q - bq) + g), 0, 0, -Va * sth, 0; ...
         -g * cph * cth, -Va * (r - br) * sth - Va * (p - bp) * cth + g * sph * sth, 0, Va * sth, 0, -Va * cth; ...
         g * sph * cth, sth * (Va * (q - bq) + g * cph), 0, 0, Va * cth, 0; ...
         0, 0, 1, 0, 0, 0];
     
    % Kalman gain
    K = P * C' / (C * P * C' + R);
    
    % Update error covariance matrix
    P = (eye(length(x)) - K * C) * P;
    
    % Calculate heading angle from magnetometer
    psiMag = atan2(-my * cph + mz * sph, (mx * cth + sth * (my * sph + mz * cph))) + magDeclinationRad;   
    
    % Update state estimate using measurements
    x = x + K * ([ax; ay; az; psiMag] - z);
             
    % Store state estimates
    phihat(n)   = x(1) * 180 / pi;
    thetahat(n) = x(2) * 180 / pi;
    psihat(n)   = x(3) * 180 / pi;
    psihatmag(n) = psiMag * 180 / pi;
    bphat(n)    = x(4) * 180 / pi;
    bqhat(n)    = x(5) * 180 / pi;
    brhat(n)    = x(6) * 180 / pi;

    
end

subplot(3, 1, 1)
plot(t, uavTruth.roll_deg, t, phihat)
title('Roll Angle')
legend('True', 'Estimate')

subplot(3, 1, 2)
plot(t, uavTruth.pitch_deg, t, thetahat)
title('Pitch Angle')
legend('True', 'Estimate')

subplot(3, 1, 3)
plot(t, uavTruth.yaw_deg, t, psihat, t, psihatmag)
title('Yaw Angle')
legend('True', 'Estimate', 'Magnetometer')
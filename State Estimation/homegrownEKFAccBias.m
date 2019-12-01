initStateCov = 0.0001;
initGBiasCov = 0.0001;
initABiasCov = 0.0001;

processNoiseState = 1e-1;
processNoiseGBias = 1e-6;
processNoiseABias = 1e-5;

measNoiseAcc = 2;
measNoisePsi = 0.1;

lpfGyrCoeff = 0.8;
lpfAccCoeff = 0.9;
lpfVaCoeff = 0.75;


P = diag([ones(3,1)*initStateCov; ones(3,1)*initGBiasCov; ones(3,1)*initABiasCov]);
Q = diag([ones(3,1)*processNoiseState; ones(3,1)*processNoiseGBias; ones(3,1)*processNoiseABias]);
R = diag([ones(3,1)*measNoiseAcc; measNoisePsi]);
xhat = zeros(9, 1);

dt = 0.1;
g = 9.81;

N = length(uavTruth.time_s);

t = uavTruth.time_s';
roll = uavTruth.roll_deg;
rollhat = zeros(1, N);
pitch = uavTruth.pitch_deg;
pitchhat = zeros(1, N);
altitudehat = zeros(1, N);

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;
V = 0;

figure
hold on
for n=1:N

    p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * uavSensors.gyro_wb_rps(n, 3);
    
    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * uavSensors.accel_fb_mps2(n, 3);
    
    V = lpfVaCoeff * V + (1 - lpfVaCoeff) * uavSensors.pitot_airspeed_mps(n);
   
    mx = uavSensors.mag3D_unitVector_in_body(n, 1);
    my = uavSensors.mag3D_unitVector_in_body(n, 1);
    mz = uavSensors.mag3D_unitVector_in_body(n, 1);   
    
    pressure = uavTruth.pressure_Npm2(n, 1);
    altitudehat(n) = -8576.6 * log(pressure / 101325);
    
    phi = xhat(1);
    theta = xhat(2);
    psi = xhat(3);
    bp = xhat(4);
    bq =  xhat(5);
    br =  xhat(6);
    bx = xhat(7);
    by = xhat(8);
    bz = xhat(9);
       
    psimeas = atan2(-my * cos(phi) + mz * sin(phi), mx * cos(theta) + (my * sin(phi) + mz * cos(phi)) * sin(theta)) + 50 * pi / 180;
    
    for IT=1:10
        phi = xhat(1);
        theta = xhat(2);
        psi = xhat(3);
        bp = xhat(4);
        bq =  xhat(5);
        br =  xhat(6);
        bx = xhat(7);
        by = xhat(8);
        bz = xhat(9);
        
        % State transition function
        f =  [p - bp - cos(phi)*tan(theta)*(br - r) - sin(phi)*tan(theta)*(bq - q); ...
                                    sin(phi)*(br - r) - cos(phi)*(bq - q); ...
        - (sin(phi)*(bq - q))/cos(theta) - (cos(phi)*(br - r))/cos(theta); ...
                                                                        0; ...
                                                                        0; ...
                                                                        0; ...
                                                                        0; ...
                                                                        0; ...
                                                                        0];                                   

        % Jacobian of f    
        A = [sin(phi)*tan(theta)*(br - r) - cos(phi)*tan(theta)*(bq - q),               - cos(phi)*(br - r)*(tan(theta)^2 + 1) - sin(phi)*(bq - q)*(tan(theta)^2 + 1), 0, -1, -sin(phi)*tan(theta), -cos(phi)*tan(theta), 0, 0, 0; ...
                               cos(phi)*(br - r) + sin(phi)*(bq - q),                                                                                           0, 0,  0,            -cos(phi),             sin(phi), 0, 0, 0; ...
     (sin(phi)*(br - r))/cos(theta) - (cos(phi)*(bq - q))/cos(theta), - (cos(phi)*sin(theta)*(br - r))/cos(theta)^2 - (sin(phi)*sin(theta)*(bq - q))/cos(theta)^2, 0,  0, -sin(phi)/cos(theta), -cos(phi)/cos(theta), 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0; ...
                                                                   0,                                                                                           0, 0,  0,                    0,                    0, 0, 0, 0];


        xhat = xhat + (dt / 10) * f;
        P = P + (dt / 10) * (A * P + P * A' + Q);
    end
                                  
    % Jacobian of h   
    C = [                      0,                                  g*cos(theta) - V*cos(theta)*(bq - q), 0,            0, -V*sin(theta),             0, -1,  0,  0; ...
 -g*cos(phi)*cos(theta), V*cos(theta)*(bp - p) + V*sin(theta)*(br - r) + g*sin(phi)*sin(theta), 0, V*sin(theta),             0, -V*cos(theta),  0, -1,  0; ...
  g*cos(theta)*sin(phi),                         g*cos(phi)*sin(theta) - V*sin(theta)*(bq - q), 0,            0,  V*cos(theta),             0,  0,  0, -1; ...
                      0,                                                                     0, 1,            0,             0,             0,  0,  0,  0];
                  
                  
    K = P * C' / (C * P * C' + R);
    P = (eye(9) - K * C) * P;
        
    % Output function h
    %h = [                         g*sin(theta); ...
    %         - V*(br - r) - g*cos(theta)*sin(phi); ...
    %     V*(bq - q) - g*cos(phi)*cos(theta); ...
    %                              psi];
    
    h = [g*sin(theta) - V*sin(theta)*(bq - q) - bx; ...
 V*sin(theta)*(bp - p) - V*cos(theta)*(br - r) - g*cos(theta)*sin(phi) - by; ...
                         V*cos(theta)*(bq - q) - g*cos(phi)*cos(theta) - bz; ...
                         psi];
                              
    xhat = xhat + K * ([ax - bx; ay - by; az - bz; psimeas] - h);

    rollhat(n) = xhat(1) * 180 / pi;
    pitchhat(n) = xhat(2) * 180 / pi;
    yawhat(n) = xhat(3) * 180 / pi;
    biasp(n) = xhat(4) * 180 / pi;
    biasq(n) = xhat(5) * 180 / pi;
    biasr(n) = xhat(6) * 180 / pi;
    biasx(n) = xhat(7);
    biasy(n) = xhat(8);
    biasz(n) = xhat(9);    
end

subplot(3,2,1)
plot(t, roll, t, rollhat, 'r-.')
legend('True', 'Estimate')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Roll')

subplot(3,2,2)
plot(t, abs(roll'-rollhat))
xlabel('Time (s)')
ylabel('Estimation Error (deg)')
title('Absolute Roll Angle Estimation Error')

subplot(3,2,3)
plot(t, pitch, t, pitchhat, 'r-.')
legend('True', 'Estimate')
xlabel('Time (s)')
ylabel('Angle (deg)')
title('Pitch')

subplot(3,2,4)
plot(t, abs(pitch'-pitchhat))
xlabel('Time (s)')
ylabel('Estimation Error (deg)')
title('Absolute Pitch Angle Estimation Error')

subplot(3,2,5)
plot(t, biasp, t, biasq, t, biasr)
legend('p', 'q', 'r')
xlabel('Time (s)')
ylabel('Bias (deg/s)')
title('Gyroscope Bias Estimates')

subplot(3,2,6)
plot(t, biasx, t, biasy, t, biasz)
legend('x', 'y', 'z')
xlabel('Time (s)')
ylabel('Bias (m/s^2)')
title('Accelerometer Bias Estimates')

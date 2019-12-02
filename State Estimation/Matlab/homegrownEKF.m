
P = eye(6) * 0.01;
Q = diag([0.1, 0.1, 0.1, 0.0000001, 0.0000001, 0.0000001]);
R = eye(4) * 0.1;
xhat = [0.0; 0.0; 0.0; ...
        0.0; ...
        0.0; ...
        0.0];

dt = 0.1;
g = 9.81;

N = length(uavTruth.time_s);

t = uavTruth.time_s';
roll = uavTruth.roll_deg;
rollhat = zeros(1, N);
pitch = uavTruth.pitch_deg;
pitchhat = zeros(1, N);

p = 0; q = 0; r = 0;
ax = 0; ay = 0; az = 0;
V = 0;

lpfGyrCoeff = 0.8;
lpfAccCoeff = 0.8;
lpfVaCoeff = 0.75;

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
    
    phi = xhat(1);
    theta = xhat(2);
    psi = xhat(3);
    bp = xhat(4);
    bq =  xhat(5);
    br =  xhat(6);
       
    psimeas = atan2(-my * cos(phi) + mz * sin(phi), mx * cos(theta) + (my * sin(phi) + mz * cos(phi)) * sin(theta)) + 50 * pi / 180;
    
    % State transition function
    f =  [p - bp - cos(phi)*tan(theta)*(br - r) - sin(phi)*tan(theta)*(bq - q); ...
                                sin(phi)*(br - r) - cos(phi)*(bq - q); ...
    - (sin(phi)*(bq - q))/cos(theta) - (cos(phi)*(br - r))/cos(theta); ...
                                                                    0; ...
                                                                    0; ...
                                                                    0];                                   
                                                                                  
    % Jacobian of f    
    A = [     sin(phi)*tan(theta)*(br - r) - cos(phi)*tan(theta)*(bq - q),               - cos(phi)*(br - r)*(tan(theta)^2 + 1) - sin(phi)*(bq - q)*(tan(theta)^2 + 1), 0, -1, -sin(phi)*tan(theta), -cos(phi)*tan(theta); ...
                           cos(phi)*(br - r) + sin(phi)*(bq - q),                                                                                           0, 0,  0,            -cos(phi),             sin(phi); ...
(sin(phi)*(br - r))/cos(theta) - (cos(phi)*(bq - q))/cos(theta), - (cos(phi)*sin(theta)*(br - r))/cos(theta)^2 - (sin(phi)*sin(theta)*(bq - q))/cos(theta)^2, 0,  0, -sin(phi)/cos(theta), -cos(phi)/cos(theta); ...
                                                               0,                                                                                           0, 0,  0,                    0,                    0; ...
                                                              0,                                                                                           0, 0,  0,                    0,                    0; ...
                                                              0,                                                                                           0, 0,  0,                    0,                    0];
              
    % Discretize     
    xhat = xhat + dt * f;
    P = P + dt * (A * P + P * A' + Q);
                                  
    % Jacobian of h
 %   C =  [                      0,          g*cos(theta), 0, 0, 0,  0; ...
% -g*cos(phi)*cos(theta), g*sin(phi)*sin(theta), 0, 0, 0, -V; ...
%  g*cos(theta)*sin(phi), g*cos(phi)*sin(theta), 0, 0, V,  0; ...
%                      0,                     0, 1, 0, 0,  0];
    
    C = [                      0,                                  g*cos(theta) - V*cos(theta)*(bq - q), 0,            0, -V*sin(theta),             0; ...
 -g*cos(phi)*cos(theta), V*cos(theta)*(bp - p) + V*sin(theta)*(br - r) + g*sin(phi)*sin(theta), 0, V*sin(theta),             0, -V*cos(theta); ...
  g*cos(theta)*sin(phi),                         g*cos(phi)*sin(theta) - V*sin(theta)*(bq - q), 0,            0,  V*cos(theta),             0; ...
                      0,                                                                     0, 1,            0,             0,             0];
                  
                  
    K = P * C' / (C * P * C' + R);
    P = (eye(6) - K * C) * P;
        
    % Output function h
    %h = [                         g*sin(theta); ...
    %         - V*(br - r) - g*cos(theta)*sin(phi); ...
    %     V*(bq - q) - g*cos(phi)*cos(theta); ...
    %                              psi];
    
    h = [g*sin(theta) - V*sin(theta)*(bq - q); ...
 V*sin(theta)*(bp - p) - V*cos(theta)*(br - r) - g*cos(theta)*sin(phi); ...
                         V*cos(theta)*(bq - q) - g*cos(phi)*cos(theta); ...
                         psi];
                              
    xhat = xhat + K * ([ax; ay; az; psimeas] - h);

    rollhat(n) = xhat(1) * 180 / pi;
    pitchhat(n) = xhat(2) * 180 / pi;
    yawhat(n) = xhat(3) * 180 / pi;
    biasp(n) = xhat(4) * 180 / pi;
    biasq(n) = xhat(5) * 180 / pi;
    biasr(n) = xhat(6) * 180 / pi;
    
    subplot(2,1,1)
    plot(t(1:n), roll(1:n), t(1:n), rollhat(1:n), 'r-.')
   % if (n == 1)
   %     legend('True', 'Estimate')
   %     xlabel('Time (s)')
   %     ylabel('Angle (deg)')
   %     title('Roll')
   % end

    subplot(2,1,2)
    plot(t(1:n), pitch(1:n), t(1:n), pitchhat(1:n), 'r-.')
    %if (n == 1)
   %     legend('True', 'Estimate')
   %     xlabel('Time (s)')
  %      ylabel('Angle (deg)')
  %      title('Pitch')
  %  end
    
    pause(0.01)
    
end




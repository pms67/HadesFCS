function [roll, pitch, yaw, altitude] = ekfAHRS(acc, gyr, mag, airspeed, pressure, dt)

    persistent flag
    persistent P
    persistent Q
    persistent R
    persistent xhat
    persistent g
    persistent lpfGyrCoeff
    persistent lpfAccCoeff
    persistent lpfVaCoeff
    persistent p q r
    persistent ax ay az
    persistent V
    
    if isempty(flag)
        flag = 1;
        g = 9.80665;
        P = eye(9) * 0.0001;
        Q = diag([0.1, 0.1, 0.1, 0.0000001, 0.0000001, 0.0000001, 1e-6, 1e-6, 1e-6]);
        R = eye(4) * 10;
        xhat = zeros(9, 1);
        
        lpfGyrCoeff = 0.8;
        lpfAccCoeff = 0.6;
        lpfVaCoeff = 0.75;
        
        p = 0; q = 0; r = 0;
        ax = 0; ay = 0; az = 0;
        V = 0;
    end

	p = lpfGyrCoeff * p + (1 - lpfGyrCoeff) * gyr(1);
    q = lpfGyrCoeff * q + (1 - lpfGyrCoeff) * gyr(2);
    r = lpfGyrCoeff * r + (1 - lpfGyrCoeff) * gyr(3);
    
    ax = lpfAccCoeff * ax + (1 - lpfAccCoeff) * acc(1);
    ay = lpfAccCoeff * ay + (1 - lpfAccCoeff) * acc(2);
    az = lpfAccCoeff * az + (1 - lpfAccCoeff) * acc(3);
    
    V = lpfVaCoeff * V + (1 - lpfVaCoeff) * airspeed;
   
    mx = mag(1);
    my = mag(2);
    mz = mag(3);   
    
    altitude = -8576.6 * log(pressure / 101325);
    
    phi = xhat(1);
    theta = xhat(2);
       
    for IT=1:10
        phi = xhat(1);
        theta = xhat(2);
        bp = xhat(4);
        bq =  xhat(5);
        br =  xhat(6);
        
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
    
    phi = xhat(1);
    theta = xhat(2);
    psi = xhat(3);
    bp = xhat(4);
    bq =  xhat(5);
    br =  xhat(6);
    bx = xhat(7);
    by = xhat(8);
    bz = xhat(9);
                                  
    % Jacobian of h   
    C = [                      0,                                  g*cos(theta) - V*cos(theta)*(bq - q), 0,            0, -V*sin(theta),             0, -1,  0,  0; ...
 -g*cos(phi)*cos(theta), V*cos(theta)*(bp - p) + V*sin(theta)*(br - r) + g*sin(phi)*sin(theta), 0, V*sin(theta),             0, -V*cos(theta),  0, -1,  0; ...
  g*cos(theta)*sin(phi),                         g*cos(phi)*sin(theta) - V*sin(theta)*(bq - q), 0,            0,  V*cos(theta),             0,  0,  0, -1; ...
                      0,                                                                     0, 1,            0,             0,             0,  0,  0,  0];
                  
                  
    K = P * C' / (C * P * C' + R);
    P = (eye(9) - K * C) * P;
        
    % Output function h    
    h = [g*sin(theta) - V*sin(theta)*(bq - q) - bx; ...
 V*sin(theta)*(bp - p) - V*cos(theta)*(br - r) - g*cos(theta)*sin(phi) - by; ...
                         V*cos(theta)*(bq - q) - g*cos(phi)*cos(theta) - bz; ...
                         psi];
                              
    psimeas = atan2(-my * cos(phi) + mz * sin(phi), mx * cos(theta) + (my * sin(phi) + mz * cos(phi)) * sin(theta)) + 50 * pi / 180;
    xhat = xhat + K * ([ax; ay; az; psimeas] - h);

    roll = xhat(1) * 180 / pi;
    pitch = xhat(2) * 180 / pi;
    yaw = xhat(3) * 180 / pi;
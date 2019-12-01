function x = ekfAtt2(acc, gyr, psimeas, V, dt)

    persistent flag
    persistent P
    persistent Q
    persistent R 
    persistent xhat
    
    if isempty(flag)
       flag = 1;
       P = eye(5);
       Q = eye(5) * 0.01;
       R = eye(4) * 0.01;
       xhat = zeros(5, 1);
    end

    g = 9.81;
    
    p = gyr(1);
    q = gyr(2);
    r = gyr(3);
    
    phi = xhat(1);
    theta = xhat(2);
    bp = xhat(3);
    bq = xhat(4);
    br = xhat(5);
    
    % State transition function
    f = [p - bp - cos(phi)*tan(theta)*(br - r) - sin(phi)*tan(theta)*(bq - q); ...
         sin(phi)*(br - r) - cos(phi)*(bq - q)];                                           
       
    xhat = xhat + dt * f;
    
    phi = xhat(1);
    theta = xhat(2);
    bq = xhat(4);
    br = xhat(5);
                                                                       
    % Jacobian of f    
    A = [ sin(phi)*tan(theta)*(br - r) - cos(phi)*tan(theta)*(bq - q), - cos(phi)*(br - r)*(tan(theta)^2 + 1) - sin(phi)*(bq - q)*(tan(theta)^2 + 1), -1, -sin(phi)*tan(theta), -cos(phi)*tan(theta); ...
          cos(phi)*(br - r) + sin(phi)*(bq - q), 0,  0,            -cos(phi),             sin(phi)];
                  
    P = P + dt * (A * P + P * A' + Q);    
                                  
    % Jacobian of z
    C =  [                      0,          g*cos(theta), 0, 0,  0; ...
         -g*cos(phi)*cos(theta), g*sin(phi)*sin(theta), 0, 0, -V; ...
          g*cos(theta)*sin(phi), g*cos(phi)*sin(theta), 0, V,  0];
     
    K = P * C' * (R + C * P * C') ^ -1;
    P = (eye(6) - K * C) * P;
        
    % Output function z
    z = [g*sin(theta); ...
        -V*(br - r) - g*cos(theta)*sin(phi); ...
         V*(bq - q) - g*cos(phi)*cos(theta)];

    xhat = xhat + K * ([ax; ay; az] - z);
    
    x = xhat;
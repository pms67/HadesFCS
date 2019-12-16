function data = UAVDataAnalysis( filename )

    %First row contains variable names, so read from second row
    data = csvread(filename, 1, 0);
    
    % Extract variables
    t       = data(:, 1);
    delta_t = data(:, 2); delta_a = data(:, 3); delta_e = data(:, 4); delta_r = data(:, 5);
    Va = data(:, 6); alpha = data(:, 7); beta = data(:, 8);
    sp_airspeed = data(:, 9); sp_roll_angle  = data(:, 10); sp_pitch_angle = data(:, 11); sp_altitude = data(:, 12); sp_sideslip = data(:, 13); 
    north = data(:, 14); east = data(:, 15); altitude = data(:, 16);    
    u = data(:, 17); v = data(:, 18); w = data(:, 19);    
    phi = data(:, 20); theta = data(:, 21); psi = data(:, 22);    
    p = data(:, 23); q = data(:, 24); r = data(:, 25);
    
    subplot(3,3,1)
    plot3(north, east, altitude)
    title('Trajectory')
    xlabel('North (m)')
    ylabel('East (m)')
    zlabel('Altitude (m)')
    
    subplot(3,3,2)
    plot(t,sp_airspeed,t,Va)
    title('Airspeed')
    legend('Setpoint', 'Airspeed')
    xlabel('Time (s)')
    ylabel('Velocity (m/s)')
    
    subplot(3,3,3)
    plot(t,sp_altitude,t,altitude)
    title('Altitude')
    legend('Setpoint', 'Altitude')
    xlabel('Time (s)')
    ylabel('Altitude (m)')
    
    subplot(3,3,4)
    plot(t,sp_roll_angle,t,phi)
    title('Roll Angle')
    legend('Setpoint', 'Roll Angle')
    xlabel('Time (s)')
    ylabel('Angle (°)')
        
    subplot(3,3,5)
    plot(t,sp_pitch_angle,t,theta)
    title('Pitch Angle')
    legend('Setpoint', 'Pitch Angle')
    xlabel('Time (s)')
    ylabel('Angle (°)')
                
    subplot(3,3,6)
    plot(t,sp_sideslip,t,beta)
    title('Sideslip')
    legend('Setpoint', 'Sideslip')
    xlabel('Time (s)')
    ylabel('Angle (°)')
    
    subplot(3,3,7)
    plot(t,u,t,v,t,w)
    title('Velocity (Body Frame)')
    legend('u', 'v', 'w')
    xlabel('Time (s)')
    ylabel('Velocity (m/s)')
    
    subplot(3,3,8)
    plot(t,p,t,q,t,r)
    title('Angular Velocity (Body Frame)')
    legend('p', 'q', 'r')
    xlabel('Time (s)')
    ylabel('Velocity (°/s)')
    
    subplot(3,3,9)
    plot(t,delta_t,t,delta_a,t,delta_e,t,delta_r)
    title('Control Inputs')
    legend('Thrust', 'Aileron', 'Elevator', 'Rudder')
    xlabel('Time (s)')
    ylabel('Magnitude')
    
end


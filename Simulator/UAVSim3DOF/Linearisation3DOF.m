% Aircraft parameters
m = 1.56;
Iyy = 0.0576;
S = 0.2589;
c = 0.3302;

CD0 = 0.01631;
CDalpha = 0.2108;
CDq = 0.0;
CDde = 0.3045;

CL0 = 0.09167;
CLalpha = 3.5016;
CLq = 2.8932;
CLde = 0.2724;

Cm0 = -0.02338;
Cmalpha = -0.5675;
Cmq = -1.3990;
Cmde = -0.3254;

g = 9.8605;
rho = 1.225;

% Aircraft state or linearisation
u = 20.0;
w = 0.0;

Va = sqrt(u * u + w * w);
q = 0.0;
theta = 5.0 * pi / 180.0;

A = [0, -q, -(w+0.25*rho*Va*S*c*CDq/m),-(g*cos(theta)+0.5*rho*Va*Va*S*CDalpha/m); ...
     q,  0,   u-0.25*rho*Va*S*c*CLq/m, -(g*sin(theta)+0.5*rho*Va*Va*S*CLalpha/m); ...
     0,  0, 0.25*rho*Va*S*c*c*Cmq/Iyy, 0.5*rho*Va*Va*S*c*Cmalpha/Iyy; ...
     0, 0, 1, 0];
 
Btwoinputs = [1/m, -0.5*rho*Va*Va*S*CDde/m; ...
     0,   -0.5*rho*Va*Va*S*CLde/m; ...
     0,    0.5*rho*Va*Va*S*Cmde/m; ...
     0,    0];

Bthrottle = [1/m; 0; 0; 0];
Belevator = [-0.5*rho*Va*Va*S*CDde/m; ...
             -0.5*rho*Va*Va*S*CLde/m; ...
              0.5*rho*Va*Va*S*Cmde/m; ...
              0]; 

Cairspeed = [1, 0, 0, 0];
Cpitch    = [0, 0, 0, 1];

maxMotorThrustN = 0.5 * 9.81;
motorTimeConstant = 0.01;
GMotor = tf(0.01 * maxMotorThrustN, [motorTimeConstant, 1]); % Max throttle setting = 100%, min = 0%

GVa = tf(ss(A,Bthrottle,Cairspeed,0));

Gtheta = tf(ss(A,Belevator,Cpitch,0));

